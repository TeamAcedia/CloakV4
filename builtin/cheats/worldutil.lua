-- Credits to Lizzy Fleckenstein
-- lgpl license

core.register_on_dignode(function(pos)
	if core.settings:get_bool("replace") then
		core.after(0, core.place_node, pos)
	end
end)

local etime = 0
local uptime = 0
local hud_id = nil

core.register_globalstep(function(dtime)
	etime = etime + dtime
	if etime < 1 then return end
	local player = core.localplayer
	if not player then return end
	local pos = player:get_pos()
	local item = player:get_wielded_item()
	local def = core.get_node_def(item:get_name())
	local nodes_per_tick = tonumber(core.settings:get("nodes_per_tick")) or 8

	if item and item:get_count() > 0 and def and def.node_placement_prediction ~= "" and def.walkable == true and (core.settings:get_bool("scaffold") or core.settings:get_bool("scaffold_plus")) then
        local total_items = core.get_total_items(item:get_name())
        local player = core.localplayer
        if player then
            if hud_id then
                player:hud_change(hud_id, "text", total_items .. " " .. item:get_description() .. " left")
            else
                hud_id = player:hud_add({
                    type = "text",
                    position = {x = 0.5, y = 0.5},
                    offset = {x = 0, y = 25},
                    text = total_items .. " " .. item:get_description() .. " left",
                    alignment = {x = 0, y = 0},
                    scale = {x = 150, y = 150},
                    number = 0xF0F0F0
                })
            end
        end
    else
        if hud_id and core.localplayer then
            core.localplayer:hud_remove(hud_id)
            hud_id = nil
        end
    end
	if item and item:get_count() > 0 and def and def.node_placement_prediction ~= "" and def.walkable == true then
		if core.settings:get_bool("scaffold") or core.settings:get_bool("scaffold_plus") then
			core.settings:set_bool("scaffold.active", true)
			local control = core.localplayer and core.localplayer:get_control()
			uptime = uptime + dtime
			if control and control.jump then
				if uptime > tonumber(core.settings:get("scaffold.jump_delay")) / 2 then
					local node_above = core.get_node_or_nil(vector.round(vector.add(pos, {x = 0, y = 2.4, z = 0})))
					if node_above and node_above.name == "air" then
						uptime = 0
						pos = vector.add(pos, {x = 0, y = 1, z = 0})
						core.localplayer:set_pos(pos)
					end
				end
			else
				uptime = tonumber(core.settings:get("scaffold.jump_delay")) / 2
			end
		else
			core.settings:set_bool("scaffold.active", false)
		end

		if core.settings:get_bool("scaffold") then
			local p = vector.round(vector.add(pos, {x = 0, y = -0.6, z = 0}))
			local node = core.get_node_or_nil(p)
			if not node or core.get_node_def(node.name).buildable_to then
				core.place_node(p)
			end
		end
		if core.settings:get_bool("scaffold_plus") then
			local max_distance = 4    -- Maximum radius from the player
			local nodes_placed = 0

			local origin = vector.round(pos)

			local visited = {}
			local queue = {{x = 0, y = 0, z = 0}} -- Start from the origin
			local checked = {}

			while #queue > 0 and nodes_placed < nodes_per_tick do
				local offset = table.remove(queue, 1)
				local world_pos = vector.add(origin, offset)
                world_pos.y = world_pos.y - 1 --otherwise scaffoldplus places blocks at player height
				local key = minetest.pos_to_string(world_pos)

				-- Ensure we haven't already processed this block
				if not checked[key] then
					checked[key] = true

					-- Check distance from origin (XZ only)
					local dist = math.sqrt(offset.x ^ 2 + offset.z ^ 2)
					if dist <= max_distance then
						local node = core.get_node_or_nil(world_pos)
						if node and node.name == "air" then
							core.place_node(world_pos)
							nodes_placed = nodes_placed + 1
						end

						-- Add surrounding positions to the queue
						for dx = -1, 1 do
							for dz = -1, 1 do
								if dx ~= 0 or dz ~= 0 then
									local neighbor = {x = offset.x + dx, y = 0, z = offset.z + dz}
									local n_key = minetest.pos_to_string(neighbor)
									if not visited[n_key] then
										table.insert(queue, neighbor)
										visited[n_key] = true
									end
								end
							end
						end
					end
				end
			end
		end


		if core.settings:get_bool("block_water") then
			local positions = core.find_nodes_near(pos, 5, {"mcl_core:water_source", "mcl_core:water_floating", "default:water_source", "default:water_flowing"}, true)
			for i, p in pairs(positions) do
				if i > nodes_per_tick then return end
				core.place_node(p)
			end
		end
		if core.settings:get_bool("block_lava") then
			local positions = core.find_nodes_near(pos, 5, {"mcl_core:lava_source", "mcl_core:lava_floating", "default:lava_source", "default:lava_flowing"}, true)
			for i, p in pairs(positions) do
				if i > nodes_per_tick then return end
				core.place_node(p)
			end
		end
		if core.settings:get_bool("autotnt") then -- little modification here, autoTNT now will only work if the player is holding tnt
            core.switch_to_item("mcl_tnt:tnt") 
			local positions = core.find_nodes_near_under_air_except(pos, 2, item:get_name(), true)
			for i, p in pairs(positions) do
                core.switch_to_item("mcl_tnt:tnt") 
				if i > nodes_per_tick then return end
                    if core.switch_to_item("mcl_tnt:tnt") then
				        core.place_node(vector.add(p, {x = 0, y = 1, z = 0}))
                    else end
			end
		end
	else
		core.settings:set_bool("scaffold.active", false)
	end
	if core.settings:get_bool("nuke") then
		local i = 0
		for x = pos.x - 4, pos.x + 4 do
			for y = pos.y - 4, pos.y + 4 do
				for z = pos.z - 4, pos.z + 4 do
					local p = vector.new(x, y, z)
					local node = core.get_node_or_nil(p)
					local def = node and core.get_node_def(node.name)
					if def or def.diggable then
						if i > nodes_per_tick then return end
						core.dig_node(p)
						i = i + 1
					end
				end
			end
		end
	end
end)

core.register_cheat("Scaffold", "World", "scaffold")
core.register_cheat_setting("Jump Delay", "World", "scaffold", "scaffold.jump_delay", {type="slider_float", min=0.0, max=0.8, steps=9})
core.register_cheat("ScaffoldPlus", "World", "scaffold_plus")
core.register_cheat_setting("Jump Delay", "World", "scaffold_plus", "scaffold.jump_delay", {type="slider_float", min=0.0, max=0.8, steps=9})
core.register_cheat("BlockWater", "World", "block_water")
core.register_cheat("BlockLava", "World", "block_lava")
core.register_cheat("AutoTNT", "World", "autotnt")
core.register_cheat("Nuke", "World", "nuke")
core.register_cheat("Replace", "World", "replace")
core.register_cheat_setting("Nodes Per Tick", "World", "scaffold_plus", "nodes_per_tick", {type="slider_int", min=1, max=64, steps=64})