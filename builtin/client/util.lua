function core.parse_pos(param)
	local p = {}
	local playerpos = core.localplayer:get_pos()
	p.x, p.y, p.z = string.match(param, "^([~|%d.-]+)[, ] *([~|%d.-]+)[, ] *([~|%d.-]+)$")
	for k, v in pairs(p) do
		if p[k] == "~" then
			p[k] = playerpos[k]
		else
			p[k] = tonumber(v)
		end
	end
	if p.x and p.y and p.z then
		return true, vector.round(p)
	end
	return false, "Invalid position (" .. param .. ")"
end

function core.parse_relative_pos(param)
	local success, pos = core.parse_pos(param:gsub("~", "0"))
	if success then pos = vector.round(vector.add(core.localplayer:get_pos(), pos)) end
	return success, pos
end

function core.parse_num(n)
	local num = tonumber(n)
	if num then
		return true, num
	end
	return false, "Invalid number (" .. n .. ")"
end

function core.find_item(item, mini, maxi)
	for index, stack in ipairs(core.get_inventory("current_player").main) do
		if (not mini or index >= mini) and (not maxi or index <= maxi) and stack:get_name() == item then
			return index
		end
	end
end

function core.find_additional_items(item, exclude_index)
    for index, stack in ipairs(core.get_inventory("current_player").main) do
        if index ~= exclude_index and stack:get_name() == item then
            return index
        end
    end
    return nil
end

function core.get_total_items(item)
    local total_count = 0
    for _, stack in ipairs(core.get_inventory("current_player").main) do
        if stack:get_name() == item then
            total_count = total_count + stack:get_count()
        end
    end
    return total_count
end

function core.switch_to_item(item)
	local i = core.find_item(item)
	if i then
		core.localplayer:set_wield_index(i)
		return true
	else
		return false
	end
end

function core.get_nearby_objects(radius)

	return core.get_objects_inside_radius(core.localplayer:get_pos(), radius)
end

function core.set_player_list(setting, names)
	local server_url = core.get_server_url()
	if not server_url then
		return
	end
	local data = core.settings:get_json(setting) or {}
	data[server_url] = table.concat(names, ",")
	core.settings:set_json(setting, data)
end

function core.add_to_player_list(setting, name)
	local server_url = core.get_server_url()
	if not server_url then
		return
	end
	local data = core.settings:get_json(setting) or {}
	local list = (data[server_url] or ""):split(",")
	if table.indexof(list, name) ~= -1 then
		return false, name .. " is already on the list."
	end
	table.insert(list, name)
	core.set_player_list(setting, list)
	return true, "Added " .. name .. " to the list."
end

function core.remove_from_player_list(setting, server_url, name)
	local server_url = core.get_server_url()
	if not server_url then
		return
	end
	local data = core.settings:get_json(setting) or {}
	local list = (data[server_url] or ""):split(",")
	local index = table.indexof(list, name)
	if index == -1 then
		return false, name .. " is not on the list."
	end
	table.remove(list, index)
	core.set_player_list(setting, server_url, list)
	return true, "Removed " .. name .. " from the list."
end

local game
local awaiting_status = false

function core.get_server_game()
	if not game then
		return "not_initialized"
	else
		return game
	end
end

core.register_on_receiving_chat_message(function(message)
	message = string.gsub(string.gsub(string.gsub(message, "(T@ctf_teams)", ""), "F", ""), "E", "")

	local game_match = string.match(message, "game:%s*([^|]+)")
	if game_match then
		game = game_match:gsub("%s+", ""):lower()
		if awaiting_status then			
						awaiting_status = false						
						return true						
					else						
						return false						
					end						
				end
																
				if string.sub(message, 1, 24) == "— JMA Capture the Flag" then						
					game = "capturetheflag"					
					if awaiting_status then						
						awaiting_status = false						
						return true						
					else						
						return false
					end
	end
end)

core.register_globalstep(function(dtime)
	if not game and not awaiting_status then
		core.send_chat_message("/status")
		awaiting_status = true
	end
end)