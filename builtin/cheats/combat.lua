-------------- Killaura -----------------

core.register_cheat_with_infotext("Killaura", "Combat", "killaura", "")
core.register_cheat_setting("Target Mode", "Combat", "killaura", "targeting.target_mode", {type="selectionbox", options={"Nearest", "Lowest HP", "Highest HP"}})
core.register_cheat_setting("Target Type", "Combat", "killaura", "targeting.target_type", {type="selectionbox", options={"Players", "Entities", "Both"}})
core.register_cheat_setting("Distance", "Combat", "killaura", "targeting.distance", {type="slider_int", min=1, max=10, steps=10})
core.register_cheat_setting("Enemies Only", "Combat", "killaura", "targeting.enemies_only", {type="bool"})
core.register_cheat_setting("Line Of Sight", "Combat", "killaura", "killaura.lineofsight", {type="bool"})
core.register_cheat_setting("Assist", "Combat", "killaura", "killaura.assist", {type="bool"})
core.register_cheat_setting("Many Punches", "Combat", "killaura", "killaura.manypunches", {type="bool"})

-------------- Auto Aim -----------------

core.register_cheat_with_infotext("AutoAim", "Combat", "autoaim", "")
core.register_cheat_setting("Target Mode", "Combat", "autoaim", "targeting.target_mode", {type="selectionbox", options={"Nearest", "Lowest HP", "Highest HP"}})
core.register_cheat_setting("Target Type", "Combat", "autoaim", "targeting.target_type", {type="selectionbox", options={"Players", "Entities", "Both"}})
core.register_cheat_setting("Distance", "Combat", "autoaim", "targeting.distance", {type="slider_int", min=1, max=10, steps=10})
core.register_cheat_setting("Enemies Only", "Combat", "autoaim", "targeting.enemies_only", {type="bool"})
core.register_cheat_setting("Y offset", "Combat", "autoaim", "autoaim.y_offset", {type="slider_int", min=-5, max=10, steps=16})

--------------- Orbit -------------------

local qtime = 0

core.register_cheat_with_infotext("Orbit", "Combat", "orbit", "Nearest")
core.register_cheat_setting("Target Mode", "Combat", "orbit", "targeting.target_mode", {type="selectionbox", options={"Nearest", "Lowest HP", "Highest HP"}})
core.register_cheat_setting("Target Type", "Combat", "orbit", "targeting.target_type", {type="selectionbox", options={"Players", "Entities", "Both"}})
core.register_cheat_setting("Distance", "Combat", "orbit", "targeting.distance", {type="slider_int", min=1, max=10, steps=10})
core.register_cheat_setting("Radius", "Combat", "orbit", "orbit.radius", {type="slider_float", min=1, max=3, steps=5})
core.register_cheat_setting("Enemies Only", "Combat", "orbit", "targeting.enemies_only", {type="bool"})

--------------- TPAura -------------------

core.register_cheat_with_infotext("TP Aura", "Combat", "tpaura", "Nearest")
core.register_cheat_setting("Target Mode", "Combat", "tpaura", "targeting.target_mode", {type="selectionbox", options={"Nearest", "Lowest HP", "Highest HP"}})
core.register_cheat_setting("Target Type", "Combat", "tpaura", "targeting.target_type", {type="selectionbox", options={"Players", "Entities", "Both"}})
core.register_cheat_setting("Distance", "Combat", "tpaura", "tpaura.distance", {type="slider_int", min=1, max=20, steps=20})
core.register_cheat_setting("TP Delay", "Combat", "tpaura", "tpaura.delay", {type="slider_float", min=0, max=1, steps=11})
core.register_cheat_setting("Enemies Only", "Combat", "tpaura", "targeting.enemies_only", {type="bool"})
--------------- TriggerBot -------------------
core.register_cheat("TriggerBot", "Combat", "tbot")

--------------- Functions -------------------
function is_valid_target(obj, target_type, max_distance, ppos)
    local target_pos = obj:get_pos()
    local distance = vector.distance(ppos, target_pos)
    if distance > max_distance then
        return false
    end

    local is_player = obj:is_player()

    local type_check =
        (target_type == "Entities" and not is_player) or
        (target_type == "Players" and is_player) or
        (target_type == "Both")
	
    if not type_check then
        return false
    end

	if obj:is_local_player() then
        return false
    end

    if is_player then
        local relationship = core.localplayer:get_entity_relationship(obj:get_id())
        if relationship ~= core.EntityRelationship.ENEMY and core.settings:get_bool("targeting.enemies_only") then
            return false
        end
    end

    if obj:get_hp() <= 0 then
        return false
    end

    if not core.can_attack(obj:get_id()) then
        return false
    end

    return true
end


function get_best_target(objects, target_mode, target_type, max_distance, player)
    local best_target = nil
    local best_value = target_mode == "Highest HP" and 0 or math.huge -- Initialize based on mode
	local ppos = player:get_pos()
    for _, obj in ipairs(objects) do
        if is_valid_target(obj, target_type, max_distance, ppos) then
            local value = target_mode == "Nearest" and vector.distance(ppos, obj:get_pos()) or obj:get_hp()
            local is_better = (target_mode == "Nearest" and value < best_value) or
                              (target_mode == "Lowest HP" and value < best_value) or
                              (target_mode == "Highest HP" and value > best_value)

            if is_better then
                best_value = value
                best_target = obj
            end
        end
    end

    return best_target
