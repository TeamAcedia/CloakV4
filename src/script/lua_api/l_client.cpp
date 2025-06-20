// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

#include <iostream>

#include "l_client.h"
#include "chatmessage.h"
#include "client/client.h"
#include "client/clientevent.h"
#include "client/sound.h"
#include "client/clientenvironment.h"
#include "client/game.h"
#include "common/c_content.h"
#include "common/c_converter.h"
#include "cpp_api/s_base.h"
#include "gettext.h"
#include "l_internal.h"
#include "lua_api/l_nodemeta.h"
#include "gui/mainmenumanager.h"
#include "map.h"
#include "util/string.h"
#include "nodedef.h"
#include "client/render/plain.h"

#include "l_clientobject.h"
#include "client/keycode.h"
#include "client/game.h"

#define checkCSMRestrictionFlag(flag) \
	( getClient(L)->checkCSMRestrictionFlag(CSMRestrictionFlags::flag) )

// Not the same as FlagDesc, which contains an `u32 flag`
struct CSMFlagDesc {
	const char *name;
	u64 flag;
};

/*
	FIXME: This should eventually be moved somewhere else
	It also needs to be kept in sync with the definition of CSMRestrictionFlags
	in network/networkprotocol.h
*/
const static CSMFlagDesc flagdesc_csm_restriction[] = {
	{"load_client_mods",  CSM_RF_LOAD_CLIENT_MODS},
	{"chat_messages",     CSM_RF_CHAT_MESSAGES},
	{"read_itemdefs",     CSM_RF_READ_ITEMDEFS},
	{"read_nodedefs",     CSM_RF_READ_NODEDEFS},
	{"lookup_nodes",      CSM_RF_LOOKUP_NODES},
	{"read_playerinfo",   CSM_RF_READ_PLAYERINFO},
	{NULL,      0}
};

// get_current_modname()
int ModApiClient::l_get_current_modname(lua_State *L)
{
	std::string s = ScriptApiBase::getCurrentModNameInsecure(L);
	if (!s.empty())
		lua_pushstring(L, s.c_str());
	else
		lua_pushnil(L);
	return 1;
}

// get_modpath(modname)
int ModApiClient::l_get_modpath(lua_State *L)
{
	std::string modname = readParam<std::string>(L, 1);
	// Client mods use a virtual filesystem, see Client::scanModSubfolder()
	std::string path = modname + ":";
	lua_pushstring(L, path.c_str());
	return 1;
}

// print(text)
int ModApiClient::l_print(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string text = luaL_checkstring(L, 1);
	rawstream << text << std::endl;
	return 0;
}

// display_chat_message(message)
int ModApiClient::l_display_chat_message(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return 0;

	std::string message = luaL_checkstring(L, 1);
	getClient(L)->pushToChatQueue(new ChatMessage(utf8_to_wide(message)));
	lua_pushboolean(L, true);
	return 1;
}

// send_chat_message(message)
int ModApiClient::l_send_chat_message(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return 0;

	// If server disabled this API, discard

	if (checkCSMRestrictionFlag(CSM_RF_CHAT_MESSAGES))
		return 0;

	std::string message = luaL_checkstring(L, 1);
	getClient(L)->sendChatMessage(utf8_to_wide(message));
	return 0;
}

// clear_out_chat_queue()
int ModApiClient::l_clear_out_chat_queue(lua_State *L)
{
	getClient(L)->clearOutChatQueue();
	return 0;
}

