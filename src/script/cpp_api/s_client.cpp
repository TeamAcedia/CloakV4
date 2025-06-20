// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

#include "s_client.h"
#include "s_internal.h"
#include "client/client.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "lua_api/l_item.h"
#include "itemdef.h"
#include "lua_api/l_clientobject.h"
#include "s_item.h"

void ScriptApiClient::on_mods_loaded()
{
	SCRIPTAPI_PRECHECKHEADER

	// Get registered shutdown hooks
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_mods_loaded");
	// Call callbacks
	try {
		runCallbacks(0, RUN_CALLBACKS_MODE_FIRST);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}
}

void ScriptApiClient::on_shutdown()
{
	SCRIPTAPI_PRECHECKHEADER

	// Get registered shutdown hooks
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_shutdown");
	// Call callbacks
	try {
		runCallbacks(0, RUN_CALLBACKS_MODE_FIRST);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}
}

bool ScriptApiClient::on_sending_message(const std::string &message)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_chat_messages
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_sending_chat_message");
	// Call callbacks
	lua_pushstring(L, message.c_str());
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_receiving_message(const std::string &message)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_chat_messages
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_receiving_chat_message");
	// Call callbacks
	lua_pushstring(L, message.c_str());
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

void ScriptApiClient::on_damage_taken(int32_t damage_amount)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_chat_messages
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_damage_taken");
	// Call callbacks
	lua_pushinteger(L, damage_amount);
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}
}

void ScriptApiClient::on_hp_modification(int32_t newhp)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_chat_messages
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_hp_modification");
	// Call callbacks
	lua_pushinteger(L, newhp);
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}
}

void ScriptApiClient::environment_step(float dtime)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_globalsteps
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_globalsteps");
	// Call callbacks
	lua_pushnumber(L, dtime);
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_FIRST);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}
}

bool ScriptApiClient::on_dignode(v3s16 p, MapNode node)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_dignode
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_dignode");

	// Push data
	push_v3s16(L, p);
	pushnode(L, node);

	// Call functions
	try {
		runCallbacks(2, RUN_CALLBACKS_MODE_OR);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return lua_toboolean(L, -1);
}

bool ScriptApiClient::on_punchnode(v3s16 p, MapNode node)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_punchgnode
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_punchnode");

	// Push data
	push_v3s16(L, p);
	pushnode(L, node);

	// Call functions
	try {
		runCallbacks(2, RUN_CALLBACKS_MODE_OR);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_placenode(const PointedThing &pointed, const ItemDefinition &item)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_placenode
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_placenode");

	// Push data
	push_pointed_thing(L, pointed, true);
	push_item_definition(L, item);

	// Call functions
	try {
		runCallbacks(2, RUN_CALLBACKS_MODE_OR);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_item_use(const ItemStack &item, const PointedThing &pointed)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_item_use
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_item_use");

	// Push data
	LuaItemStack::create(L, item);
	push_pointed_thing(L, pointed, true);

	// Call functions
	try {
		runCallbacks(2, RUN_CALLBACKS_MODE_OR);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_recieve_physics_override(float speed, float jump, float gravity, bool sneak, bool sneak_glitch, bool new_move, float speed_climb, float speed_crouch, float liquid_fluidity, 
													  float liquid_fluidity_smooth, float liquid_sink, float acceleration_default, float acceleration_air)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_recieve_physics_override
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_recieve_physics_override");

	// Push data
	push_physics_override(L, speed, jump, gravity, sneak, sneak_glitch, new_move, speed_climb, speed_crouch, liquid_fluidity, liquid_fluidity_smooth, liquid_sink, acceleration_default, acceleration_air);

	// Call functions
	runCallbacks(1, RUN_CALLBACKS_MODE_OR);
	return readParam<bool>(L, -1);
}

void ScriptApiClient::on_object_properties_change(s16 id)
{
	SCRIPTAPI_PRECHECKHEADER
	// Get core.on_object_properties_change
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_object_properties_change");
	// Push data
	ClientObjectRef::create(L, id);
	// Call functions
	runCallbacks(1, RUN_CALLBACKS_MODE_FIRST);
}

void ScriptApiClient::on_object_hp_change(s16 id)
{
	SCRIPTAPI_PRECHECKHEADER
	// Get core.on_object_hp_change
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_object_hp_change");
	// Push data
	ClientObjectRef::create(L, id);
	// Call functions
	runCallbacks(1, RUN_CALLBACKS_MODE_FIRST);
}

bool ScriptApiClient::on_object_add(s16 id)
{
	SCRIPTAPI_PRECHECKHEADER

	// Get core.registered_on_object_add
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_object_add");

	// Push data
	push_objectRef(L, id);

	// Call functions
	runCallbacks(1, RUN_CALLBACKS_MODE_OR);
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_inventory_open(Inventory *inventory)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_inventory_open");

	push_inventory_lists(L, *inventory);

	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_block_data(v3s16 pos)
{
	SCRIPTAPI_PRECHECKHEADER

	//std::cout << "ScriptAPIClient: " << pos.X << ", " << pos.Y << ", " << pos.Z << "\n";

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_block_data");

	// Push data
	push_v3s16(L, pos);

	// Call functions
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
	}

	return false;
}

bool ScriptApiClient::on_player_join(std::string name)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_player_join");

	lua_pushstring(L, name.c_str());
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

bool ScriptApiClient::on_player_leave(std::string name)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_player_leave");

	lua_pushstring(L, name.c_str());
	try {
		runCallbacks(1, RUN_CALLBACKS_MODE_OR_SC);
	} catch (LuaError &e) {
		getClient()->setFatalError(e);
		return true;
	}
	return readParam<bool>(L, -1);
}

v3f ScriptApiClient::get_send_speed(v3f speed)
{
    SCRIPTAPI_PRECHECKHEADER

    PUSH_ERROR_HANDLER(L);
    int error_handler = lua_gettop(L) - 1;
    lua_insert(L, error_handler);

    lua_getglobal(L, "core");
    lua_getfield(L, -1, "get_send_speed");
    int type = lua_type(L, -1);

    bool callable = false;

    if (type == LUA_TFUNCTION) {
        callable = true;
    } else if (type == LUA_TTABLE) {
        if (lua_getmetatable(L, -1)) {
            lua_getfield(L, -1, "__call");
            if (lua_isfunction(L, -1)) {
                callable = true;
            }
            lua_pop(L, 2);
        }
    }

    if (callable) {
        speed /= BS;
        push_v3f(L, speed); 
        if (lua_pcall(L, 1, 1, error_handler) != 0) {
            const char *error_msg = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getglobal(L, "core");
            lua_getfield(L, -1, "debug");
          //  lua_pushfstring(L, "Error calling core.get_send_speed: %s", error_msg);
            lua_pcall(L, 1, 0, error_handler);
        } else {
            speed = read_v3f(L, -1);
            speed *= BS;
        }
    } else {
        lua_getglobal(L, "core");
        lua_getfield(L, -1, "debug");
    //   lua_pushstring(L, "core.get_send_speed is not callable, skipping call");
        lua_pcall(L, 1, 0, error_handler);
    }

    return speed;
}



void ScriptApiClient::setEnv(ClientEnvironment *env)
{
	ScriptApiBase::setEnv(env);
}
