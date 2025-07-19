local arts = {
	{
		"#####  #___#  _###_  #__#_  _____  #___#  _###_  #___#",
		"#____  #___#  #____  #_#_#  _____  _###_  #___#  #___#",
		"####_  #___#  #____  ###__  _____  __#__  #___#  #___#",
		"#____  #___#  #____  #_#_#  _____  __#__  #___#  #___#",
		"#____  _###_  _###_  #__#_  _____  __#__  _###_  _###_"
	}, {
		"#___#  #####  _###_  _###_  #####  ####_",
		"##__#  __#__  #____  #____  #____  #__#_",
		"#_#_#  __#__  #__#_  #__#_  ####_  ####_",
		"#__##  __#__  #__#_  #__#_  #____  #_#__",
		"#___#  #####  _###_  _###_  #####  #__#_"
	}, {
		"#____  #####  #####  _###_  _____  #___#  __#__  #___#  #####  _____  _###_  #####  #___#",
		"#____  #____  __#__  #____  _____  #___#  _###_  #___#  #____  _____  #____  #____  _###_",
		"#____  ####_  __#__  _###_  _____  #####  #####  #___#  ####_  _____  _###_  ####_  __#__",
		"#____  #____  __#__  ____#  _____  #___#  #___#  _###_  #____  _____  ____#  #____  _###_",
		"#####  #####  __#__  ###__  _____  #___#  #___#  __#__  #####  _____  ###__  #####  #___#"
	}, {
		"_###_  _###_  _###_  #__#_",
		"#____  #___#  #____  #_#_#",
		"#____  #___#  #____  ###__",
		"#____  #___#  #____  #_#_#",
		"_###_  _###_  _###_  #__#_"
	},
}

local current_art = 1
local current_line = 1
local message_time = 0

local function send_ascii_art()
	if not core.settings:get_bool("nsfw_spammer") then return end
	local art = arts[current_art]
	if current_line <= #art then
		core.send_chat_message(art[current_line])
		current_line = current_line + 1
	else
		core.settings:set_bool("nsfw_spammer", false)
		current_art = math.random(1, #arts)
		current_line = 1
		core.send_chat_message("sussy ^")
	end
end

core.register_globalstep(function(dtime)
	if core.localplayer then
		if core.settings:get_bool("nsfw_spammer") then
			message_time = message_time + dtime
		end
		if message_time > 0.1 then
			message_time = 0
			send_ascii_art()
		end
	end
end)

core.register_cheat_with_infotext("NSFW Spammer", "Misc", "nsfw_spammer", "Ascii")