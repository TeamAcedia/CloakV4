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
		["HealthESP"] = "enable_health_esp",
		["NoHurtCam"] = "no_hurt_cam",
		["NoDrownCam"] = "no_drown_cam",
		["NoParticles"] = "norender.particles",
		["TaskTracers"] = "enable_task_tracers",
		["TaskNodes"] = "enable_task_nodes",
		["DetachedCamera"] = "detached_camera",
    },
	["Player"] = {
		["PrivBypass"] = "priv_bypass",
		["NoFallDamage"] = "prevent_natural_damage",
		["Reach"] = "reach",
		["AutoRespawn"] = "autorespawn",
		["LuaControl"] = "lua_control",
		["NoForceRotate"] = "no_force_rotate",
		["QuickMenu"] = "use_old_menu",
		["NoViewBob"] = "nobob",
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
		["Step"] = "step",
    },
	["Combat"] = {
		["AntiKnockback"] = "antiknockback",
		["AttachmentFloat"] = "float_above_parent",
    },
	["Interact"] = {
		["FastHit"] = "spamclick",
		["AutoHit"] = "autohit",
		["FastPlace"] = "fastplace",
		["AutoPlace"] = "autoplace",
		["AutoDig"] = "autodig",
		["FastDig"] = "fastdig",
		["InstantBreak"] = "instant_break",
		["AutoTool"] = "autotool",
    },
	["Misc"] = {
		["Hints"] = "use_hints",
    },
}
-----------------------------------------------------------REGISTER CHEATS-----------------------------------------------------------
function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end
-----------------------------------------------------------CHEAT SETTINGS-----------------------------------------------------------
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
-----------------------------------------------------------CHEAT INFOTEXTS-----------------------------------------------------------
core.infotexts = {}


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
-----------------------------------------------------------CHEAT DESCRIPTIONS-----------------------------------------------------------
core.descriptions = {}

function core.register_cheat_with_description(cheatname, category, func, description)
	core.descriptions[category] = core.descriptions[category] or {}
	core.descriptions[category][cheatname] = description
	core.get_description()
end

function core.register_cheat_description(cheatname, category, func, description)
	core.descriptions[category] = core.descriptions[category] or {}
	core.descriptions[category][cheatname] = description
	core.get_description()
end
-----------------------------------------------------------PANIC-----------------------------------------------------------
function core.panic()
	for category_name, category in pairs(minetest.cheats) do
		for cheat_name, cheat in pairs(category) do
			local disable_cheats = minetest.cheats[category_name][cheat_name]
			if type(disable_cheats) == "string" then core.settings:set(disable_cheats, "false") end
		end
	end
end
core.register_cheat("Panic", "Misc", core.panic)
-----------------------------------------------------------TESTS, PRESET VALUES, ETC-----------------------------------------------------------
local pos_y = 0
local corners = {
	{x=0, y=pos_y, z=50},
	{x=2, y=pos_y, z=50},
	{x=2, y=pos_y, z=52},
	{x=0, y=pos_y, z=52}
}

