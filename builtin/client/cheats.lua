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
--		["HealthESP"] = "enable_health_esp",
		["PlayerTracers"] = "enable_player_tracers",
		["TargetHUD"] = "enable_combat_target_hud",
		["NodeESP"] = "enable_node_esp",
		["NodeTracers"] = "enable_node_tracers",
		["TunnelESP"] = "enable_tunnel_esp",
		["TunnelTracers"] = "enable_tunnel_tracers",
		["NoRender"] = "norender.particles",
        ["NoDrownCam"] = "small_post_effect_color", 
        ["BrightNight"] = "no_night",
		["FOV"] = "fov_setting",
	},
	["Interact"] = {
		["Blink"] = "blink",
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
	["Misc"] = {
		["Nametags"] = "use_colored_nametags",
		["AntiAFK"] = "anti_afk",
		["AntiAdmin"] = "autostaff",
		["Panic"] = "panic",
		["Hints"] = "use_hints",
	}
}

core.infotexts = {}
core.descriptions = {}

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

function core.register_cheat_description(cheatname, category, func, description)
	core.descriptions[category] = core.descriptions[category] or {}
	core.descriptions[category][cheatname] = description
	core.get_description()
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

--core.register_cheat_setting("Mode", "Combat", "autoaim", "autoaim.mode", {type="selectionbox", options={"Lock", "Assist"}})   i'll work on this later
core.register_cheat_setting("Type", "Render", "enable_health_esp", "enable_health_esp.type", {type="selectionbox", options={"Health Bar", "Above Head"}})
core.register_cheat_setting("Players Only", "Render", "enable_health_esp", "enable_health_esp.players_only", {type="bool"})
core.register_cheat_setting("Status color", "Render", "enable_entity_esp", "enable_entity_esp.custom_color", {type="bool"})
--core.register_cheat_setting("Speed", "Movement", "fast_move", "fast_move.speed", {type="slider_float", min=1.0, max=10.0, steps=100})
--core.register_cheat_setting("Speed", "Interact", "autohit", "autohit.speed", {type="slider_int", min=1, max=10, steps=10})
core.register_cheat_setting("Nodelist", "Render", "xray", "xray.nodes", {type="text", size=10})
--core.register_cheat_setting("Instant", "Interact", "fastdig", "fastdig.instant", {type="bool"})
core.register_cheat_setting("Field Of View", "Render", "fov_setting", "fov.step", {type="slider_int", min=72, max=160, steps = 89});
core.register_cheat_setting("Multiplier", "Movement", "step", "step.mult", {type="slider_float", min=1.0, max=3.5, steps=6})
core.register_cheat_setting("Y Offset", "Render", "cheat_hud", "cheat_hud.offset", {type="slider_int", min=0, max=200, steps=41})
core.register_cheat_setting("Position", "Render", "cheat_hud", "cheat_hud.position", {type="selectionbox", options={"Top", "Bottom"}})

-- Some cheats with infotexts
core.register_cheat_with_infotext("Step", "Movement", "step", "Mult: 0")
core.register_cheat_with_infotext("HealthESP", "Render", "enable_health_esp", "")
core.register_cheat_with_infotext("Blink", "Interact", "blink", "0ms")


