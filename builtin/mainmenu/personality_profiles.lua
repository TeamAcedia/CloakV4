-- Global storage for all loaded profiles
personality_profiles = {}

-- trim helper
local function trim(s)
	return (s:gsub("^%s*(.-)%s*$", "%1"))
end

-- get a sorted list of profile names
function get_profile_names()
	local names = {}
	for name, _ in pairs(personality_profiles) do
		table.insert(names, name)
	end
	table.sort(names)
	return names
end

-- Parse a single profile file into a table
function parse_personality_profile(filepath, filename)
	local profile = {}
	local in_profile_section = false

	local file = io.open(filepath, "r")
	if not file then
		core.log("error", "[profiles] Could not open file: " .. filepath)
		return nil
	end
	profile.id = filename:match("^(.-)%.profile$")
	for line in file:lines() do
		line = trim(line)

		-- skip blank or comment lines
		if line ~= "" and not line:match("^#") then
			-- check for section
			if line:match("^%[profile%]$") then
				in_profile_section = true

			elseif in_profile_section then
				-- parse key = value
				local key, value = line:match("^(.-)%s*=%s*(.-)$")
				if key and value then
					key = trim(key)
					value = trim(value)

					-- convert numeric versions
					if key:match("^version_") and value:match("^%d+$") then
						profile[key] = tonumber(value)
					else
						profile[key] = value
					end
				end
			end
		end
	end

	file:close()
	return profile
end

function load_personality_profiles()
	local folder = core.get_personality_profiles_path()

	local files = core.get_dir_list(folder, false) -- only files
	for _, filename in ipairs(files) do
		if filename:match("%.profile$") then
			local path = folder .. "/" .. filename
			local profile = parse_personality_profile(path, filename)

			-- check that all required keys exist
			local required_keys = {
				"profile_name",
				"version_major",
				"version_minor",
				"version_patch",
				"protocol_version_min",
				"protocol_version_max"
			}

			local has_all_keys = true
			for _, key in ipairs(required_keys) do
				if profile[key] == nil then
					has_all_keys = false
					core.log("warning", "[profiles] Skipped file (missing key '" .. key .. "'): " .. filename)
					break
				end
			end

			if has_all_keys then
				personality_profiles[profile.profile_name] = profile
				core.log("action", "[profiles] Loaded profile: " .. profile.profile_name)
			end
		end
	end
end