local function add_tracers()
	for i = 1, #corners do
		local a = corners[i]
		local b = corners[i % #corners + 1]
		core.add_task_tracer(a, b, {r=0, g=0, b=255})
	end
end

local function update_corner_colors(current_index)
	for i = 1, #corners do
		local pos = corners[i]
		core.clear_task_node(pos)
		local color = (i == current_index) and {r=0, g=255, b=0} or {r=255, g=0, b=0}
		core.add_task_node(pos, color)
	end
end

local function round2(num, numDecimalPlaces)
    return tonumber(string.format("%." .. (numDecimalPlaces or 0) .. "f", num))
end

local function aim(tpos)
    local ppos=minetest.localplayer:get_pos()
    local dir=vector.direction(ppos,tpos)
    local yyaw=0;
    if dir.x < 0 then
        yyaw = math.atan2(-dir.x, dir.z) + (math.pi * 2)
    else
        yyaw = math.atan2(-dir.x, dir.z)
    end
    yyaw = round2(math.deg(yyaw),2)
    return yyaw
end

local function walk_to(pos, next_step_callback)
	local interval = 0.01
	local tolerance = 0.5
	local function step()
		local ppos = core.localplayer:get_pos()
		local dx = pos.x - ppos.x
		local dz = pos.z - ppos.z
		local dist = math.sqrt(dx * dx + dz * dz)

		if dist < tolerance then
			if next_step_callback then next_step_callback() end
			return
		end

		local yaw = aim(pos)
		core.localplayer:set_yaw(yaw)
		core.after(interval, step)
	end
	step()
end

local function walk_rectangle(index)
	if index > #corners then index = 1 end
	local next_index = (index % #corners) + 1

	update_corner_colors(next_index)

	walk_to(corners[next_index], function()
		core.after(0.01, function()
			walk_rectangle(next_index)
		end)
	end)
end

-- Main entry point
core.after(0.5, function()
	core.add_task_node({x=0, y=0, z=50}, {r=196, g=164, b=132})
	core.add_task_node({x=-1, y=0, z=50}, {r=196, g=164, b=132})
	core.add_task_node({x=1, y=0, z=50}, {r=196, g=164, b=132})
	core.add_task_node({x=0, y=1, z=50}, {r=196, g=164, b=132})
	core.add_task_node({x=0, y=2, z=50}, {r=196, g=164, b=132})
	core.add_task_tracer({x=0, y=2, z=50}, {x=0, y=6, z=50}, {r=255, g=255, b=255})
	core.add_task_tracer({x=0, y=6, z=50}, {x=1, y=7, z=50}, {r=255, g=255, b=255})	

local last_path = {}
local last_tracers = {}

-- Clear nodes and tracers from previous path
local function clear_old_path()
	for _, pos in ipairs(last_path) do
		core.clear_task_node(pos)
	end
	last_path = {}

	for _, tracer in ipairs(last_tracers) do
		core.clear_task_tracer(tracer[1], tracer[2])
	end
	last_tracers = {}
end

-- Draws path and records it
local function draw_path(path, reached_goal, target_pos)
	clear_old_path()
	last_path = path

	-- Always show start node (blue)
	core.add_task_node(start_pos, {r=0, g=0, b=255})

	if reached_goal then
		core.add_task_node(end_pos, {r=0, g=255, b=0}) -- End node green
	else
		local final = path[#path]
		core.add_task_node(final, {r=255, g=0, b=255}) -- Last reachable node purple
		core.add_task_node(target_pos, {r=0, g=255, b=0}) 
	end

	-- Tracers: green if complete path, purple if partial
	local tracer_color = reached_goal and {r=0, g=255, b=0} or {r=255, g=0, b=255}

	for i = 1, #path - 1 do
		local a = path[i]
		local b = path[i+1]
		core.add_task_tracer(a, b, tracer_color)
		table.insert(last_tracers, {a, b})
	end
end

-- Continuously updates the path
local function refresh_path_loop()
	local path = core.find_path(start_pos, end_pos)

	-- Determine if goal was reached (based on last node)
	local reached_goal = false
	if path and #path > 0 then
		local last = path[#path]
		reached_goal = vector.equals(last, end_pos)
	end

	if path and #path > 0 then
		draw_path(path, reached_goal, end_pos)
	else
		-- Path is completely empty
		clear_old_path()
		core.add_task_node(start_pos, {r=0, g=0, b=255})
		core.add_task_node(end_pos, {r=255, g=0, b=255}) -- Purple end
	end

	core.after(0.5, refresh_path_loop)
end

-- Start the refresh loop
--core.after(0.5, refresh_path_loop) todo: fix massive lag when joining minetest game

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
--core.register_cheat_description("Blink", "Interact", "blink", "Delay sending of packets until this cheat is disabled.")
core.register_cheat_description("FastPlace", "Interact", "fastplace", "No block placement cooldown")
core.register_cheat_description("AutoDig", "Interact", "autodig", "Player can dig blocks without mouse press")
core.register_cheat_description("AutoPlace", "Interact", "autoplace", "Auto place blocks")
core.register_cheat_description("InstantBreak","Interact", "instant_break", "Instantly break blocks regardless of tool used")
core.register_cheat_description("FastHit", "Interact", "spamclick", "Hit faster while holding")
core.register_cheat_description("AutoHit","Interact", "autohit", "Auto hit when looking at entity")
core.register_cheat_description("AutoTool", "Interact", "autotool", "Selects the best tool for an action")
--Inventory
core.register_cheat_description("Enderchest", "Misc", minetest.open_enderchest, "Preview enderchest content in mcl/mcla")
core.register_cheat_description("Hand", "Misc", minetest.open_handslot, "Open hand formspec in mcl/mcla")
--Misc
--core.register_cheat_description("Anti AFK", "Misc", "anti_afk", "Prevent afk by moving")
--core.register_cheat_description("AutoStaff", "Misc", "autostaff", "Automatically check player privs and assign them as a staff.")
--core.register_cheat_setting("Warn Staff", "Misc", "autostaff", "autostaff.warn_staff", {type="bool"})
core.register_cheat_description("AutoTeam", "Misc", "autoteam", "Sets allied players to your team in ctf")
--core.register_cheat_description("Nametags", "Misc", "use_colored_nametags", "Sets player nametag colors based on their friend/enemy status")
core.register_cheat_description("Panic", "Misc", "panic", "Disables all cheats")
core.register_cheat_description("Hints", "Misc", "use_hints", "Enable cheat hints")
core.register_cheat_description("Spammer", "Misc", "spammer", "Sends many chat messages")
--Movement
core.register_cheat_description("AirJump", "Movement", "airjump", "Jump on air")
core.register_cheat_description("AntiSlip", "Movement", "antislip", "Walk on slippery blocks without slipping")
core.register_cheat_description("AutoForward", "Movement", "continuous_forward", "Walk forward automatically")
core.register_cheat_description("AutoJump", "Movement", "autojump", "Jump automatically")
core.register_cheat_description("AutoSneak", "Movement", "autosneak", "Always sneak")
--core.register_cheat_description("BunnyHop", "Movement", "BHOP", "No jump acceleration time and always jump")
core.register_cheat_description("FastMove", "Movement", "fast_move", "Toggle fast (req. PrivBypass)")
core.register_cheat_description("Flight", "Movement", "free_move", "Toggle flight (req. PrivBypass)")
core.register_cheat_description("Freecam", "Movement", "freecam", "Spectator mode")
core.register_cheat_description("Jesus", "Movement", "jesus", "Walk on liquids")
core.register_cheat_description("JetPack", "Movement", "jetpack", "AirJump but you fall after jumping")
core.register_cheat_description("NoSlow", "Movement", "noslow", "Sneaking doesn't slow you down")
core.register_cheat_description("Noclip", "Movement", "noclip", "Walk through walls (req. PrivBypass)")
core.register_cheat_description("Overrides", "Movement", "overrides", "Movement overrides")
core.register_cheat_description("PitchMove", "Movement", "pitch_move", "While flying, you move where you're pointing")
core.register_cheat_description("Spider", "Movement", "spider", "Climb walls")
core.register_cheat_description("Step", "Movement", "step", "Climbs the block you're facing")
core.register_cheat_description("Velocity", "Movement", "velocity", "Various velocity overrides")
--Player
core.register_cheat_description("AutoRespawn", "Player", "autorespawn", "Respawn after dying")
core.register_cheat_description("NoFallDamage", "Player", "prevent_natural_damage", "Receive no fall damage")
core.register_cheat_description("NoForceRotate", "Player", "noforcerotate", "Prevent server from changing the player's view direction")
--core.register_cheat_description("NoViewBob", "Player", "nobob", "Disable view bobbing")
core.register_cheat_description("PrivBypass", "Player", "priv_bypass", "Bypass fly, noclip, fast and wireframe rendering")
core.register_cheat_description("QuickMenu", "Player", "use_old_menu", "Add a menu for quicker access to cheats")
core.register_cheat_description("Reach", "Player", "reach", "Increase reach")
core.register_cheat_description("LuaControl", "Player", "luacontrol", "The player moves regardless of the received input")
--Render
core.register_cheat_description("BrightNight", "Render", "no_night", "Always daytime")
core.register_cheat_description("CheatHUD", "Render", "cheathud", "List enabled cheats")
core.register_cheat_description("Coords", "Render", "coords", "Render coordinates in the bottom left corner")
core.register_cheat_description("EntityESP", "Render", "enable_entity_esp", "See entities through walls")
core.register_cheat_description("EntityTracers", "Render", "enable_entity_tracers", "Draw tracers to entities")
core.register_cheat_description("FullBright", "Render", "fullbright", "No darkness")
--core.register_cheat_description("HUDBypass", "Render", "hudbypass", "Allows player to toggle hud elements disabled by the game")
core.register_cheat_description("HealthESP", "Render", "show_players_hp", "Shows player and entity HP")
core.register_cheat_description("NoDrownCam", "Render", "no_drown_cam", "Disables drowning camera effect")
core.register_cheat_description("NoHurtCam", "Render", "no_hurt_cam", "Disables hurt camera effect")
core.register_cheat_description("NoParticles", "Render", "norender.particles", "Don't render particles")
core.register_cheat_description("NodeESP", "Render", "enable_node_esp", "See specified nodes through walls")
core.register_cheat_description("NodeTracers", "Render", "enable_node_tracers", "Draw tracers to specified nodes")
core.register_cheat_description("PlayerESP", "Render", "enable_player_esp", "See players through walls")
core.register_cheat_description("PlayerTracers", "Render", "enable_player_tracers", "Draw tracers to players")
--core.register_cheat_description("TunnelESP", "Render", "enable_tunnel_esp", "See tunnels through walls")
--core.register_cheat_description("TunnelTracers", "Render", "enable_tunnel_tracers", "Draw tracers to tunnels")
core.register_cheat_description("Xray", "Render", "xray", "Don't render specific nodes")
--core.register_cheat_description("TargetHUD", "Render", "enable_combat_target_hud", "Show currently attacking target on HUD")
--World
core.register_cheat_description("AutoTNT", "World", "autotnt", "Puts TNT on the ground")
core.register_cheat_description("BlockLava", "World", "blocklava", "Replace lava with the block you're holding")
core.register_cheat_description("BlockWater", "World", "blockwater", "Replace water with the block you're holding")
core.register_cheat_description("Replace", "World", "replace", "When you break a block it gets replaced by the block you're holding")
core.register_cheat_description("Scaffold", "World", "scaffold", "Puts blocks below you")
core.register_cheat_description("ScaffoldPlus", "World", "scaffoldplus", "Puts even more blocks under you")




--SOME SETTINGS

core.register_cheat_setting("Nodelist", "Render", "xray", "xray.nodes", {type="text", size=10})
core.register_cheat_setting("Nodelist", "Render", "enable_node_esp", "enable_node_esp.nodes", {type="text", size=10})
core.register_cheat_setting("Multiplier", "Movement", "step", "step.mult", {type="slider_float", min=1.0, max=3.5, steps=6})
core.register_cheat_setting("Y Offset", "Render", "cheat_hud", "cheat_hud.offset", {type="slider_int", min=0, max=200, steps=41})
core.register_cheat_setting("Position", "Render", "cheat_hud", "cheat_hud.position", {type="selectionbox", options={"Top", "Bottom"}})
core.register_cheat_setting("Type", "Render", "enable_health_esp", "enable_health_esp.type", {type="selectionbox", options={"Health Bar", "Above Head"}})
core.register_cheat_setting("Players Only", "Render", "enable_health_esp", "enable_health_esp.players_only", {type="bool"})


local update_interval = 0.25
local timer = 0

minetest.register_globalstep(function(dtime)
    timer = timer + dtime
    if timer >= update_interval then
        timer = 0
        --Starts here

		--Step infotext
		local step_mult = minetest.settings:get("step.mult")
		core.update_infotext("Step", "Movement", "step", "Mult: " .. core.settings:get("step.mult"))

		--CombatLog infotext
		local combatlog_hp = minetest.settings:get("be_a_bitch.hp")
		minetest.update_infotext("CombatLog", "Combat", "be_a_bitch", "Min HP:" .. core.settings:get("combatlog.hp"))

		--Ends here
    end
end)
