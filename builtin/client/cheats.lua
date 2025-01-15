core.cheats = {
	["Combat"] = {
		["AntiKnockback"] = "antiknockback",
		["AttachmentFloat"] = "float_above_parent",
	},
	["Movement"] = {
		["Freecam"] = "freecam",
		["AutoForward"] = "continuous_forward",
		["PitchMove"] = "pitch_move",
		["AutoJump"] = "autojump",
		["Flight"] = "free_move",
		["Noclip"] = "noclip",
		["FastMove"] = "fast_move",
		["Jesus"] = "jesus",
		["NoSlow"] = "no_slow",
		["JetPack"] = "jetpack",
		["AntiSlip"] = "antislip",
		["AirJump"] = "airjump",
		["Spider"] = "spider",
		["AutoSneak"] = "autosneak",
		["BunnyHop"] = "BHOP",
	},
	["Render"] = {
		["Xray"] = "xray",
		["Fullbright"] = "fullbright",
		["HUDBypass"] = "hud_flags_bypass",
		["NoHurtCam"] = "no_hurt_cam",
		["Coords"] = "coords",
		["CheatHUD"] = "cheat_hud",
		["EntityESP"] = "enable_entity_esp",
		["EntityTracers"] = "enable_entity_tracers",
		["PlayerESP"] = "enable_player_esp",
		["PlayerTracers"] = "enable_player_tracers",
		["NodeESP"] = "enable_node_esp",
		["NodeTracers"] = "enable_node_tracers",
		["TunnelESP"] = "enable_tunnel_esp",
		["TunnelTracers"] = "enable_tunnel_tracers",
		["NoRender"] = "norender.particles",
        ["NoDrownCam"] = "small_post_effect_color", 
        ["BrightNight"] = "no_night", 
		["XrayNodes"] = "XrayNodes",
		["ESPNodes"] = "ESPNodes",
	},
	["Interact"] = {
		["FastDig"] = "fastdig",
		["FastPlace"] = "fastplace",
		["AutoDig"] = "autodig",
		["AutoPlace"] = "autoplace",
		["InstantBreak"] = "instant_break",
		["FastHit"] = "spamclick",
		["AutoHit"] = "autohit",
		["AutoTool"] = "autotool",
	},
	["Player"] = {
		["NoFallDamage"] = "prevent_natural_damage",
		["NoForceRotate"] = "no_force_rotate",
		["Reach"] = "reach",
		--["PointLiquids"] = "point_liquids",
		["AutoRespawn"] = "autorespawn",
		--["ThroughWalls"] = "dont_point_nodes",
        ["PrivBypass"] = "priv_bypass",
        ["QuickMenu"] = "use_old_menu",
        ["NoViewBob"] = "nobob",
	},
	["World"] = {
		["Silence"] = "silence",

	},
	["Exploits"] = {
		["ShowFriends"] = "show_friends_nametag",
	}
}

core.infotexts = {}

function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end

function core.register_cheat_with_infotext(cheatname, category, func, infotext)
	core.infotexts[category] = core.infotexts[category] or {}
	core.infotexts[category][cheatname] = infotext
	core.register_cheat(cheatname, category, func)
end

function core.update_infotext(cheatname, category, func, infotext)
	core.infotexts[category] = core.infotexts[category] or {}
	core.infotexts[category][cheatname] = infotext
	core.update_infotexts()
end

core.cheat_settings = {}

function core.register_cheat_setting(setting_name, parent_category, parent_setting, setting_id, setting_data)
	 --settingname is the formatted setting name, e.g "Assist Mode"
	 --parent_category is the category of the parent setting, e.g "Combat", 
	 --parent_setting is the cheat this setting is for, e.g "autoaim", 
	 --setting_id is the setting string, e.g "autoaim.mode", 
	 --setting_data is the setting table, e.g 
	 --if its a bool,         {type="bool"}
	 --if its an int slider,  {type="slider_int", min=0, max=10, steps=10}
	 --if its a float slider, {type="slider_float", min=0.0, max=10.0, steps=100}
     --if its a text field,   {type="text", size=10}
	 --if its a selectionbox, {type="selectionbox", options={"lock", "assist"}}
	core.cheat_settings[parent_category] = core.cheat_settings[parent_category] or {}
	core.cheat_settings[parent_category][parent_setting] = core.cheat_settings[parent_category][parent_setting] or {}

	core.cheat_settings[parent_category][parent_setting][setting_id] = {
        name = setting_name,
        type = setting_data.type,
        min = setting_data.min,
        max = setting_data.max,
        steps = setting_data.steps,
        size = setting_data.size,
		options = setting_data.options
    }
end

--core.register_cheat_setting("Assist Mode", "Combat", "autoaim", "autoaim.mode", {type="selectionbox", options={"lock", "assist"}})
--core.register_cheat_setting("FastMove Speed", "Movement", "fast_move", "fast_move.speed", {type="slider_float", min=0.0, max=10.0, steps=100})
--core.register_cheat_setting("AutoHit Speed", "Interact", "autohit", "autohit.speed", {type="slider_int", min=0, max=10, steps=10})
--core.register_cheat_setting("XRay Nodelist", "Render", "xray", "xray.nodes", {type="text", size=10})
--core.register_cheat_setting("FastDig Instant", "Interact", "fastdig", "fastdig.instant", {type="bool"})
--core.setPlayerColorESP("FoxLoveFire", {r = 255, g = 0, b = 255})