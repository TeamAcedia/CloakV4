-- For server-side translations (if INIT == "game")
-- Otherwise, use core.gettext
local S = core.get_translator("__builtin")

core.registered_chatcommands = {}

-- Interpret the parameters of a command, separating options and arguments.
-- Input: command, param
--   command: name of command
--   param: parameters of command
-- Returns: opts, args
--   opts is a string of option letters, or false on error
--   args is an array with the non-option arguments in order, or an error message
-- Example: for this command line:
--      /command a b -cd e f -g
-- the function would receive:
--      a b -cd e f -g
-- and it would return:
--	"cdg", {"a", "b", "e", "f"}
-- Negative numbers are taken as arguments. Long options (--option) are
-- currently rejected as reserved.
local function getopts(command, param)
	local opts = ""
	local args = {}
	for match in param:gmatch("%S+") do
		if match:byte(1) == 45 then -- 45 = '-'
			local second = match:byte(2)
			if second == 45 then
				return false, S("Invalid parameters (see /help @1).", command)
			elseif second and (second < 48 or second > 57) then -- 48 = '0', 57 = '9'
				opts = opts .. match:sub(2)
			else
				-- numeric, add it to args
				args[#args + 1] = match
			end
		else
			args[#args + 1] = match
		end
	end
	return opts, args
end

function core.register_chatcommand(cmd, def)
	def = def or {}
	def.params = def.params or ""
	def.description = def.description or ""
	def.privs = def.privs or {}
	def.mod_origin = core.get_current_modname() or "??"
	core.registered_chatcommands[cmd] = def
end

function core.unregister_chatcommand(name)
	if core.registered_chatcommands[name] then
		core.registered_chatcommands[name] = nil
	else
		core.log("warning", "Not unregistering chatcommand " ..name..
			" because it doesn't exist.")
	end
end

local function get_pt_name(object_type)
	local player = core.localplayer
	if not player then return end
	local pt = player:get_pointed_thing()
	if not pt then return end

	local item
	if object_type == "node" then
		item = core.get_node_or_nil(pt.under)
	elseif object_type == "player" then
		item = core.get_object_or_nil(pt.id)
	end
	if not item then return end
	return item.name
end

function core.override_chatcommand(name, redefinition)
	local chatcommand = core.registered_chatcommands[name]
	assert(chatcommand, "Attempt to override non-existent chatcommand "..name)
	for k, v in pairs(redefinition) do
		rawset(chatcommand, k, v)
	end
	core.registered_chatcommands[name] = chatcommand
end

local function format_help_line(cmd, def)
	local cmd_marker = INIT == "client" and "." or "/"
	local msg = core.colorize("#00ffff", cmd_marker .. cmd)
	if def.params and def.params ~= "" then
		msg = msg .. " " .. def.params
	end
	if def.description and def.description ~= "" then
		msg = msg .. ": " .. def.description
	end
	return msg
end

local function do_help_cmd(name, param)
	local opts, args = getopts("help", param)
	if not opts then
		return false, args
	end
	if #args > 1 then
		return false, S("Too many arguments, try using just /help <command>")
	end
	local use_gui = INIT == "client" or core.get_player_by_name(name)
	use_gui = use_gui and not opts:find("t")

	if #args == 0 and not use_gui then
		local cmds = {}
		for cmd, def in pairs(core.registered_chatcommands) do
			if INIT == "client" or core.check_player_privs(name, def.privs) then
				cmds[#cmds + 1] = cmd
			end
		end
		table.sort(cmds)
		local msg
		if INIT == "game" then
			msg = S("Available commands: @1",
				table.concat(cmds, " ")) .. "\n"
				.. S("Use '/help <cmd>' to get more "
				.. "information, or '/help all' to list "
				.. "everything.")
		else
			msg = core.gettext("Available commands: ")
				.. table.concat(cmds, " ") .. "\n"
				.. core.gettext("Use '.help <cmd>' to get more "
				.. "information, or '.help all' to list "
				.. "everything.")
		end
		return true, msg
	elseif #args == 0 or (args[1] == "all" and use_gui) then
		core.show_general_help_formspec(name)
		return true
	elseif args[1] == "all" then
		local cmds = {}
		for cmd, def in pairs(core.registered_chatcommands) do
			if INIT == "client" or core.check_player_privs(name, def.privs) then
				cmds[#cmds + 1] = format_help_line(cmd, def)
			end
		end
		table.sort(cmds)
		local msg
		if INIT == "game" then
			msg = S("Available commands:")
		else
			msg = core.gettext("Available commands:")
		end
		return true, msg.."\n"..table.concat(cmds, "\n")
	elseif INIT == "game" and args[1] == "privs" then
		if use_gui then
			core.show_privs_help_formspec(name)
			return true
		end
		local privs = {}
		for priv, def in pairs(core.registered_privileges) do
			privs[#privs + 1] = priv .. ": " .. def.description
		end
		table.sort(privs)
		return true, S("Available privileges:").."\n"..table.concat(privs, "\n")
	else
		local cmd = args[1]
		local def = core.registered_chatcommands[cmd]
		if not def then
			local msg
			if INIT == "game" then
				msg = S("Command not available: @1", cmd)
			else
				msg = core.gettext("Command not available: ") .. cmd
			end
			return false, msg
		else
			return true, format_help_line(cmd, def)
		end
	end
end

if INIT == "client" then
	function core.register_list_command(command, desc, setting)
		local def = {}
		def.description = desc
		def.params = "del <item> | add <item> | list | clear"
		def.list_setting = setting
		function def.func(param)
			local list = (minetest.settings:get(setting) or ""):split(",")
			if param == "list" then
				return true, table.concat(list, ", ")
			elseif param == "clear" then
				for k in pairs(list) do
					list[k] = nil
				end
				core.settings:set(setting, "")
				return true, "Cleared list."
			else
				local sparam = param:split(" ")
				local cmd = sparam[1]
				local item = sparam[2]
				if cmd == "del" then
					if not item then
						local pt_name = get_pt_name("node")
						if not pt_name then
							return false, "Missing item and pointed node is invalid."
						end

						-- remove pt_name
						local i = table.indexof(list, pt_name)
						if i == -1 then
							return false, pt_name .. " is not on the list."
						else
							table.remove(list, i)
							core.settings:set(setting, table.concat(list, ","))
							return true, "Removed " .. pt_name .. " from the list."
						end

					end

					-- remove item
					local i = table.indexof(list, item)
					if i == -1 then
						return false, item .. " is not on the list."
					else
						table.remove(list, i)
						core.settings:set(setting, table.concat(list, ","))
						return true, "Removed " .. item .. " from the list."
					end

				elseif cmd == "add" then
					if not item then
						local pt_name = get_pt_name("node")
						if not pt_name then
							return false, "Missing item and pointed node is invalid."
						end

						-- add pt_name
						local i = table.indexof(list, pt_name)
						if i ~= -1 then
							return false, pt_name .. " is already on the list."
						else
							table.insert(list, pt_name)
							core.settings:set(setting, table.concat(list, ","))
							return true, "Added " .. pt_name .. " to the list."
						end
					end

					-- add item
					local i = table.indexof(list, item)
					if i ~= -1 then
						return false, item .. " is already on the list."
					else
						table.insert(list, item)
						core.settings:set(setting, table.concat(list, ","))
						return true, "Added " .. item .. " to the list."
					end

				end
			end
			return false, "Invalid usage. (See .help " .. command .. ")"
		end
		core.register_chatcommand(command, def)
	end
	function core.register_player_list_command(command, desc, setting)
		local def = {}
		def.description = desc
		def.params = "del <name> | add <name> | list | clear"
		def.list_setting = setting
		function def.func(param)
			local server_url = minetest.get_server_url()
			if not server_url then
				return false, "This command does not work in singleplayer mode."
			end
			local data = minetest.settings:get_json(setting)
			local list = (data[server_url] or ""):split(",")
			if param == "list" then
				return true, table.concat(list, ", ")
			elseif param == "clear" then
				for k in pairs(list) do
					list[k] = nil
				end
				data[server_url] = ""
				core.settings:set_json(setting, data)
				return true, "Cleared list."
			else
				local sparam = param:split(" ")
				local cmd = sparam[1]
				local item = sparam[2]
				if cmd == "del" then
					if not item then
						local pt_name = get_pt_name("player")
						if not pt_name then
							return false, "Missing player name and pointed player is invalid."
						end

						-- remove pt_name
						local i = table.indexof(list, pt_name)
						if i == -1 then
							return false, pt_name .. " is not on the list."
						else
							table.remove(list, i)
							data[server_url] = table.concat(list, ",")
							core.settings:set_json(setting, data)
							return true, "Removed " .. pt_name .. " from the list."
						end

					end

					-- remove item
					local i = table.indexof(list, item)
					if i == -1 then
						return false, item .. " is not on the list."
					else
						table.remove(list, i)
						data[server_url] = table.concat(list, ",")
						core.settings:set_json(setting, data)
						return true, "Removed " .. item .. " from the list."
					end

				elseif cmd == "add" then
					if not item then
						local pt_name = get_pt_name("player")
						if not pt_name then
							return false, "Missing player name and pointed player is invalid."
						end

						-- add pt_name
						local i = table.indexof(list, pt_name)
						if i ~= -1 then
							return false, pt_name .. " is already on the list."
						else
							table.insert(list, pt_name)
							data[server_url] = table.concat(list, ",")
							core.settings:set_json(setting, data)
							return true, "Added " .. pt_name .. " to the list."
						end
					end

					-- add item
					local i = table.indexof(list, item)
					if i ~= -1 then
						return false, item .. " is already on the list."
					else
						table.insert(list, item)
						data[server_url] = table.concat(list, ",")
						core.settings:set_json(setting, data)
						return true, "Added " .. item .. " to the list."
					end

				end
			end
			return false, "Invalid usage. (See .help " .. command .. ")"
		end
		core.register_chatcommand(command, def)
	end

	function core.register_server_list_command(command, desc, setting)
		local def = {}
		def.description = desc
		def.params = "del | add | list | clear"
		def.list_setting = setting
		function def.func(param)
			local server_url = minetest.get_server_url()
			if not server_url then
				return false, "This command cannot be used in singleplayer mode."
			end

			local list = (minetest.settings:get(setting) or ""):split(",")
			if param == "list" then
				return true, table.concat(list, ", ")
			elseif param == "clear" then
				for k in pairs(list) do
					list[k] = nil
				end
				core.settings:set(setting, "")
				return true, "Cleared list."
			else
				local sparam = param:split(" ")
				local cmd = sparam[1]
				if cmd == "del" then
					-- remove server_url
					local i = table.indexof(list, server_url)
					if i == -1 then
						return false, server_url .. " is not on the list."
					else
						table.remove(list, i)
						core.settings:set(setting, table.concat(list, ","))
						return true, "Removed " .. server_url .. " from the list."
					end

				elseif cmd == "add" then
					-- add server_url
					local i = table.indexof(list, server_url)
					if i ~= -1 then
						return false, server_url .. " is already on the list."
					else
						table.insert(list, server_url)
						core.settings:set(setting, table.concat(list, ","))
						return true, "Added " .. server_url .. " to the list."
					end

				end
			end
			return false, "Invalid usage. (See .help " .. command .. ")"
		end
		core.register_chatcommand(command, def)
	end

	core.register_chatcommand("help", {
		params = core.gettext("[all | <cmd>] [-t]"),
		description = core.gettext("Get help for commands (-t: output in chat)"),
		func = function(param)
			return do_help_cmd(nil, param)
		end,
	})
else
	core.register_chatcommand("help", {
		params = S("[all | privs | <cmd>] [-t]"),
		description = S("Get help for commands or list privileges (-t: output in chat)"),
		func = do_help_cmd,
	})
end
