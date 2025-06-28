local scriptpath = core.get_builtin_path()
local clientpath = scriptpath.."client"..DIR_DELIM
local commonpath = scriptpath.."common"..DIR_DELIM
local cheatspath = scriptpath.."cheats"..DIR_DELIM

local builtin_shared = {}

assert(loadfile(commonpath .. "register.lua"))(builtin_shared)
assert(loadfile(clientpath .. "register.lua"))(builtin_shared)
dofile(commonpath .. "after.lua")
dofile(commonpath .. "mod_storage.lua")
dofile(commonpath .. "chatcommands.lua")
dofile(commonpath .. "information_formspecs.lua")
dofile(clientpath .. "chatcommands.lua")
dofile(clientpath .. "misc.lua")
dofile(clientpath .. "cheats.lua")
dofile(clientpath .. "util.lua")
dofile(clientpath .. "wasplib.lua")
dofile(cheatspath .. "combat.lua")
dofile(cheatspath .. "physics_override.lua")
dofile(cheatspath .. "worldutil.lua")
dofile(cheatspath .. "misc.lua")
dofile(cheatspath .. "combatlog.lua")
dofile(cheatspath .. "autoteam.lua")
dofile(cheatspath .. "flagaura.lua")
dofile(cheatspath .. "autototem.lua")
dofile(cheatspath .. "inventory.lua")
dofile(cheatspath .. "strata.lua")

assert(loadfile(commonpath .. "item_s.lua"))({}) -- Just for push/read node functions
