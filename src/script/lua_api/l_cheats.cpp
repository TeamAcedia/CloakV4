#include "l_cheats.h"
#include "client/render/core.h"

int l_cheats::l_setPlayerColorESP(lua_State *L)
{

    size_t name_;
    const char* name = luaL_checklstring(L, 1, &name_);

    luaL_checktype(L, 2, LUA_TTABLE);

    lua_getfield(L, 2, "r");
    int r = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "g");
    int g = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "b");
    int b = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    RenderingCore::ESPplayersNames[name].push_back(r);
    RenderingCore::ESPplayersNames[name].push_back(g);
    RenderingCore::ESPplayersNames[name].push_back(b);

    return 1;
}

int l_cheats::l_set_target_esp_color(lua_State *L)
{

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "r");
    int r = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "g");
    int g = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "b");
    int b = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    RenderingCore::target_esp_color = video::SColor(255, r, g, b);

    return 1;
}

int l_cheats::l_set_combat_target(lua_State *L)
{

    uint16_t player_id = static_cast<uint16_t>(luaL_checkinteger(L, 1));

    RenderingCore::combat_target = player_id;

    return 0;
}

int l_cheats::l_clear_combat_target(lua_State *L)
{
    RenderingCore::combat_target = 0;

    return 0;
}


void l_cheats::Initialize(lua_State *L, int top)
{
    API_FCT(setPlayerColorESP);
    API_FCT(set_target_esp_color);
    API_FCT(set_combat_target);
    API_FCT(clear_combat_target);
}