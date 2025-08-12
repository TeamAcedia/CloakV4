chateffects = {}

--[[

 All credit goes to nathan422.

 This function replaces specific ASCII letters with similar-looking letters.
 It is used to bypass chat filters that may block certain words or phrases.

 This file requires frequent updates to keep up with the latest chat filter bypass techniques.
 
--]]
local function replace_ascii_with_similar_text(text)
    local mapping = {
    
			-- Uppercase
		['A'] = 'Α',  -- Greek Alpha
		['B'] = 'Β',  -- Greek Beta
		['C'] = 'С',  -- Cyrillic Es
		--['D'] = 'D',  -- Greek Delta
		['E'] = 'Ε',  -- Greek Epsilon
		--['F'] = 'Ф',  -- Cyrillic Ef
		--['G'] = 'Γ',  -- Greek Gamma
		['H'] = 'Η',  -- Greek Eta
		['I'] = 'Ι',  -- Greek Iota
		['J'] = 'Ј',  -- Cyrillic Je
		['K'] = 'Κ',  -- Greek Kappa
		--['L'] = 'Л',  -- Cyrillic El
		['M'] = 'Μ',  -- Greek Mu
		['N'] = 'Ν',  -- Greek Nu
		['O'] = 'Ο',  -- Greek Omicron
		['P'] = 'Ρ',  -- Greek Rho
		--['Q'] = 'Қ',  -- Cyrillic Ka with descender
		--['R'] = 'Я',  -- Cyrillic Ya
		--['S'] = 'Σ',  -- Greek Sigma
		['T'] = 'Τ',  -- Greek Tau
		--['U'] = 'Ц',  -- Cyrillic Tse
		--['V'] = 'Л',  -- Cyrillic El (inverted V shape)
		--['W'] = 'Ш',  -- Cyrillic Sha
		['X'] = 'Χ',  -- Greek Chi
		['Y'] = 'Υ',  -- Greek Upsilon
		['Z'] = 'Ζ',  -- Greek Zeta

		-- Lowercase
		['a'] = 'α',  -- Greek alpha
		--['b'] = 'β',  -- Greek beta
		['c'] = 'ϲ​',  -- Greek lunate sigma
		--['d'] = 'δ',  -- Greek delta
		--['e'] = 'ε',  -- Greek epsilon
		--['f'] = 'ф',  -- Cyrillic ef
		--['g'] = 'γ',  -- Greek gamma
		--['h'] = 'η',  -- Greek eta
		--['i'] = 'ι',  -- Greek iota
		['j'] = 'ј',  -- Cyrillic je
		['k'] = 'κ',  -- Greek kappa
		--['l'] = 'λ',  -- Greek lamda
		--['m'] = 'μ',  -- Greek mu
		--['n'] = 'ν',  -- Greek nu
		['o'] = 'ο',  -- Greek omicron
		['p'] = 'ρ',  -- Greek rho
		--['q'] = 'қ',  -- Cyrillic ka with descender
		--['r'] = 'я',  -- Cyrillic ya
		--['s'] = 'σ',  -- Greek sigma
		--['t'] = 'τ',  -- Greek tau
		['u'] = 'υ​',  -- Greek upsilon
		['v'] = 'ν',  -- Greek nu
		['w'] = 'ω',  -- Greek omega (closest to “w” curve)
		['x'] = 'χ',  -- Greek chi
		['y'] = 'у',  -- Cyrillic u (better match than Υ)
		--['z'] = 'ζ',  -- Greek zeta
        
    }
    
    return text:gsub(".", function(character)
        return mapping[character] or character
    end)
end

function core.colorize(color, message)
	local lines = tostring(message):split("\n", true)
	local color_code = core.get_color_escape_sequence(color)

	for i, line in ipairs(lines) do
		lines[i] = color_code .. line
	end

	return table.concat(lines, "\n") .. core.get_color_escape_sequence("#ffffff")
end

local function rgb_to_hex(rgb)
	local hexadecimal = "#"

	for key, value in pairs(rgb) do
		local hex = ""

		while(value > 0)do
			local index = math.fmod(value, 16) + 1
			value = math.floor(value / 16)
			hex = string.sub("0123456789ABCDEF", index, index) .. hex
		end

		if(string.len(hex) == 0)then
			hex = "00"
		elseif(string.len(hex) == 1)then
			hex = "0" .. hex
		end

		hexadecimal = hexadecimal .. hex
	end

	return hexadecimal
end

local function color_from_hue(hue)
	local h = hue / 60
	local c = 255
	local x = (1 - math.abs(h % 2 - 1)) * 255

	local i = math.floor(h)
	if i == 0 then
		return rgb_to_hex({c, x, 0})
	elseif i == 1 then
		return rgb_to_hex({x, c, 0})
	elseif i == 2 then
		return rgb_to_hex({0, c, x})
	elseif i == 3 then
		return rgb_to_hex({0, x, c})
	elseif i == 4 then
		return rgb_to_hex({x, 0, c})
	else
		return rgb_to_hex({c, 0, x})
	end
end

function core.rainbow(input)
	local step = 360 / input:len()
	local hue = 0
	local output = ""
	for i = 1, input:len() do
		local char = input:sub(i, i)
		if char:match("%s") then
			output = output .. char
		else
			output = output  .. core.get_color_escape_sequence(color_from_hue(hue)) .. char
		end
		hue = hue + step
	end
	return output
end

function chateffects.send(message)
	if not core.settings:get_bool("use_chat_effects") then return end
	local starts_with = message:sub(1, 1)
	
	if starts_with == "/" or starts_with == "." then return end

	local reverse = core.settings:get_bool("chat_reverse")
	
	if reverse then
		local msg = ""
		for i = 1, #message do
			msg = message:sub(i, i) .. msg
		end
		message = msg
	end
	
	local use_chat_color = core.settings:get_bool("use_chat_color")
	local color = core.settings:get("chat_color") or "rainbow"

	if use_chat_color then
		local msg
		if color == "rainbow" then
			msg = core.rainbow(message)
		else
			msg = core.colorize(color, message)
		end
		message = msg
	end
	
	local bp_filter = core.settings:get_bool("bypass_filter")
	if bp_filter then
		
		local msg = message

		message = replace_ascii_with_similar_text(msg)		

	end

	core.send_chat_message(message)
	return true


end

core.register_on_sending_chat_message(chateffects.send)

core.register_cheat("ChatEffects", "Misc", "use_chat_effects")
core.register_cheat_description("ChatEffects", "Misc", "use_chat_effects", "Custom chat effects, such as color and reverse text.")
core.register_cheat_setting("Colored", "Misc", "use_chat_effects", "use_chat_color", {type="bool"})
core.register_cheat_setting("Chat Color", "Misc", "use_chat_effects", "chat_color", {type="text", size=5})
core.register_cheat_setting("Reversed", "Misc", "use_chat_effects", "chat_reverse", {type="bool"})
core.register_cheat_setting("BypassFilter", "Misc", "use_chat_effects", "bypass_filter", {type="bool"})

