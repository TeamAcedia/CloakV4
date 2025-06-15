// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

// send_damage(damage)
int ModApiClient::l_send_damage(lua_State *L)
{
	u16 damage = luaL_checknumber(L, 1);
	getClient(L)->sendDamage(damage);
	return 0;	
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
	API_FCT(place_node);
	API_FCT(dig_node);
	API_FCT(start_dig);
	API_FCT(get_inv_item_damage);
	API_FCT(get_inv_item_break);
	API_FCT(add_task_node);
	API_FCT(clear_task_node);
	API_FCT(add_task_tracer);
	API_FCT(clear_task_tracer);
}
