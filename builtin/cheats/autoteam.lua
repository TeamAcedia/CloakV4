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
	message = string.gsub(string.gsub(string.gsub(message, "(T@ctf_teams)", ""), "F", ""), "E", "")
	
	if string.sub(message, 1, 19) == "(T@ctf_teams)Team " or string.sub(message, 1, 5) == "Team " then
		local enemies = {}
		local allies = {}
		local team_strings = string.split(message, "\n")
		for _, value in ipairs(team_strings) do
			local team = string.match(value, "Team %(c@#%x+%)%s*(%w+)")

			local playerNames = {}
			message = string.split(value, ":")[2]

			for playerName in string.gmatch(message, "[%a%d_-]+") do
				table.insert(playerNames, playerName)
			end
			
			local is_team = false
			for _, name in ipairs(playerNames) do
				if name == core.localplayer:get_name() then
					is_team = true
				end
			end

			if is_team == false then
				for _, name in ipairs(playerNames) do
					table.insert(enemies, name)
				end
			else
				for _, name in ipairs(playerNames) do
					table.insert(allies, name)
				end
			end
			
		end
		core.set_player_list("enemies", enemies)
		core.set_player_list("allies", allies)

		if awaiting_team then
			awaiting_team = false
			return true
		end
	end
end)

core.register_cheat_with_infotext("AutoTeam", "Misc", "autoteam", "CTF")