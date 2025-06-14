core.cheats = {
	["Render"] = {
		["CheatHUD"] = "cheat_hud",
		["FullBright"] = "fullbright",
		["BrightNight"] = "no_night",
		["Xray"] = "xray",
		["EntityESP"] = "enable_entity_esp",
		["EntityTracers"] = "enable_entity_tracers",
		["PlayerESP"] = "enable_player_esp",
		["PlayerTracers"] = "enable_player_tracers",
		["NodeESP"] = "enable_node_esp",
		["NodeTracers"] = "enable_node_tracers",
	--	["HUDBypass"] = "hud_flags_bypass", dont wanna work, will fix later
    },
	["Player"] = {
		["PrivBypass"] = "priv_bypass",
		["NoFallDamage"] = "prevent_natural_damage",
    },
	["Movement"] = {
		["Freecam"] = "freecam",
    },
	["Combat"] = {
		["AntiKnockback"] = "antiknockback",
		["AttachmentFloat"] = "float_above_parent",
    }
}

function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end