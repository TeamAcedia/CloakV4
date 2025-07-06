local message = ""
local exit_after = false
login_username = ""

local function get_message_formspec(this)
	return table.concat({
		"formspec_version[8]",
		"size[4,3]",
		"bgcolor[;neither;]",

		-- Header image
		"image[0,0;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_header.png") .. "]",
		-- message label
		"style_type[textarea;textcolor=white;font_size=*1.25;font=bold]" ..
		"textarea[0.2,1;3.6,1;;" .. message ..";]",

		-- Styled button
		"style_type[image_button;border=false;textcolor=white;font_size=*2;padding=0;font=bold;" ..
		"bgimg=" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. ";" ..
		"bgimg_hovered=" .. core.formspec_escape(defaulttexturedir .. "menu_button_hovered.png") .. "]",

		"image_button[0.5,2.1;3,0.6;;back;" .. fgettext("Continue") .. "]"
	}, "\n")
end

local function handle_message_buttons(this, fields)
	if fields.back then
		local current_dlg = ui.find_by_name("dlg_login")
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
		local current_dlg = ui.find_by_name("dlg_login")
		local message_dlg = ui.find_by_name("dlg_login_message")
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
	local dlg = dialog_create("dlg_login_message", get_message_formspec, handle_message_buttons, handle_message_event)
	return dlg
end

local function show_message_dialog(msg, exit)
	if exit ~= nil then
		exit_after = exit
	end
	local current_dlg = ui.find_by_name("dlg_login")
	current_dlg:hide()
	local dlg = create_message_dialog()
	message = msg
	dlg:set_parent(current_dlg)
	current_dlg:hide()
	dlg:show()
	ui.update()
end

local function get_formspec(dialogdata)
	return table.concat({
		"formspec_version[8]",
		"size[5,6]",
		"bgcolor[;neither;]",

		-- Header image
		"image[0,0;5,1;" .. core.formspec_escape(defaulttexturedir .. "menu_header.png") .. "]",

		-- Field styling to match buttons
		"style_type[*;border=false;textcolor=white;font_size=*1.5;padding=0;font=bold]",

		-- Username label and field
		"label[0.5,1.5;" .. fgettext("Username:") .. "]",
		"image[0.5,1.8;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. "]",
		"field[0.5,1.8;4,0.8;username;;" .. core.formspec_escape(login_username) .. "]",

		-- Password label and field
		"label[0.5,2.9;" .. fgettext("Password:") .. "]",
		"image[0.5,3.1;4,0.8;" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. "]",
		"field[0.5,3.1;4,0.8;password;;" .. core.formspec_escape(dialogdata.password or "") .. "]",

		-- Styled buttons
		"style_type[image_button;border=false;textcolor=white;font_size=*2;padding=0;font=bold;" ..
		"bgimg=" .. core.formspec_escape(defaulttexturedir .. "menu_button.png") .. ";" ..
		"bgimg_hovered=" .. core.formspec_escape(defaulttexturedir .. "menu_button_hovered.png") .. "]",

		"image_button[0.5,4.2;4,0.8;;login;" .. fgettext("Log In") .. "]",
		"image_button[0.5,5.1;4,0.8;;back;" .. fgettext("Back") .. "]"
	}, "\n")
end

local function buttonhandler(this, fields)
	login_username = fields.username or ""
	local password = fields.password or ""

	if fields.login then
		if login_username == "" or password == "" then
			show_message_dialog(fgettext("You must enter a username and password."))
			return true
		end

		local http = core.get_http_api()
		if not http then
			show_message_dialog(fgettext("HTTP API not available."))
			return true
		end

		local hashed_pw = core.sha256(password)

		-- Start async block
		local handle = http.fetch_async({
			url = "http://teamacedia.nomorecheating.org:999/api/login",  -- replace with your endpoint
			timeout = 5,
			post_data = core.write_json({
				username = login_username,
				password = hashed_pw
			}),
			extra_headers = {
				"Content-Type: application/json"
			}
		})
		-- Poll until request completes
		local result = http.fetch_async_get(handle)
		while not result.completed do
			result = http.fetch_async_get(handle)
		end

		-- Process result

		if result.succeeded and result.code == 200 then
			cache_settings:set(LOGIN_PASSWORD_SETTING_NAME, hashed_pw)
			cache_settings:set(LOGIN_USERNAME_SETTING_NAME, login_username)
			show_message_dialog(fgettext("Successfully logged in."), true)
		else
			local resp = core.parse_json(result.data)
			show_message_dialog(fgettext("Login failed: ") .. (resp and resp.message or "Network error: Failed to contact login server."))
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

local function eventhandler(event)
	if event == "DialogShow" then
		mm_game_theme.set_engine()
		mm_game_theme.clear_single("header")
		return true
	elseif event == "MenuQuit" then
		local mainmenu = ui.find_by_name("mainmenu")
		local current_dlg = ui.find_by_name("dlg_login")
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

function create_login_dialog()
	local dlg = dialog_create("dlg_login", get_formspec, buttonhandler, eventhandler)
	return dlg
end
