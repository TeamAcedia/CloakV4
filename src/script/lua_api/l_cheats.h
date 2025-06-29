#ifndef L_CHEATS_H
#define L_CHEATS_H

#include "lua_api/l_base.h"
#include "lua_api/l_internal.h"
#include "serverenvironment.h"
#include "remoteplayer.h"


class l_cheats: public ModApiBase
{
public:
    static void Initialize(lua_State *L, int top);
private:
static int l_setPlayerColorESP(lua_State *L);
static int l_set_target_esp_color(lua_State *L);
static int l_clear_combat_target(lua_State *L);
static int l_set_combat_target(lua_State *L);
};

#endif // L_CHEATS_H