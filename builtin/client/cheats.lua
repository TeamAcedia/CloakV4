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
		["NoHurtCam"] = "no_hurt_cam",
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
    }
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
	add_tracers()
	update_corner_colors(1)
	core.after(0.5, function()
		core.localplayer:set_lua_control({
			up = true
		})
		walk_rectangle(1)
	end)
end)