end

function get_punch_interval(player)
    local interval_str = core.settings:get("punch_interval")
    local interval
    if interval_str == "auto" then
        interval = player:get_wielded_item():get_tool_capabilities().full_punch_interval + 0.1
    else
        interval = tonumber(interval_str) or 0.1
    end

    if interval <= 0 then
        interval = 0.1
    end

    if core.settings:get_bool("spamclick") then
        local multiplier = tonumber(core.settings:get("spamclick_multiplier")) or 6
        interval = interval / multiplier
    end

    return interval
end

local function get_send_speed_impl(critspeed)
    if core.settings:get_bool("critical_hits") then 
        critspeed.y = -7
    end
    return critspeed
end

local callable = {}

setmetatable(callable, {
    __call = function(_, critspeed)
        return get_send_speed_impl(critspeed)
    end
})

core.get_send_speed = callable


function extendPoint(yaw, distance)
	local radYaw = math.rad(yaw)
	local dir = vector.new(math.cos(radYaw), 0, math.sin(radYaw))
	dir = vector.multiply(dir, distance)
	return dir
end


function get_sine_color(delta_time)
    local wave_speed = 2.0 -- Controls the speed of the color wave
    local intensity = 127  -- Controls the intensity (how bright the color is)

    local r = math.floor(127 * math.sin(delta_time * wave_speed) + intensity)
    local g = math.floor(127 * math.sin(delta_time * wave_speed + math.pi / 3) + intensity)
    local b = math.floor(127 * math.sin(delta_time * wave_speed + 2 * math.pi / 3) + intensity)

	r = math.max(0, math.min(255, r))
    g = math.max(0, math.min(255, g))
    b = math.max(0, math.min(255, b))

    return r, g, b
end


total_time = 0


-- Helper function: Truncate a number to one decimal place.
local function truncate_to_one_decimal(num)
    local integer_part = math.modf(num * 10)
    return integer_part / 10
end

-- Helper function: Truncate each component of a position vector.
local function truncate_pos(pos)
    return {
        x = truncate_to_one_decimal(pos.x),
        y = truncate_to_one_decimal(pos.y),
        z = truncate_to_one_decimal(pos.z)
    }
end

-- Returns true if there is a solid block between pos1 and pos2.
local function is_block_between(pos1, pos2, step)
    step = step or 0.5  -- How far apart to sample along the ray
    local diff = vector.subtract(pos2, pos1)
    local total_distance = vector.length(diff)
    local direction = vector.normalize(diff)
    local steps = math.floor(total_distance / step)

    for i = 1, steps do
        local sample_pos = vector.add(pos1, vector.multiply(direction, i * step))
        local node = minetest.get_node_or_nil(sample_pos)
        -- Consider only nodes that are not air or ignore
        --if node and node.name ~= "air" and node.name ~= "ignore" then
        if node and not string.find(node.name, "air", 1, true) and 
        not string.find(node.name, "ignore", 1, true) and
        not string.find(node.name, "water", 1, true)then
            --local node_def = minetest.registered_nodes[node.name]
            -- Check if the node is considered "solid" (walkable), you might
            -- adjust this logic depending on what you consider a blockage.
           -- if node_def and node_def.walkable then
                return true -- Solid block found along the path.
            --end
        end
    end

    return false -- No solid blocks found.
end