// get_player_names()
int ModApiClient::l_get_player_names(lua_State *L)
{
	if (checkCSMRestrictionFlag(CSM_RF_READ_PLAYERINFO))
		return 0;

	auto plist = getClient(L)->getConnectedPlayerNames();
	lua_createtable(L, plist.size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	for (const std::string &name : plist) {
		lua_pushstring(L, name.c_str());
		lua_rawseti(L, newTable, index);
		index++;
	}
	return 1;
}

// disconnect()
int ModApiClient::l_disconnect(lua_State *L)
{
	// Stops badly written Lua code form causing boot loops
	if (getClient(L)->isShutdown()) {
		lua_pushboolean(L, false);
		return 1;
	}

	g_gamecallback->disconnect();
	lua_pushboolean(L, true);
	return 1;
}

// gettext(text)
int ModApiClient::l_gettext(lua_State *L)
{
	std::string text = strgettext(luaL_checkstring(L, 1));
	lua_pushstring(L, text.c_str());

	return 1;
}

// get_node_or_nil(pos)
// pos = {x=num, y=num, z=num}
int ModApiClient::l_get_node_or_nil(lua_State *L)
{
	// pos
	v3s16 pos = read_v3s16(L, 1);

	// Do it
	bool pos_ok;
	MapNode n = getClient(L)->CSMGetNode(pos, &pos_ok);
	if (pos_ok) {
		// Return node
		pushnode(L, n);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int iterator_func(lua_State *L) {
    std::vector<std::pair<v3s16, MapNode>> *nodes = reinterpret_cast<std::vector<std::pair<v3s16, MapNode>> *>(lua_touserdata(L, lua_upvalueindex(2)));

    // Get the current index from the upvalue
    int i = luaL_checkinteger(L, lua_upvalueindex(1));
    if (i >= static_cast<int>(nodes->size())){
        // If we've passed the end, return nil to signal end of iteration
        return 0;
    } else {
        // Otherwise, increment the upvalue for the next call...
        lua_pushinteger(L, i + 1);
        lua_replace(L, lua_upvalueindex(1));

        // ...and return the node at the current index
        push_v3s16(L, (*nodes)[i].first);
        pushnode(L, (*nodes)[i].second);
        return 2;  // number of returned values
    }
}

/*
returns an iterator:
for pos, node in minetest.all_loaded_nodes() do
	-- process node at pos
end
*/
// all_loaded_nodes()
int ModApiClient::l_all_loaded_nodes(lua_State *L){
    if (checkCSMRestrictionFlag(CSM_RF_LOOKUP_NODES)) {
        return 0;
    }

    Client *client = getClient(L);
    std::vector<std::pair<v3s16, MapNode>> *nodes = new std::vector<std::pair<v3s16, MapNode>>(client->getAllLoadedNodes());

    // We'll store the current index in the first upvalue of the iterator
    lua_pushinteger(L, 0);

    lua_pushlightuserdata(L, nodes);

    // This is the iterator function
    lua_pushcclosure(L, iterator_func, 2);  // 2 upvalues (the index and nodes)

    return 1;  // return the iterator function
}

/*
returns an iterator:
for pos, node in minetest.nodes_at_block_pos(block_pos) do
	-- process node at pos
end
*/
// nodes_at_block_pos(pos)
int ModApiClient::l_nodes_at_block_pos(lua_State *L){
    if (checkCSMRestrictionFlag(CSM_RF_LOOKUP_NODES)) {
        return 0;
    }

    v3s16 pos = read_v3s16(L, 1);
    Client *client = getClient(L);
    std::vector<std::pair<v3s16, MapNode>> *nodes = new std::vector<std::pair<v3s16, MapNode>>(client->getNodesAtBlockPos(pos));

    // We'll store the current index in the first upvalue of the iterator
    lua_pushinteger(L, 0);

    lua_pushlightuserdata(L, nodes);

    // This is the iterator function
    lua_pushcclosure(L, iterator_func, 2);  // 2 upvalues (the index and nodes)

    return 1;  // return the iterator function
}


// get_langauge()
int ModApiClient::l_get_language(lua_State *L)
{
#ifdef _WIN32
	char *locale = setlocale(LC_ALL, NULL);
#else
	char *locale = setlocale(LC_MESSAGES, NULL);
#endif
	std::string lang = gettext("LANG_CODE");
	if (lang == "LANG_CODE")
		lang.clear();

	lua_pushstring(L, locale);
	lua_pushstring(L, lang.c_str());
	return 2;
}

// get_meta(pos)
int ModApiClient::l_get_meta(lua_State *L)
{
	v3s16 p = read_v3s16(L, 1);

	// check restrictions first
	bool pos_ok;
	getClient(L)->CSMGetNode(p, &pos_ok);
	if (!pos_ok)
		return 0;

	NodeMetadata *meta = getEnv(L)->getMap().getNodeMetadata(p);
	NodeMetaRef::createClient(L, meta);
	return 1;
}

// get_server_info()
int ModApiClient::l_get_server_info(lua_State *L)
{
	Client *client = getClient(L);
	Address serverAddress = client->getServerAddress();
	lua_newtable(L);
	lua_pushstring(L, client->getAddressName().c_str());
	lua_setfield(L, -2, "address");
	lua_pushstring(L, serverAddress.serializeString().c_str());
	lua_setfield(L, -2, "ip");
	lua_pushinteger(L, serverAddress.getPort());
	lua_setfield(L, -2, "port");
	lua_pushinteger(L, client->getProtoVersion());
	lua_setfield(L, -2, "protocol_version");
	lua_pushinteger(L, client->getMapSeed());
	lua_setfield(L, -2, "seed");
	return 1;
}

// get_item_def(itemstring)
int ModApiClient::l_get_item_def(lua_State *L)
{
	IGameDef *gdef = getGameDef(L);
	assert(gdef);

	IItemDefManager *idef = gdef->idef();
	assert(idef);

	if (checkCSMRestrictionFlag(CSM_RF_READ_ITEMDEFS))
		return 0;

	if (!lua_isstring(L, 1))
		return 0;

	std::string name = readParam<std::string>(L, 1);
	if (!idef->isKnown(name))
		return 0;
	const ItemDefinition &def = idef->get(name);

	push_item_definition_full(L, def);

	return 1;
}

// get_node_def(nodename)
int ModApiClient::l_get_node_def(lua_State *L)
{
	IGameDef *gdef = getGameDef(L);
	assert(gdef);

	const NodeDefManager *ndef = gdef->ndef();
	assert(ndef);

	if (!lua_isstring(L, 1))
		return 0;

	if (checkCSMRestrictionFlag(CSM_RF_READ_NODEDEFS))
		return 0;

	std::string name = readParam<std::string>(L, 1);
	const ContentFeatures &cf = ndef->get(ndef->getId(name));
	if (cf.name != name) // Unknown node. | name = <whatever>, cf.name = ignore
		return 0;

	push_content_features(L, cf);

	return 1;
}

// get_privilege_list()
int ModApiClient::l_get_privilege_list(lua_State *L)
{
	const Client *client = getClient(L);
	lua_newtable(L);
	for (const std::string &priv : client->getPrivilegeList()) {
		lua_pushboolean(L, true);
		lua_setfield(L, -2, priv.c_str());
	}
	return 1;
}

// get_builtin_path()
int ModApiClient::l_get_builtin_path(lua_State *L)
{
	lua_pushstring(L, BUILTIN_MOD_NAME ":");
	return 1;
}

// get_csm_restrictions()
int ModApiClient::l_get_csm_restrictions(lua_State *L)
{
	u64 flags = getClient(L)->getCSMRestrictionFlags();
	const CSMFlagDesc *flagdesc = flagdesc_csm_restriction;

	lua_newtable(L);
	for (int i = 0; flagdesc[i].name; i++) {
		setboolfield(L, -1, flagdesc[i].name, !!(flags & flagdesc[i].flag));
	}
	return 1;
}
// place_node(pos)
int ModApiClient::l_place_node(lua_State *L)
{
	Client *client = getClient(L);
	v3s16 pos = read_v3s16(L, 1);
	PointedThing pointed;
	pointed.type = POINTEDTHING_NODE;
	pointed.node_abovesurface = pos;
	pointed.node_undersurface = pos;
	client->interact(INTERACT_PLACE, pointed);
	return 0;
}

// dig_node(pos)
int ModApiClient::l_dig_node(lua_State *L)
{
	Client *client = getClient(L);
	v3s16 pos = read_v3s16(L, 1);
	PointedThing pointed;
	pointed.type = POINTEDTHING_NODE;
	pointed.node_abovesurface = pos;
	pointed.node_undersurface = pos;
	client->interact(INTERACT_START_DIGGING, pointed);
	client->interact(INTERACT_DIGGING_COMPLETED, pointed);
	client->removeNode(pos);
	return 0;
}

// start_dig(pos)
int ModApiClient::l_start_dig(lua_State *L)
{
	Client *client = getClient(L);
	v3s16 pos = read_v3s16(L, 1);
	PointedThing pointed;
	pointed.type = POINTEDTHING_NODE;
	pointed.node_abovesurface = pos;
	pointed.node_undersurface = pos;
	client->interact(INTERACT_START_DIGGING, pointed);
	return 0;
}

// can_attack(object_id)
int ModApiClient::l_can_attack(lua_State *L)
{
	u16 object_id = lua_tointeger(L, 1);

	ClientEnvironment &env = getClient(L)->getEnv();
	GenericCAO *gcao = env.getGenericCAO(object_id);

	if (!gcao) {
		lua_pushnil(L);
		return 0;
	}

	bool can_attack = gcao->canAttack(1);

	lua_pushboolean(L, can_attack);

	return 1;
}

// get_server_url
int ModApiClient::l_get_server_url(lua_State *L)
{
	Client *client = getClient(L);
	if (!client->m_simple_singleplayer_mode) {
		Address serverAddress = client->getServerAddress();
		std::string address = client->getAddressName().c_str();
		u16 port = serverAddress.getPort();
		std::string server_url = address + ":" + toPaddedString(port);
		lua_pushstring(L, server_url.c_str());
		return 1;
	}
	lua_pushnil(L);
	return 0;
}

// get_inv_item_damage(index, object_id)
int ModApiClient::l_get_inv_item_damage(lua_State *L)
{
	Client *client = getClient(L);

	u32 index = luaL_checkinteger(L, 1) - 1;
	u16 object_id = lua_tointeger(L, 2);

	InventoryLocation inventory_location;
	std::string location = "current_player";

	inventory_location.deSerialize(location);
	Inventory *inventory = client->getInventory(inventory_location);
	if (!inventory) {
		lua_pushnil(L);
		return 0;
	}
	InventoryList *list = inventory->getList("main");
	if (!list) {
		lua_pushnil(L);
		return 0;
	}

	if (index < 0 || index > list->getSize() - 1) {
		lua_pushnil(L);
		return 0;
	}

	ItemStack punchitem;
	try {
		punchitem = list->getItem(index);
	} catch (...) {
		lua_pushnil(L);
		return 0;
	}

	const ToolCapabilities toolcap = punchitem.getToolCapabilities(client->idef());

	ClientEnvironment &env = client->getEnv();
	GenericCAO *gcao = env.getGenericCAO(object_id);
	if (!gcao) {
		lua_pushnil(L);
		return 0;
	}

	PunchDamageResult result = getPunchDamageFleshy(
			gcao->getGroups(),
			&toolcap,
			&punchitem,
			g_game->getRunData().time_from_last_punch,
			punchitem.wear);

	push_punch_damage_result(L, &result);

	return 1;
}

int ModApiClient::l_get_inv_item_break(lua_State *L)
{
	// get node and itemdefs
	IGameDef *gdef = getGameDef(L);
	if (!gdef)
	{
	    lua_pushnil(L);
	    return 0;
	}

	const NodeDefManager *ndef = gdef->ndef();
	if (!ndef)
	{
	    lua_pushnil(L);
	    return 0;
	}

	IItemDefManager *idef = gdef->idef();
	if (!idef)
	{
	    lua_pushnil(L);
	    return 0;
	}

	// get inputs
	u32 index = luaL_checkinteger(L, 1) - 1;
	v3s16 nodepos = read_v3s16(L, 2);

	// get inventory item
	Client *client = getClient(L);

	InventoryLocation inventory_location;
	std::string location = "current_player";

	inventory_location.deSerialize(location);
	Inventory *inventory = client->getInventory(inventory_location);
	if (!inventory) {
		lua_pushnil(L);
		return 0;
	}
	InventoryList *list = inventory->getList("main");
	if (!list) {
		lua_pushnil(L);
		return 0;
	}

	if (index < 0 || index > list->getSize() - 1) {
		lua_pushnil(L);
		return 0;
	}

	ItemStack selecteditem;
	try {
		selecteditem = list->getItem(index);
	} catch (...) {
		lua_pushnil(L);
		return 0;
	}

	const ToolCapabilities toolcap = selecteditem.getToolCapabilities(idef);

	// get node data
	ClientEnvironment &env = client->getEnv();
	ClientMap &map = env.getClientMap();
	MapNode n = map.getNode(nodepos);
	const ContentFeatures &features = ndef->get(n);

	// get dig result
	DigParams result = getDigParams(
			features.groups,
			&toolcap,
			selecteditem.wear);

	if (!result.diggable) {
		LocalPlayer *player = env.getLocalPlayer();
		ItemStack hand_item;
		player->getHandItem(&hand_item);
		result = getDigParams(
			features.groups,
			&hand_item.getToolCapabilities(idef));
	}

	push_dig_result(L, &result);

	return 1;
}

// add_task_node(pos, color)
int ModApiClient::l_add_task_node(lua_State *L)
{	
	v3f pos = checkFloatPos(L, 1);
	video::SColor color = read_ARGB8(L, 2);
	
	DrawTaskBlocksAndTracers::addTaskNode(TaskNode{pos, color});

	return 0;
}

// clear_task_node(pos)
int ModApiClient::l_clear_task_node(lua_State *L)
{
	v3f pos = checkFloatPos(L, 1);
	
	DrawTaskBlocksAndTracers::removeTaskNode(TaskNode{pos});

	return 0;
}

// add_task_tracer(start_pos, end_pos, color)
int ModApiClient::l_add_task_tracer(lua_State *L)
{
	v3f start_pos = checkFloatPos(L, 1);
	v3f end_pos = checkFloatPos(L, 2);
	video::SColor color = read_ARGB8(L, 3);
	
	DrawTaskBlocksAndTracers::addTaskTracer(TaskTracer{start_pos, end_pos, color});

	return 0;
}

// clear_task_tracer(start_pos, end_pos)
int ModApiClient::l_clear_task_tracer(lua_State *L)
{
	v3f start_pos = checkFloatPos(L, 1);
	v3f end_pos = checkFloatPos(L, 2);
	
	DrawTaskBlocksAndTracers::removeTaskTracer(TaskTracer{start_pos, end_pos});

	return 0;
}


int ModApiClient::l_send_damage(lua_State *L)
{
	u16 damage = luaL_checknumber(L, 1);
	getClient(L)->sendDamage(damage);
	return 0;
}

// set_fast_speed(speed)
int ModApiClient::l_set_fast_speed(lua_State *L)
{
	f32 newSpeed = luaL_checknumber(L, 1);
	g_settings->setFloat("movement_speed_fast", newSpeed);
	return 0;
}

// get_inventory(location)
int ModApiClient::l_get_inventory(lua_State *L)
{
	Client *client = getClient(L);
	InventoryLocation inventory_location;
	Inventory *inventory;
	std::string location;

	location = readParam<std::string>(L, 1);

	try {
		inventory_location.deSerialize(location);
		inventory = client->getInventory(inventory_location);
		if (! inventory)
			throw SerializationError(std::string("Attempt to access nonexistant inventory (") + location + ")");
		push_inventory_lists(L, *inventory);
	} catch (SerializationError &) {
		lua_pushnil(L);
	}

	return 1;
}

// set_keypress(key_setting, pressed) -> returns true on success
int ModApiClient::l_set_keypress(lua_State *L)
{
	std::string setting_name = "keymap_" + readParam<std::string>(L, 1);
	bool pressed = lua_isboolean(L, 2) && readParam<bool>(L, 2);
	try {
		KeyPress keyCode = getKeySetting(setting_name.c_str());
		if (pressed)
			g_game->getInput()->setKeypress(keyCode);
		else
			g_game->getInput()->unsetKeypress(keyCode);
		lua_pushboolean(L, true);
	} catch (SettingNotFoundException &) {
		lua_pushboolean(L, false);
	}
	return 1;
}

// drop_selected_item()
int ModApiClient::l_drop_selected_item(lua_State *L)
{
	g_game->dropSelectedItem();
	return 0;
}

// get_objects_inside_radius(pos, radius)
int ModApiClient::l_get_objects_inside_radius(lua_State *L)
{
    ClientEnvironment &env = getClient(L)->getEnv();

    v3f pos = checkFloatPos(L, 1);
    float radius = readParam<float>(L, 2) * BS;

    std::vector<DistanceSortedActiveObject> objs;
    env.getActiveObjects(pos, radius, objs);

    lua_createtable(L, objs.size(), 0);
    for (size_t i = 0; i < objs.size(); ++i) {
        ClientObjectRef::create(L, objs[i].obj);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

// get_all_objects(pos)
int ModApiClient::l_get_all_objects(lua_State *L)
{
	ClientEnvironment &env = getClient(L)->getEnv();

	v3f pos = checkFloatPos(L, 1);

	std::vector<DistanceSortedActiveObject> objs;
	env.getAllActiveObjects(pos, objs);

	int i = 0;
	lua_createtable(L, objs.size(), 0);
	for (const auto obj : objs) {
		push_objectRef(L, obj.obj->getId());
		lua_rawseti(L, -2, ++i);
	}
	return 1;
}

// make_screenshot()
int ModApiClient::l_make_screenshot(lua_State *L)
{
	getClient(L)->makeScreenshot();
	return 0;
}

/*
`pointed_thing`
---------------

* `{type="nothing"}`
* `{type="node", under=pos, above=pos}`
    * Indicates a pointed node selection box.
    * `under` refers to the node position behind the pointed face.
    * `above` refers to the node position in front of the pointed face.
* `{type="object", ref=ObjectRef}`

Exact pointing location (currently only `Raycast` supports these fields):

* `pointed_thing.intersection_point`: The absolute world coordinates of the
  point on the selection box which is pointed at. May be in the selection box
  if the pointer is in the box too.
* `pointed_thing.box_id`: The ID of the pointed selection box (counting starts
  from 1).
* `pointed_thing.intersection_normal`: Unit vector, points outwards of the
  selected selection box. This specifies which face is pointed at.
  Is a null vector `{x = 0, y = 0, z = 0}` when the pointer is inside the
  selection box.
*/
// interact(action, pointed_thing)
int ModApiClient::l_interact(lua_State *L)
{
	std::string action_str = readParam<std::string>(L, 1);
	InteractAction action;

	if (action_str == "start_digging")
		action = INTERACT_START_DIGGING;
	else if (action_str == "stop_digging")
		action = INTERACT_STOP_DIGGING;
	else if (action_str == "digging_completed")
		action = INTERACT_DIGGING_COMPLETED;
	else if (action_str == "place")
		action = INTERACT_PLACE;
	else if (action_str == "use")
		action = INTERACT_USE;
	else if (action_str == "activate")
		action = INTERACT_ACTIVATE;
	else
		return 0;

	lua_getfield(L, 2, "type");
	if (! lua_isstring(L, -1))
		return 0;
	std::string type_str = lua_tostring(L, -1);
	lua_pop(L, 1);

	PointedThingType type;

	if (type_str == "nothing")
		type = POINTEDTHING_NOTHING;
	else if (type_str == "node")
		type = POINTEDTHING_NODE;
	else if (type_str == "object")
		type = POINTEDTHING_OBJECT;
	else
		return 0;

	PointedThing pointed;
	pointed.type = type;
	ClientObjectRef *obj;

	switch (type) {
	case POINTEDTHING_NODE:
		lua_getfield(L, 2, "under");
		pointed.node_undersurface = check_v3s16(L, -1);

		lua_getfield(L, 2, "above");
		pointed.node_abovesurface = check_v3s16(L, -1);
		break;
	case POINTEDTHING_OBJECT:
		lua_getfield(L, 2, "ref");
		obj = ClientObjectRef::checkobject(L, -1);
		pointed.object_id = obj->getClientActiveObject()->getId();
		break;
	default:
		break;
	}

	getClient(L)->interact(action, pointed);
	lua_pushboolean(L, true);
	return 1;
}


// file_write(path, content)
int ModApiClient::l_file_write(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *path = luaL_checkstring(L, 1);
	auto content = readParam<std::string_view>(L, 2);
	bool unsafe_write = false;
	if(lua_istable(L, 3)){
		lua_getfield(L, 3, "UNSAFE");
		if(lua_isboolean(L, -1)){
			unsafe_write = lua_toboolean(L, -1);
		}
		lua_pop(L, 1); 
	}

	if (!unsafe_write || g_settings->getBool("secure.enable_security")) {
		CHECK_SECURE_PATH(L, path, true);
	}

	bool ret = fs::safeWriteToFile(path, content);
	lua_pushboolean(L, ret);

	return 1;
}

// file_append(path, content)
int ModApiClient::l_file_append(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *path = luaL_checkstring(L, 1);
	auto content = readParam<std::string_view>(L, 2);
	bool unsafe_write = false;
	if(lua_istable(L, 3)){
		lua_getfield(L, 3, "UNSAFE");
		if(lua_isboolean(L, -1)){
			unsafe_write = lua_toboolean(L, -1);
		}
		lua_pop(L, 1); 
	}

	if (!unsafe_write || g_settings->getBool("secure.enable_security")) {
		CHECK_SECURE_PATH(L, path, true);
	}

	bool ret = fs::safeAppendToFile(path, content);
	lua_pushboolean(L, ret);

	return 1;
}

// get_node_name(pos)
int ModApiClient::l_get_node_name(lua_State *L)
{
	v3s16 pos = read_v3s16(L, 1);

	Client *client = getClient(L);
	const NodeDefManager *nodedef = client->getNodeDefManager();

	bool pos_ok;
	MapNode n = getClient(L)->CSMGetNode(pos, &pos_ok);
	if (pos_ok) {
		if (n.getContent() == CONTENT_IGNORE) {
			lua_pushstring(L, "ignore");
		} else if (nodedef->get(n).name == "unknown") {
			lua_pushstring(L, "unknown");
		} else {
			lua_pushstring(L, nodedef->get(n).name.c_str());
		}
	} else {
		lua_pushnil(L);
	}
    
    return 1; 
}



// add_task_node(pos, color)
int ModApiClient::l_add_task_node(lua_State *L)
{	
	v3f pos = checkFloatPos(L, 1);
	video::SColor color = read_ARGB8(L, 2);
	
	DrawTaskBlocksAndTracers::addTaskNode(TaskNode{pos, color});

	return 0;
}

// clear_task_node(pos)
int ModApiClient::l_clear_task_node(lua_State *L)
{
	v3f pos = checkFloatPos(L, 1);
	
	DrawTaskBlocksAndTracers::removeTaskNode(TaskNode{pos});

	return 0;
}

// add_task_tracer(start_pos, end_pos, color)
int ModApiClient::l_add_task_tracer(lua_State *L)
{
	v3f start_pos = checkFloatPos(L, 1);
	v3f end_pos = checkFloatPos(L, 2);
	video::SColor color = read_ARGB8(L, 3);
	
	DrawTaskBlocksAndTracers::addTaskTracer(TaskTracer{start_pos, end_pos, color});

	return 0;
}

// clear_task_tracer(start_pos, end_pos)
int ModApiClient::l_clear_task_tracer(lua_State *L)
{
	v3f start_pos = checkFloatPos(L, 1);
	v3f end_pos = checkFloatPos(L, 2);
	
	DrawTaskBlocksAndTracers::removeTaskTracer(TaskTracer{start_pos, end_pos});

	return 0;
}


StringMap *table_to_stringmap(lua_State *L, int index)
{
	StringMap *m = new StringMap;

	lua_pushvalue(L, index);
	lua_pushnil(L);

	while (lua_next(L, -2)) {
		lua_pushvalue(L, -2);
		std::basic_string<char> key = lua_tostring(L, -1);
		std::basic_string<char> value = lua_tostring(L, -2);
		(*m)[key] = value;
		lua_pop(L, 2);
	}

	lua_pop(L, 1);

	return m;
}

// send_inventory_fields(formname, fields)
// Only works if the inventory form was opened beforehand.
int ModApiClient::l_send_inventory_fields(lua_State *L)
{
	std::string formname = luaL_checkstring(L, 1);
	StringMap *fields = table_to_stringmap(L, 2);

	getClient(L)->sendInventoryFields(formname, *fields);
	return 0;
}

// send_nodemeta_fields(position, formname, fields)
int ModApiClient::l_send_nodemeta_fields(lua_State *L)
{
	v3s16 pos = check_v3s16(L, 1);
	std::string formname = luaL_checkstring(L, 2);
	StringMap *m = table_to_stringmap(L, 3);

	getClient(L)->sendNodemetaFields(pos, formname, *m);
	return 0;
}

// update_infotexts()
int ModApiClient::l_update_infotexts(lua_State *L)
{
	getClient(L)->getScript()->update_infotexts();
	lua_pushboolean(L, true);
	return 1;
}

int ModApiClient::l_get_description(lua_State* L) {
	getClient(L)->getScript()->get_description();
	lua_pushboolean(L, true);
	return 1;
}

void ModApiClient::Initialize(lua_State *L, int top)
{
	API_FCT(get_current_modname);
	API_FCT(get_modpath);
	API_FCT(print);
	API_FCT(display_chat_message);
	API_FCT(send_chat_message);
	API_FCT(clear_out_chat_queue);
	API_FCT(get_player_names);
	API_FCT(gettext);
	API_FCT(get_node_or_nil);
	API_FCT(disconnect);
	API_FCT(get_meta);
	API_FCT(get_server_info);
	API_FCT(get_item_def);
	API_FCT(get_node_def);
	API_FCT(get_privilege_list);
	API_FCT(get_builtin_path);
	API_FCT(get_language);
	API_FCT(get_csm_restrictions);
	API_FCT(send_damage);
	API_FCT(dig_node);
	API_FCT(start_dig);
	API_FCT(get_inv_item_damage);
	API_FCT(get_inv_item_break);
	API_FCT(add_task_node);
	API_FCT(clear_task_node);
	API_FCT(add_task_tracer);
	API_FCT(clear_task_tracer);

	API_FCT(set_fast_speed);
	API_FCT(place_node);

	API_FCT(interact);
	API_FCT(get_inventory);
	API_FCT(set_keypress);
	API_FCT(drop_selected_item);
	API_FCT(get_objects_inside_radius);
	API_FCT(get_all_objects);
	API_FCT(make_screenshot);
	API_FCT(all_loaded_nodes);
	API_FCT(nodes_at_block_pos);
	API_FCT(can_attack);
	API_FCT(get_server_url);
	API_FCT(file_write);
	API_FCT(file_append);
	API_FCT(get_node_name);
	API_FCT(add_task_node);
	API_FCT(clear_task_node);
	API_FCT(add_task_tracer);
	API_FCT(clear_task_tracer);
	API_FCT(send_inventory_fields);
	API_FCT(send_nodemeta_fields);
	API_FCT(update_infotexts);
	API_FCT(get_description);
}
