local message = ""
local exit_after = false
local agree_tos = false

local function get_message_formspec(this)
	return table.concat({
		"formspec_version[8]",
		"size[4,4]",
		"bgcolor[;neither;]",

		-- Header image
		"image[0,0;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_header.png") .. "]",
		-- message label
		"style_type[textarea;textcolor=white;font_size=*1.25;font=bold]" ..
		"textarea[0.2,1;3.6,2;;" .. message ..";]",

		-- Styled button
		"style_type[image_button;border=false;textcolor=white;font_size=*1;padding=0;font=bold;" ..
		"bgimg=" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. ";" ..
		"bgimg_hovered=" .. core.formspec_escape(defaulttexturedir .. "menu_button_hovered.png") .. "]",

		"image_button[0.5,3.1;3,0.6;;back;" .. fgettext("Continue") .. "]"
	}, "\n")
end

local function handle_message_buttons(this, fields)
	if fields.back then
		local current_dlg = ui.find_by_name("dlg_register_account")
		this:delete()
		if exit_after then
			current_dlg:delete()
		else
			current_dlg:show()
		end
		ui.update()
		return true
	end

	return false
end

local function handle_message_event(event)
	if event == "MenuQuit" then
		local current_dlg = ui.find_by_name("dlg_register_account")
		local message_dlg = ui.find_by_name("dlg_register_account_message")
		message_dlg:delete()
		if exit_after then
			current_dlg:delete()
		else
			current_dlg:show()
		end
		ui.update()
		return true
	end

	return false
end

local function create_message_dialog()
	local dlg = dialog_create("dlg_register_account_message", get_message_formspec, handle_message_buttons, handle_message_event)
	return dlg
end

local function show_message_dialog(msg, exit)
	if exit ~= nil then
		exit_after = exit
	end
	local current_dlg = ui.find_by_name("dlg_register_account")
	current_dlg:hide()
	local dlg = create_message_dialog()
	message = msg
	dlg:set_parent(current_dlg)
	current_dlg:hide()
	dlg:show()
	ui.update()
end

local function get_register_formspec(dialogdata)
	return table.concat({
		"formspec_version[8]",
		"size[5,8]",
		"bgcolor[;neither;]",

		-- Header image
		"image[0,0;5,1;" .. core.formspec_escape(defaulttexturedir .. "menu_header.png") .. "]",

		-- Field styling
		"style_type[*;border=false;textcolor=white;font_size=*1.5;padding=0;font=bold]",

		-- Username
		"label[0.5,1.3;" .. fgettext("Username:") .. "]",
		"image[0.5,1.55;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. "]",
		"field[0.5,1.55;4,0.8;username;;" .. core.formspec_escape(login_username) .. "]",

		-- Password
		"label[0.5,2.55;" .. fgettext("Password:") .. "]",
		"image[0.5,2.8;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. "]",
		"field[0.5,2.8;4,0.8;password;;" .. core.formspec_escape(dialogdata.password or "") .. "]",

		-- Confirm Password
		"label[0.5,3.8;" .. fgettext("Confirm Password:") .. "]",
		"image[0.5,4.05;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. "]",
		"field[0.5,4.05;4,0.8;confirm;;" .. core.formspec_escape(dialogdata.confirm or "") .. "]",

		-- Terms checkbox and view button
		"checkbox[0.75,5.2;agree_tos;" .. fgettext("I agree to the ToS and Privacy Policy") .. ";false]",
		"image_button[0.5,5.5;4,0.4;;view_terms;" .. fgettext("View ToS & Privacy Policy") .. "]",

		-- Styled buttons
		"style_type[image_button;border=false;textcolor=white;font_size=*2;padding=0;font=bold;" ..
		"bgimg=" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. ";" ..
		"bgimg_hovered=" .. core.formspec_escape(defaulttexturedir .. "menu_button_hovered.png") .. "]",

		"image_button[0.5,6.2;4,0.8;;register;" .. fgettext("Register") .. "]",
		"image_button[0.5,7.1;4,0.8;;back;" .. fgettext("Back") .. "]"
	}, "\n")
