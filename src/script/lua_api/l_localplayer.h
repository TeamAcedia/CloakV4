// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2017 Dumbeldor, Vincent Glize <vincent.glize@live.fr>

#pragma once

#include "l_base.h"

class LocalPlayer;

class LuaLocalPlayer : public ModApiBase
{
private:
	static const luaL_Reg methods[];

	// garbage collector
	static int gc_object(lua_State *L);

	// get_velocity(self)
	static int l_get_velocity(lua_State *L);

	// get_yaw(self)
	static int l_get_yaw(lua_State *L);

	// get_pitch(self)
	static int l_get_pitch(lua_State *L);	

	// get_hp(self)
	static int l_get_hp(lua_State *L);

	// get_name(self)
	static int l_get_name(lua_State *L);

	// get_wield_index(self)
	static int l_get_wield_index(lua_State *L);

	// get_wielded_item(self)
	static int l_get_wielded_item(lua_State *L);

	// get_hotbar_size(self)
	static int l_get_hotbar_size(lua_State *L);

	static int l_is_attached(lua_State *L);
	static int l_is_touching_ground(lua_State *L);
	static int l_is_in_liquid(lua_State *L);
	static int l_is_in_liquid_stable(lua_State *L);
	static int l_is_climbing(lua_State *L);
	static int l_swimming_vertical(lua_State *L);

	static int l_get_physics_override(lua_State *L);
	static int l_set_physics_override(lua_State *L);

	static int l_get_override_pos(lua_State *L);

	static int l_get_last_pos(lua_State *L);
	static int l_get_last_velocity(lua_State *L);
	static int l_get_last_look_vertical(lua_State *L);
	static int l_get_last_look_horizontal(lua_State *L);

	// get_control(self)
	static int l_get_control(lua_State *L);

	// get_breath(self)
	static int l_get_breath(lua_State *L);

	// get_pos(self)
	static int l_get_pos(lua_State *L);


	// get_movement_acceleration(self)
	static int l_get_movement_acceleration(lua_State *L);

	// get_movement_speed(self)
	static int l_get_movement_speed(lua_State *L);

	// get_movement(self)
	static int l_get_movement(lua_State *L);

	// get_armor_groups(self)
	static int l_get_armor_groups(lua_State *L);

	// hud_add(self, id, form)
	static int l_hud_add(lua_State *L);

	// hud_rm(self, id)
	static int l_hud_remove(lua_State *L);

	// hud_change(self, id, stat, data)
	static int l_hud_change(lua_State *L);
	// hud_get(self, id)
	static int l_hud_get(lua_State *L);
	// hud_get_all(self)
	static int l_hud_get_all(lua_State *L);

	static int l_get_move_resistance(lua_State *L);

	//cheats n stuff

	static LocalPlayer *getobject(LuaLocalPlayer *ref);
	static LocalPlayer *getobject(lua_State *L, int narg);

	LocalPlayer *m_localplayer = nullptr;

	// set_pos(self, pos)
	static int l_set_pos(lua_State *L);

	// set_velocity(self, vel)
	static int l_set_velocity(lua_State *L);
	
	// set_yaw(self, yaw)
	static int l_set_yaw(lua_State *L);
	
	// set_pitch(self, pitch)
	static int l_set_pitch(lua_State *L);
	
	// set_wield_index(self, index)
	static int l_set_wield_index(lua_State *L);

	// get_pointed_thing()
	static int l_get_pointed_thing(lua_State *L);

	// get_entity_relationship(self, object_id)
	static int l_get_entity_relationship(lua_State *L);

	// get_teamcolor(self)
	static int l_get_teamcolor(lua_State *L);

	// punch(self, object_id)
	static int l_punch(lua_State *L);

	// get_time_from_last_punch(self)
	static int l_get_time_from_last_punch(lua_State *L);
	
	// get_object(self)
	static int l_get_object(lua_State *L);

	// set_lua_control(self, control)
	static int l_set_lua_control(lua_State *L);

public:
	LuaLocalPlayer(LocalPlayer *m);
	~LuaLocalPlayer() = default;

	static void create(lua_State *L, LocalPlayer *m);

	static void Register(lua_State *L);

	static const char className[];
};
