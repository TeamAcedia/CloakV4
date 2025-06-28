Strata = {
	formatted_string = core.colorize("white", "[") .. core.colorize("purple", "Strata") .. core.colorize("white", "] -> "),

	task_tracers = {},
	task_nodes = {},

	next_actions = {},
	current_action = nil,
	current_target = {}, 
	pathfinding_complete = false,
	visual_started = false,
	current_pos = nil,
	action_timer = 0
}

Strata.clear_path_visuals = function()
	for _, pos in ipairs(Strata.task_nodes) do
		core.clear_task_node(pos)
	end
	Strata.task_nodes = {}

	for _, tracer in ipairs(Strata.task_tracers) do
		core.clear_task_tracer(tracer[1], tracer[2])
	end
	Strata.task_tracers = {}
end

Strata.visualize_actions = function(start_pos, end_pos, actions, reached_goal)
	if not Strata.visual_started then
		Strata.clear_path_visuals()
		Strata.visual_started = true
	end

	local function add_task_node(pos, color)
		core.add_task_node(pos, color)
		table.insert(Strata.task_nodes, pos)
	end

	-- Start and End Nodes (only draw end once)
	add_task_node(start_pos, {r=0, g=0, b=255})
	if reached_goal then
		add_task_node(end_pos, {r=0, g=255, b=0})
	else
		add_task_node(actions[#actions].to, {r=255, g=0, b=255})
		add_task_node(end_pos, {r=0, g=255, b=0})
	end

	for _, step in ipairs(actions) do
		local curr = step.from
		local next = step.to
		local tracer_color = {r=200, g=200, b=200} -- Default

		for _, action in ipairs(step.actions) do
			if action.action == "mine_block" then
				add_task_node(action.pos, {r=255, g=0, b=0})
				tracer_color = {r=255, g=0, b=0}
			elseif action.action == "place_block" then
				add_task_node(action.pos, {r=0, g=255, b=0})
				tracer_color = {r=0, g=0, b=255}
			elseif action.action == "jump" then
				tracer_color = {r=255, g=0, b=0}
			elseif action.action == "fall" then
				tracer_color = {r=255, g=0, b=0}
			elseif action.action == "walk" then
				tracer_color = {r=255, g=0, b=0}
			end
		end

		core.add_task_tracer(curr, next, tracer_color)
		table.insert(Strata.task_tracers, {curr, next})
	end
end


Strata.compute_actions_required_to_complete_path = function(path)
	local actions = {}

	local function is_walkable(pos)
		local node = core.get_node_or_nil(pos)
		return node and node.name == "air"
	end

	local function is_solid(pos)
		local node = core.get_node_or_nil(pos)
		return node == nil or node.name ~= "air"
	end

	-- Store all mining positions from previous steps
	local mined_positions = {}

	-- Helper to check if a position is in mined_positions
	local function will_be_mined(pos)
		for _, mined_pos in ipairs(mined_positions) do
			if vector.equals(mined_pos, pos) then
				return true
			end
		end
		return false
	end

	for i = 1, #path - 1 do
		local curr = path[i]
		local next = path[i + 1]

		local dx = next.x - curr.x
		local dy = next.y - curr.y
		local dz = next.z - curr.z

		local pre_actions = {}  -- mine/place
		local move_action = nil -- walk/jump/fall

		local curr_feet = curr
		local curr_head = vector.add(curr, {x = 0, y = 1, z = 0})
		local next_feet = next
		local next_head = vector.add(next, {x = 0, y = 1, z = 0})
		local block_below_next = vector.add(next, {x = 0, y = -1, z = 0})

		-- Ensure walkable space is clear
		if is_solid(next_feet) then
			table.insert(pre_actions, {action = "mine_block", pos = next_feet})
		end
		if is_solid(next_head) then
			table.insert(pre_actions, {action = "mine_block", pos = next_head})
		end

		-- Jumping up
		if dy > 0 then
			local block_below_curr = vector.add(curr, {x = 0, y = -1, z = 0})
			local head_above_curr = vector.add(curr, {x = 0, y = 2, z = 0})

			if not is_solid(block_below_curr) then
				table.insert(pre_actions, {action = "place_block", pos = block_below_curr})
			end

			-- Only add place_block before jump if the block_below_next is mined previously anywhere
			if will_be_mined(block_below_next) then
				move_action = {
					action = "jump",
					pre_jump_place = {action = "place_block", pos = block_below_next}
				}
			else
				move_action = {action = "jump"}
			end

			if is_solid(head_above_curr) then
				table.insert(pre_actions, {action = "mine_block", pos = head_above_curr})
			end

		-- Falling
		elseif dy < 0 then
			local above_next = vector.add(next, {x = 0, y = 2, z = 0})
			if is_solid(above_next) then
				table.insert(pre_actions, {action = "mine_block", pos = above_next})
			end
			move_action = {action = "fall"}

		-- Walking on flat
		else
			move_action = {action = "walk"}
			if not is_solid(block_below_next) then
				table.insert(pre_actions, {action = "place_block", pos = block_below_next})
			end
		end

		-- Track mining blocks for future steps
		for _, act in ipairs(pre_actions) do
			if act.action == "mine_block" then
				table.insert(mined_positions, act.pos)
			end
		end

		-- Final ordered action list
		local action_list = {}

		-- Add all pre_actions first (mining/placing)
		for _, act in ipairs(pre_actions) do
			table.insert(action_list, act)
		end

		-- Add the move action
		if move_action.pre_jump_place then
			-- Insert place block first, then jump
			table.insert(action_list, move_action.pre_jump_place)
			table.insert(action_list, {action = "jump"})
		else
			table.insert(action_list, move_action)
		end

		table.insert(actions, {
			from = curr,
			to = next,
			actions = action_list
		})
	end

	return actions
end




Strata.clear_actions = function()
	Strata.current_action = nil
	Strata.current_step = nil
	Strata.next_actions = {}
	Strata.current_target = {}
	Strata.current_pos = nil
	Strata.pathfinding_complete = false
	Strata.visual_started = false
end


Strata.set_target = function(pos)
	Strata.clear_actions()
	Strata.current_target = pos
end

Strata.get_next_action = function()
	-- If there's a current step with remaining actions, continue it
	if Strata.current_step and Strata.current_step.actions and #Strata.current_step.actions > 0 then
		local next_action = table.remove(Strata.current_step.actions, 1)
		Strata.current_action = {
			type = next_action.action,
			action_pos = next_action.pos,
			from = Strata.current_step.from,
			to = Strata.current_step.to,
		}
		Strata.action_timer = 0
		return
	end

	-- Move to the next step if the current one is done
	local next_step = table.remove(Strata.next_actions, 1)
	if next_step then
		Strata.current_step = next_step
		local next_action = table.remove(next_step.actions, 1)
		if next_action then
			Strata.current_action = {
				type = next_action.action,
				action_pos = next_action.pos,
				from = next_step.from,
				to = next_step.to,
			}
			Strata.action_timer = 0
		else
			Strata.get_next_action() -- recursive call if somehow no actions
		end
	else
		-- No more actions at all
		Strata.current_action = nil
		Strata.current_step = nil
	end
end




core.register_chatcommand("goto", {
	params = "<x> <y> <z>",
	description = "Find path from player to target position and visualize it",
	func = function(param)
		local success, target_pos = core.parse_pos(param)
		if not success then
			print(Strata.formatted_string .. "Invalid coordinates.")
			return true
		end

		local player = core.localplayer
		if not player then
			print(Strata.formatted_string .. "Localplayer not initialized.")
			return true
		end

		local end_pos = vector.round(target_pos)

		Strata.set_target(end_pos)

		print(Strata.formatted_string .. "Now pathfinding to " .. end_pos.x .. ", " .. end_pos.y .. ", " .. end_pos.z)
		return true
	end
})

local update_interval = 1
local timer = 0

Strata.set_controls = function(new_controls)
	if not Strata.last_controls then
		Strata.last_controls = {}
	end

	local updated = false
	local combined_controls = {}

	for _, key in ipairs({ "up", "down", "left", "right", "jump", "sneak", "aux1", "dig", "place" }) do
		local new_val = new_controls[key]
		local old_val = Strata.last_controls[key] or false

		if new_val == nil then
			combined_controls[key] = old_val
		else
			combined_controls[key] = new_val
			if new_val ~= old_val then
				updated = true
			end
		end
	end

	if updated then
		core.localplayer:set_lua_control(combined_controls)
		Strata.last_controls = table.copy(combined_controls)
		core.settings:set_bool("lua_control", true)
		core.settings:set_bool("detached_camera", true)
	end
end

Strata.clear_controls = function()
	Strata.last_controls = {}
	core.settings:set_bool("lua_control", false)
	core.settings:set_bool("detached_camera", false)
end

Strata.position_equals = function(pos1, pos2)
	local dx = math.abs(pos1.x - pos2.x)
	local dy = math.abs(pos1.y - pos2.y)
	local dz = math.abs(pos1.z - pos2.z)
	return dx <= 0.7 and dy <= 0.7 and dz <= 0.7
end

Strata.dig_node = function(pos)
    local nd=minetest.get_node_or_nil(pos)
    local nodename='air'
    if nd then nodename=nd.name end
    local digtime=ws.get_digtime(nodename)
	ws.aim(pos)
	ws.select_best_tool(pos)
	local pointed_thing = {
		type = "node",
		under = pos,
		above = pos
	}
	
	core.interact("start_digging", pointed_thing)
	core.after(digtime, function()
		core.interact("digging_completed", pointed_thing)
	end)
end

Strata.switch_to_blocks = function()
	if not core.localplayer then return false end

	local inventory = core.get_inventory("current_player")
	if not inventory or not inventory.main then return false end

	for index, stack in ipairs(inventory.main) do
		if stack and not stack:is_empty() then
			local itemname = stack:get_name()
			local itemdef = core.get_item_def(itemname)

			if itemdef and itemdef.type == "node" then
				-- Switch to the block
				core.localplayer:set_wield_index(index)

				-- Always attempt to refill
				local space = stack:get_free_space()
				local extra_index = core.find_additional_items(itemname, index + 1)
				if extra_index and space > 0 then
					local move_act = InventoryAction("move")
					move_act:to("current_player", "main", index)
					move_act:from("current_player", "main", extra_index)
					move_act:set_count(space)
					move_act:apply()
				end

				return true -- success
			end
		end
	end

	return false -- no placeable blocks found
end

Strata.place_node = function(pos)
	Strata.switch_to_blocks()
	ws.aim(pos)
	local pointed_thing = {
		type = "node",
		under = pos,
		above = pos
	}
	core.interact("place", pointed_thing)
end

core.register_globalstep(function(dtime)
	timer = timer + dtime
	if timer > update_interval then
		timer = 0

		if Strata.current_target and next(Strata.current_target) ~= nil and not Strata.pathfinding_complete and #Strata.next_actions < 25 then
			local player = core.localplayer
			if not player then return end

			local start_pos = Strata.current_pos or vector.round(player:get_pos())
			local end_pos = vector.round(Strata.current_target)

			local path = core.find_path(start_pos, end_pos)

			if not path or #path == 0 then
				print(Strata.formatted_string .. "Path blocked. Could not proceed from " .. core.pos_to_string(start_pos))
				Strata.pathfinding_complete = true
				return
			end

			if #path == 1 then
				-- no further progress on the path can be made yet ( mapblocks not loaded usually )
				return
			end

			local reached_goal = vector.equals(path[#path], end_pos)
			local new_actions = Strata.compute_actions_required_to_complete_path(path)

			-- Append to cumulative action list
			for _, step in ipairs(new_actions) do
				table.insert(Strata.next_actions, step)
			end

			-- Visualize current segment
			Strata.visualize_actions(start_pos, end_pos, new_actions, reached_goal)

			-- Update current_pos and completion status
			Strata.current_pos = path[#path]
			Strata.pathfinding_complete = reached_goal

			if reached_goal then
				print(Strata.formatted_string .. "Pathfinding completed. Total actions: " .. #Strata.next_actions)
			else
				print(Strata.formatted_string .. "Partial path found. Continuing from " .. core.pos_to_string(Strata.current_pos))
			end
		end
	end

	-- action handler

	local action_schema

	if Strata.target_pos ~= nil and Strata.position_equals(core.localplayer:get_pos(), Strata.target_pos) then
		print(Strata.formatted_string .. "Target reached.")
	end

	-- Timeout check before processing current_action
	if Strata.current_action and Strata.current_action.type ~= "mine_block" then
		Strata.action_timer = Strata.action_timer + dtime
		if Strata.action_timer > 2 then
			print(Strata.formatted_string .. "Action timeout. Recalculating path from " .. core.pos_to_string(vector.round(core.localplayer:get_pos())))
			Strata.pathfinding_complete = false
			Strata.current_pos = vector.round(core.localplayer:get_pos())
			Strata.next_actions = {}
			Strata.current_action = nil
			Strata.visual_started = false
			return
		end
	end


	if Strata.current_action == nil then
		Strata.get_next_action()
		Strata.clear_controls()
		return
	else
		local current_action = Strata.current_action
		local player_pos = core.localplayer:get_pos()
		if current_action.type == "walk" then
			if Strata.position_equals(player_pos, current_action.to) then
				core.clear_task_tracer(current_action.from, current_action.to)
				Strata.get_next_action()
				return
			else
				ws.aim(current_action.to)
				Strata.set_controls({up = true, jump = false, dig = false, place = false})
			end
		elseif current_action.type == "jump" then
			if Strata.position_equals(player_pos, current_action.to) then
				core.clear_task_tracer(current_action.from, current_action.to)
				Strata.get_next_action()
				return
			else
				ws.aim(current_action.to)
				Strata.set_controls({up = true, jump = true, dig = false, place = false})
			end
		elseif current_action.type == "fall" then
			if Strata.position_equals(player_pos, current_action.to) then
				core.clear_task_tracer(current_action.from, current_action.to)
				Strata.get_next_action()
				return
			else
				ws.aim(current_action.to)
				Strata.set_controls({up = true, jump = false, dig = false, place = false})
			end
		elseif current_action.type == "mine_block" then
			if ws.can_place_at(current_action.action_pos) then
				core.clear_task_node(current_action.action_pos)
				Strata.get_next_action()
				return
			else
				ws.aim(current_action.action_pos)
				Strata.set_controls({up = false, jump = false, dig = true, place = false})
				Strata.dig_node(current_action.action_pos)
			end
		elseif current_action.type == "place_block" then
			if not ws.can_place_at(current_action.action_pos) then
				core.clear_task_node(current_action.action_pos)
				Strata.get_next_action()
				return
			else
				ws.aim(current_action.action_pos)
				Strata.set_controls({up = false, jump = false, dig = false, place = true})
				Strata.place_node(current_action.action_pos)
			end
		else
			-- Unknown action
			print(Strata.formatted_string .. "Unknown action type encountered -> " .. dump(current_action.type))
		end
	end
	
	-- TODO: handle actions here (execute next_actions)
end)
