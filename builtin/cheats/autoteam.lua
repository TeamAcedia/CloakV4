local awaiting_team = false
local team_time = 5
local invalid_game = false

core.register_globalstep(function(dtime)
	if core.localplayer and not invalid_game then
		if core.get_server_game() == "not_initialized" then return end

		if core.get_server_game() ~= "capturetheflag" then
			core.update_infotext("Auto Team", "Misc", "autoteam", "Invalid Game")
			invalid_game = true
			return
		end
		if core.settings:get_bool("autoteam") then
			team_time = team_time + dtime
		end
		if team_time > 5 then
			team_time = 0
			core.send_chat_message("/team")
			awaiting_team = true
		end
	end
end)

core.register_on_receiving_chat_message(function(message)
    -- General cleanup of known universal control characters
    local cleaned_message = string.gsub(message, "(T@ctf_teams)", "")
    cleaned_message = string.gsub(cleaned_message, "F", "")
    cleaned_message = string.gsub(cleaned_message, "E", "")

    local enemies = {}
    local allies = {}
    local parsed_successfully = false

    -- Split the message into individual team lines
    local team_lines = string.split(cleaned_message, "\n")

    for _, line in ipairs(team_lines) do
        local team_name = nil
        local player_names_string = nil

        -- --- Attempt to parse NEW format ---
        -- The new format contains " has " and color codes
        local has_players_idx = string.find(line, " has ")
        if has_players_idx then
            -- This is likely the NEW format

            -- Find the colon separating team info from player list
            local colon_idx = string.find(line, ":")
            if colon_idx then
                local team_info_part = string.sub(line, 1, colon_idx - 1)
                player_names_string = string.sub(line, colon_idx + 1)

                -- Extract team name from team_info_part
                -- Example: "Team (c@#01b900)green(c@#ffffff) has 16 players"
                -- Find the last closing parenthesis and take the word after it
                local last_paren_close_idx = string.find(team_info_part, "%)", 1, true) -- true for plain match
                if last_paren_close_idx then
                    -- Extract the part after the last color code's closing parenthesis
                    local potential_team_name_part = string.sub(team_info_part, last_paren_close_idx + 1)
                    -- Now find the word before " has "
                    -- Example: "green has 16 players"
                    local team_name_end_idx = string.find(potential_team_name_part, " has ")
                    if team_name_end_idx then
                        team_name = string.sub(potential_team_name_part, 1, team_name_end_idx - 1)
                        team_name = string.gsub(team_name, "%s*$", "") -- Trim trailing spaces
                    end
                else
                    -- Fallback for new format if no color codes but still " has " (unlikely given example)
                    -- Find "Team " and " has "
                    local start_team_name = string.find(team_info_part, "Team ")
                    local end_team_name = string.find(team_info_part, " has ", start_team_name + 5)
                    if start_team_name and end_team_name then
                        team_name = string.sub(team_info_part, start_team_name + 5, end_team_name - 1)
                        team_name = string.gsub(team_name, "%s*$", "") -- Trim trailing spaces
                    end
                end
            end
        end

        -- --- If not new format, attempt to parse OLD format ---
        -- The old format did NOT have " has " but still used a colon for players
        if not team_name and string.sub(line, 1, 5) == "Team " then
            local colon_idx = string.find(line, ":")
            if colon_idx then
                -- This is likely the OLD format
                local team_info_part = string.sub(line, 1, colon_idx - 1)
                player_names_string = string.sub(line, colon_idx + 1)

                -- For the old format, the team name might have been directly after "Team "
                -- Example: "Team green: Player1, Player2"
                local team_start_idx = string.find(team_info_part, "Team ")
                if team_start_idx then
                    team_name = string.sub(team_info_part, team_start_idx + 5)
                    team_name = string.gsub(team_name, "%s*$", "") -- Trim trailing spaces
                end
            end
        end

        -- If we successfully extracted a team name and player string
        if team_name and player_names_string then
            parsed_successfully = true
            local current_line_players = {}
            -- Extract player names using gmatch for word characters (robust)
            for playerName in string.gmatch(player_names_string, "[%a%d_-]+") do
                table.insert(current_line_players, playerName)
            end

            local is_team_member = false
            local current_player_name = core.localplayer:get_name()

            for _, name in ipairs(current_line_players) do
                if name == current_player_name then
                    is_team_member = true
                    break
                end
            end

            if is_team_member then
                for _, name in ipairs(current_line_players) do
                    table.insert(allies, name)
                end
            else
                for _, name in ipairs(current_line_players) do
                    table.insert(enemies, name)
                end
            end
        end
    end

    if parsed_successfully then
        core.set_player_list("enemies", enemies)
        core.set_player_list("allies", allies)

        if awaiting_team then
            awaiting_team = false
            return true
        end
    end
end)

core.register_cheat_with_infotext("AutoTeam", "Misc", "autoteam", "CTF")