--Combat
core.register_cheat_description("AntiKnockback", "Combat", "antiknockback", "Ignore knockback")
core.register_cheat_description("AttachmentFloat", "Combat", "float_above_parent", "Puts the camera one node higher when attached to an entity")
core.register_cheat_description("AutoTotem", "Combat", "autototem", "Automatically puts a totem in your offhand")
core.register_cheat_description("AutoAim", "Combat", "autoaim", "Aims at a specified target")
core.register_cheat_description("CombatLog", "Combat", "be_a_bitch", "Logs off when certain HP is reached")
core.register_cheat_description("Criticals", "Combat", "critical_hits", "Does critical hits in mcl2/mcla")
core.register_cheat_description("CrystalSpam", "Combat", "crystalspam", "Puts end crystals under the nearest player")
core.register_cheat_description("Killaura", "Combat", "killaura", "Attacks a specified target")
core.register_cheat_description("Orbit", "Combat", "orbit", "Moves around a specified target")
--Interact
core.register_cheat_description("FastDig", "Interact", "fastdig", "No block break cooldown")
core.register_cheat_description("Blink", "Interact", "blink", "Delay sending of packets until this cheat is disabled.")
core.register_cheat_description("FastPlace", "Interact", "fastplace", "No block placement cooldown")
core.register_cheat_description("AutoDig", "Interact", "autodig", "Player can dig blocks without mouse press")
core.register_cheat_description("AutoPlace", "Interact", "autoplace", "Auto place blocks")
core.register_cheat_description("InstantBreak","Interact", "instant_break", "Instantly break blocks regardless of tool used")
core.register_cheat_description("FastHit", "Interact", "spamclick", "Hit faster while holding")
core.register_cheat_description("AutoHit","Interact", "autohit", "Auto hit when looking at entity")
core.register_cheat_description("AutoTool", "Interact", "autotool", "Selects the best tool for an action")
--Inventory
core.register_cheat_description("Enderchest", "Inventory", minetest.open_enderchest, "Preview enderchest content in mcl/mcla")
core.register_cheat_description("Hand", "Inventory", minetest.open_handslot, "Open hand formspec in mcl/mcla")
--Misc
core.register_cheat_description("Anti AFK", "Misc", "anti_afk", "Prevent afk by moving")
core.register_cheat_description("Auto Staff", "Misc", "autostaff", "Automatically check player privs and assign them as a staff.")
core.register_cheat_setting("Warn Staff", "Misc", "autostaff", "autostaff.warn_staff", {type="bool"})
core.register_cheat_description("Auto Team", "Misc", "autoteam", "Sets allied players to your team in ctf")
core.register_cheat_description("Nametags", "Misc", "use_colored_nametags", "Sets player nametag colors based on their friend/enemy status")
core.register_cheat_description("Panic", "Misc", "panic", "Disables all detectable cheats")
core.register_cheat_description("Hints", "Misc", "use_hints", "Enable cheat hints")
--Movement
core.register_cheat_description("AirJump", "Movement", "airjump", "Jump on air")
core.register_cheat_description("AntiSlip", "Movement", "antislip", "Walk on slippery blocks without slipping")
core.register_cheat_description("AutoForward", "Movement", "continuous_forward", "Walk forward automatically")
core.register_cheat_description("AutoJump", "Movement", "autojump", "Jump automatically")
core.register_cheat_description("AutoSneak", "Movement", "autosneak", "Always sneak")
core.register_cheat_description("BunnyHop", "Movement", "BHOP", "No jump acceleration time and always jump")
core.register_cheat_description("FastMove", "Movement", "fast_move", "Toggle fast (req. PrivBypass)")
core.register_cheat_description("Flight", "Movement", "free_move", "Toggle flight (req. PrivBypass)")
core.register_cheat_description("Freecam", "Movement", "freecam", "Spectator mode")
core.register_cheat_description("Jesus", "Movement", "jesus", "Walk on liquids")
core.register_cheat_description("JetPack", "Movement", "jetpack", "AirJump but you fall after jumping")
core.register_cheat_description("NoSlow", "Movement", "noslow", "Sneaking doesn't slow you down")
core.register_cheat_description("Noclip", "Movement", "noclip", "Walk through walls (req. PrivBypass & Flight)")
core.register_cheat_description("Overrides", "Movement", "overrides", "Movement overrides")
core.register_cheat_description("PitchMove", "Movement", "pitch_move", "While flying, you move where you're pointing")
core.register_cheat_description("Spider", "Movement", "spider", "Climb walls")
core.register_cheat_description("Step", "Movement", "step", "Climbs the block you're facing")
core.register_cheat_description("Velocity", "Movement", "velocity", "Various velocity overrides")
--Player
core.register_cheat_description("AutoRespawn", "Player", "autorespawn", "Respawn after dying")
core.register_cheat_description("NoFallDamage", "Player", "prevent_natural_damage", "Receive no fall damage")
core.register_cheat_description("NoForceRotate", "Player", "noforcerotate", "Prevent server from changing the player's view direction")
core.register_cheat_description("NoViewBob", "Player", "nobob", "Disable view bobbing")
core.register_cheat_description("PrivBypass", "Player", "priv_bypass", "Bypass fly, noclip, fast and wireframe rendering")
core.register_cheat_description("QuickMenu", "Player", "use_old_menu", "Add a menu for quicker access to cheats")
core.register_cheat_description("Reach", "Player", "reach", "Increase reach")
--Render
core.register_cheat_description("BrightNight", "Render", "no_night", "Always daytime")
core.register_cheat_description("CheatHUD", "Render", "cheathud", "List enabled cheats")
core.register_cheat_description("Coords", "Render", "coords", "Render coordinates in the bottom left corner")
core.register_cheat_description("EntityESP", "Render", "enable_entity_esp", "See entities through walls")
core.register_cheat_description("EntityTracers", "Render", "enable_entity_tracers", "Draw tracers to entities")
core.register_cheat_description("Fullbright", "Render", "fullbright", "No darkness")
core.register_cheat_description("HUDBypass", "Render", "hudbypass", "Allows player to toggle hud elements disabled by the game")
core.register_cheat_description("HealthESP", "Render", "show_players_hp", "Shows player and entities' HP")
core.register_cheat_description("NoDrownCam", "Render", "small_post_effect_color", "Disables drowning camera effect")
core.register_cheat_description("NoHurtCam", "Render", "no_hurt_cam", "Disables hurt camera effect")
core.register_cheat_description("NoRender", "Render", "norender.particles", "Don't render particles")
core.register_cheat_description("NodeESP", "Render", "enable_node_esp", "See specified nodes through walls")
core.register_cheat_description("NodeTracers", "Render", "enable_node_tracers", "Draw tracers to specified nodes")
core.register_cheat_description("PlayerESP", "Render", "enable_player_esp", "See players through walls")
core.register_cheat_description("PlayerTracers", "Render", "enable_player_tracers", "Draw tracers to players")
core.register_cheat_description("TunnelESP", "Render", "enable_tunnel_esp", "See tunnels through walls")
core.register_cheat_description("TunnelTracers", "Render", "enable_tunnel_tracers", "Draw tracers to tunnels")
core.register_cheat_description("Xray", "Render", "xray", "Don't render specific nodes")
core.register_cheat_description("TargetHUD", "Render", "enable_combat_target_hud", "Show currently attacking target on HUD")
--World
core.register_cheat_description("AutoTNT", "World", "autotnt", "Puts TNT on the ground")
core.register_cheat_description("BlockLava", "World", "blocklava", "Replace lava with the block you're holding")
core.register_cheat_description("BlockWater", "World", "blockwater", "Replace water with the block you're holding")
core.register_cheat_description("Replace", "World", "replace", "When you break a block it gets replaced by the block you're holding")
core.register_cheat_description("Scaffold", "World", "scaffold", "Puts blocks below you")
core.register_cheat_description("ScaffoldPlus", "World", "scaffoldplus", "Puts even more blocks under you")
core.register_cheat_description("Silence", "World", "silence", "Disables sound")


