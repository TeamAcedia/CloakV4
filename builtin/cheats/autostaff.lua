local last_players = {}
local player_queue = {}
local isAdmin = {}
local qtime = 0

local function show_staff_warning(player_name)
    local formspec =
        "formspec_version[6]" ..
        "size[7,2]" ..
        "button[0.2,1.1;2,0.7;disconnect;Disconnect]" ..
        "button[2.5,1.1;2,0.7;panic;Panic]" ..
        "button[4.8,1.1;2,0.7;continue;Continue]" ..
        "label[0.3,0.4;" .. core.formspec_escape(player_name) .. " is a Staff member]"

    core.show_formspec("staff_warning", formspec)
end


core.register_on_receiving_chat_message(function(message)
    if not core.settings:get_bool("autostaff.warn_staff") then return end
	local p = core.localplayer
	if not p then return end
    if p:get_name() == "singleplayer" then return end
    
    local cleaned_message = message
    :gsub("\27%(T@__builtin%)", "")
    :gsub("\27.", "")         -- remove simple escape+char codes
    :gsub("\27%b()", "")      -- remove escape + (...) codes


    if string.find(cleaned_message, "Privileges") then
        local player_name = string.match(cleaned_message, "of%s+(%S+)")
        if player_name then
            player_name = player_name:gsub(":$", "")
        end
        local privileges_part = string.match(cleaned_message, ":%s*(.*)")

        if privileges_part and player_name then
            local privileges = {}
            local isPlayerAdmin = false

            for privilege in string.gmatch(privileges_part, "[^,]+") do
                local trimmed_privilege = privilege:match("^%s*(.-)%s*$")
                table.insert(privileges, trimmed_privilege)

                if trimmed_privilege == "ban" or trimmed_privilege == "kick" then
                    isPlayerAdmin = true
                    if core.settings:get_bool("autostaff.warn_staff") then
                        show_staff_warning(player_name)
                    end
                    core.add_to_player_list("staff", player_name)
                    return
                end
            end

            isAdmin[player_name] = isPlayerAdmin
        end
    end
end)

core.register_globalstep(function(dtime)

    qtime = qtime + dtime
    if qtime > 1 and core.settings:get_bool("autostaff") then
        qtime = 0
	 local po = core.localplayer
    	 if not po then return end
         if po:get_name() == "singleplayer" then return end
        -- Refresh player list only when queue is empty
        if #player_queue == 0 then
            local current_players = core.get_player_names()
            if not current_players or #current_players == 0 then return end

            -- Add only new players to the queue
            for _, player in ipairs(current_players) do
                if not last_players[player] then
                    table.insert(player_queue, player)
                end
            end

            last_players = {}
            for _, player in ipairs(current_players) do
                last_players[player] = true
            end
        end

        -- Process one player per loop
        local player = table.remove(player_queue, 1)
        if player then
            core.run_server_chatcommand("privs", player)
        end
    end
end)


-- Handle button actions
core.register_on_formspec_input(function(formname, fields)
    if formname == "staff_warning" then
        if fields.disconnect then
            core.disconnect()
        elseif fields.panic then
            core.settings:set_bool("panic", true)
            core.show_formspec("", "") -- Who the hell thought it was a good idea to remove close_formspec in favor of this dumb workaround -- we could literally just make a wrapper function for this lmaoooo but who tf cares, prouncedev
        elseif fields.continue then
            core.show_formspec("", "") -- Who the hell thought it was a good idea to remove close_formspec in favor of this dumb workaround
        end
    end
end)