core.register_globalstep(function(dtime)
	total_time = total_time + dtime
	local r, g, b = get_sine_color(total_time)
	core.set_target_esp_color({r = r, g = g, b = b})
	if core.settings:get_bool("killaura") then
		local target_mode = core.settings:get("targeting.target_mode")
		local mode_text = target_mode and target_mode:gsub(" HP", "") or "Unknown"
	
		local target_description = core.settings:get("targeting.target_type")
		core.update_infotext("Killaura", "Combat", "killaura", string.format("%s, %s", mode_text, target_description))
	end

	if core.settings:get_bool("autoaim") then
		local target_mode = core.settings:get("targeting.target_mode")
		local mode_text = target_mode and target_mode:gsub(" HP", "") or "Unknown"
	
		local target_description = core.settings:get("targeting.target_type")
		core.update_infotext("AutoAim", "Combat", "autoaim", string.format("%s, %s", mode_text, target_description))
	end

	if core.settings:get_bool("orbit") then
		local target_mode = core.settings:get("targeting.target_mode")
		local mode_text = target_mode and target_mode:gsub(" HP", "") or "Unknown"
	
		local target_description = core.settings:get("targeting.target_type")
		core.update_infotext("Orbit", "Combat", "orbit", string.format("%s, %s", mode_text, target_description))
	end

	if core.settings:get_bool("tpaura") then
		local target_mode = core.settings:get("targeting.target_mode")
		local mode_text = target_mode and target_mode:gsub(" HP", "") or "Unknown"
	
		local target_description = core.settings:get("targeting.target_type")
		core.update_infotext("TP Aura", "Combat", "tpaura", string.format("%s, %s", core.settings:get("tpaura.distance"), core.settings:get("tpaura.delay")))
	end

    local player = core.localplayer
	if not player then return end
    local ppos = core.localplayer:get_pos()
	local target_enemy = nil
	
	if core.settings:get("targeting.target_mode") then
		local target_mode = core.settings:get("targeting.target_mode")
		local target_type = core.settings:get("targeting.target_type")
		local max_distance = tonumber(core.settings:get("targeting.distance")) + 0.5
		if core.settings:get_bool("tpaura") then
			max_distance = tonumber(core.settings:get("tpaura.distance")) + 0.5
		end
		local objects = core.get_nearby_objects(max_distance)
		target_enemy = get_best_target(objects, target_mode, target_type, max_distance, player)
	end
	if not target_enemy then 
		core.clear_combat_target() 
	else 	
		core.set_combat_target(target_enemy:get_id()) 	
	end

	if target_enemy and (core.settings:get_bool("killaura")) then
		
		local interval = get_punch_interval(player)

		if player:get_time_from_last_punch() > interval or core.settings:get_bool("killaura.manypunches") then
		
			if (core.settings:get_bool("killaura.lineofsight") and target_enemy ) then
			
				local pos = truncate_pos(player:get_pos())
				local enemy_pos = truncate_pos(target_enemy:get_pos())
			
				local has_los = not is_block_between(pos, enemy_pos, 1.0)
			
				if (not has_los) then
					return
				end
			end
		
			if (core.settings:get_bool("killaura.assist")) then
				local wield_index = player:get_wield_index() + 1
				local dmg = core.get_inv_item_damage(wield_index, target_enemy:get_id())
				if (dmg and target_enemy:get_hp() - dmg.damage <= 1) then
					return
				end
			end
			
			if (core.settings:get_bool("killaura.doubletap")) then
				local wield_index = player:get_wield_index() + 1
				local dmg = core.get_inv_item_damage(wield_index, target_enemy:get_id())
				local dmg2 = core.get_inv_item_damage(wield_index + 1, target_enemy:get_id())
				if ( (dmg and target_enemy:get_hp() - dmg.damage <= 1) and (dmg2 and target_enemy:get_hp() - dmg2.damage <= 1) ) then
				
					player:set_wield_index(wield_index + 1)
					player:punch(target_enemy:get_id())
					player:set_wield_index(wield_index)
					return true
				end
			end

			player:punch(target_enemy:get_id())
		end
	end
	

	if target_enemy and core.settings:get_bool("autoaim") then
		local enemyPos = target_enemy:get_pos();
		enemyPos.y = enemyPos.y - 1
		enemyPos.y = enemyPos.y + minetest.settings:get("autoaim.y_offset")/10
		ws.aim(enemyPos)
	end

	if target_enemy and core.settings:get_bool("orbit") then
		local opos = target_enemy:get_pos()

		local distance = tonumber(core.settings:get("orbit.radius")) - 0.5

		local dir=vector.direction(ppos,opos)
		local yyaw=0;
		if dir.x < 0 then
			yyaw = math.atan2(-dir.x, dir.z) + (math.pi * 2)
		else
			yyaw = math.atan2(-dir.x, dir.z)
		end
		yyaw = ws.round2(math.deg(yyaw),2)

		

		local target_pos = vector.add(extendPoint(yyaw-90, distance), opos)
		target_pos.y = ppos.y
		local temp_pos = vector.new(ppos.x, opos.y, ppos.z)
		local current_distance = vector.distance(ppos, opos)

		core.localplayer:set_pos(target_pos)

		local vec = vector.subtract(ppos, opos)
		local yaw = math.atan(vec.z / vec.x) - math.pi/2
		yaw = yaw + (opos.x >= ppos.x and math.pi or 0)
		local y = vec.y
		vec.y = 0
		local x = vector.length(vec)+90
		local enemyPos = target_enemy:get_pos();
		enemyPos.y = enemyPos.y - 1
		enemyPos.y = enemyPos.y + minetest.settings:get("autoaim.y_offset")/10
		ws.aim(enemyPos)
		Strata.set_controls({left = true, jump = true})
	else
		Strata.clear_controls()
	end
end)



core.register_cheat("Criticals", "Combat", "critical_hits")

core.register_chatcommand("fasthit", {
	params = "<multiplier>",
	description = "Set fasthit multiplier. Used for killaura ONLY when fasthit/spamclick is on.",
	func = function(param)
		local multiplierNum = tonumber(param)
		if multiplierNum then
			if multiplierNum > 0 then
				core.settings:set("spamclick_multiplier", tostring(multiplierNum))
				return true, "Fasthit multiplier has been set to: " .. tostring(multiplierNum) .. "."
			else
				return false, "Error: Input must be greater than 0."
			end
		else
			return false, "Error: Input must be a number."
		end
	end,
})


--core.register_cheat("DoubleTap", "Combat", "killaura.doubletap") -- working on how to implement this properly