-- continue with cheat description


-- Globalstep for infotexts
local update_interval = 0.25
local timer = 0
local blinktime = 0

minetest.register_globalstep(function(dtime)
    timer = timer + dtime

	if core.settings:get_bool("blink") then
		blinktime = blinktime + dtime
		core.update_infotext("Blink", "Interact", "blink", math.floor(blinktime * 1000) .. "ms")
		if blinktime > 10 then
			core.settings:set_bool("blink", false)
		end
	else
		blinktime = 0
	end
	
    if timer >= update_interval then
        timer = 0

        --Starts here

		--Step infotext
		local step_mult = minetest.settings:get("step.mult")
		core.update_infotext("Step", "Movement", "step", "Mult: " .. step_mult)
		core.update_infotext("Auto Heal", "Misc", "auto_heal", core.settings:get("auto_heal.hp") .. " HP")

		--CombatLog infotext
		local combatlog_hp = minetest.settings:get("be_a_bitch.hp")
		minetest.update_infotext("CombatLog", "Combat", "be_a_bitch", "Min HP:"  .. combatlog_hp)

		--HealthESP infotext
		if core.settings:get("enable_health_esp.type") == "Health Bar" then
			core.update_infotext("HealthESP", "Render", "show_players_hp", "Health Bar")
		else
			core.update_infotext("HealthESP", "Render", "show_players_hp", "Above head")
		end

		--AppleAura infotext
		if minetest.settings:get_bool("appleaura") then
			core.update_infotext("AppleAura", "Misc", "appleaura", tonumber(minetest.settings:get("appleaura.range")))
		end
		--Ends here
    end
end)

--How did you know I like adding comments?