end


local function register_buttonhandler(this, fields)
	login_username = fields.username or ""
	local password = fields.password or ""
	local confirm = fields.confirm or ""
	if fields.agree_tos then
		agree_tos = fields.agree_tos == "true"
	end
	if fields.view_terms then
		show_message_dialog(fgettext(
			"Terms of Service and Privacy Policy\n\n" ..
			"By creating an account, you agree to the following:\n\n" ..
			"We only store your username and a securely hashed password for login purposes.\n\n" ..
			"We do not collect personal information or IP addresses.\n\n" ..
			"We may store cosmetic preferences (such as skins, colors, or avatars) tied to your account to personalize your experience.\n\n" ..
			"If in-game messaging or social features are introduced, message content may be stored temporarily to support communication.\n\n" ..
			"Abuse, impersonation, or malicious behavior may result in account termination.\n\n" ..
			"You may request account deletion at any time through our Discord. Upon deletion, all associated account data will be removed within 7 business days.\n\n" ..
			"TeamAcedia reserves the right to terminate accounts at any time, with or without notice.\n\n" ..
			"This service is provided as-is, without warranty or guarantee of uptime or availability."
		))

		return true
	end

	if fields.register then
		if login_username == "" or password == "" or confirm == "" then
			show_message_dialog(fgettext("All fields are required."))
			return true
		end

		if #login_username > 15  then
			show_message_dialog(fgettext("Username is too long.\n Maximum 15 characters."))
			return true
		end

		if password ~= confirm then
			show_message_dialog(fgettext("Passwords do not match."))
			return true
		end

		if not agree_tos then
			show_message_dialog(fgettext("You must agree to the ToS and Privacy Policy before registering."))
			return true
		end

		local http = core.get_http_api()
		if not http then
			show_message_dialog(fgettext("HTTP API not available."))
			return true
		end

		local hashed_pw = core.sha256(password)

		local handle = http.fetch_async({
			url = "http://teamacedia.nomorecheating.org:999/api/register",
			timeout = 5,
			post_data = core.write_json({
				username = login_username,
				password = hashed_pw
			}),
			extra_headers = {
				"Content-Type: application/json"
			}
		})

		local result = http.fetch_async_get(handle)
		while not result.completed do
			result = http.fetch_async_get(handle)
		end

		if result.succeeded and result.code == 201 then
			cache_settings:set(LOGIN_PASSWORD_SETTING_NAME, hashed_pw)
			cache_settings:set(LOGIN_USERNAME_SETTING_NAME, login_username)
			show_message_dialog(fgettext("Account created successfully."), true)
		else
			local resp = core.parse_json(result.data)
			show_message_dialog(fgettext("Registration failed: ") .. (resp and resp.message or "Network error: Failed to contact login server."))
		end

		return true

	elseif fields.back then
		this:delete()
		local mainmenu = ui.find_by_name("mainmenu")
		local dlg = create_sign_in_dialog()
		dlg:set_parent(mainmenu)
		mainmenu:hide()
		dlg:show()
		ui.update()
		return true
	end
end



local function register_eventhandler(event)
	if event == "DialogShow" then
		mm_game_theme.set_engine()
		mm_game_theme.clear_single("header")
		return true
	elseif event == "MenuQuit" then
		local mainmenu = ui.find_by_name("mainmenu")
		local current_dlg = ui.find_by_name("dlg_register_account")
		current_dlg:delete()
		local dlg = create_sign_in_dialog()
		dlg:set_parent(mainmenu)
		mainmenu:hide()
		dlg:show()
		ui.update()
		return true
	end
	return false
end

function create_register_account_dialog()
	local dlg = dialog_create("dlg_register_account", get_register_formspec, register_buttonhandler, register_eventhandler)
	return dlg
end
