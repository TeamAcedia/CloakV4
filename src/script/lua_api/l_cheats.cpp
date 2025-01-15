#include "l_cheats.h"
#include "client/render/core.h"

int l_cheats::l_setPlayerColorESP(lua_State *L)
{
    GET_ENV_PTR;

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

void l_cheats::Initialize(lua_State *L, int top) 
{
    API_FCT(setPlayerColorESP);
}