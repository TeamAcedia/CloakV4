-- Do this first so you can update infontext later
minetest.register_cheat_with_infotext("FlagAura", "Misc", "flagaura", "")
minetest.register_cheat_setting("Radius", "Misc", "flagaura", "flagaura.range", {type="slider_float", min=1, max=6, steps=6})
core.register_cheat_description("FlagAura", "Misc", "flagaura", "Automatically picks up flags in CTF.")

local function getOtherColors(excludedColor)
    local teamColors = {"red", "blue", "green", "orange", "purple"}
    local otherColors = {}
    for _, color in ipairs(teamColors) do
        if color ~= excludedColor then
            table.insert(otherColors, color)
        end
    end

    return otherColors
end

local function findFlags()
	local player = minetest.localplayer
    if not player then return end
    local tcolor = player:get_teamcolor()
    if not tcolor then return end
    local availableColors = getOtherColors(tcolor)
    local nodeNames = {}

    -- Add the flag nodes for each available color
    for _, color in ipairs(availableColors) do
        table.insert(nodeNames, "ctf_modebase:flag_top_" .. color)
    end
    local foundNodes = {}
    for _, nodeName in ipairs(nodeNames) do
        local nodes = minetest.find_nodes_near(player:get_pos(), tonumber(minetest.settings:get("flagaura.range")), {nodeName})
        for _, pos in ipairs(nodes) do
            table.insert(foundNodes, pos) 
        end
    end

    return foundNodes
end

minetest.register_globalstep(function()
	if not minetest.settings:get_bool("flagaura") then return end
    if core.get_server_game() == "not_initialized" then return end
    if core.get_server_game() ~= "capturetheflag" then
        core.update_infotext("FlagAura", "Misc", "flagaura", "Invalid Game")
        return
    end
	local positions = findFlags()
    if not positions then return end
	for _, pos in ipairs(positions) do
    	minetest.dig_node(pos)
	end
end)