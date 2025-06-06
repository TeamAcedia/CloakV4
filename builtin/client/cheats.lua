-- idk im trying some stuff
core.cheats = {
	["Render"] = {
		["CheatHUD"] = "cheat_hud",
    }
}

function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end