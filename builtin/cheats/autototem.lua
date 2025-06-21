-- Credits to Lizzy Fleckenstein

local switched_to_totem = 0
local totem_move_action = InventoryAction("move")
local last_count
local last_item = "mcl_totems:totem"  -- Set the specific item to track
local epoch = 0
local invalid_game = false
local totem_stack

totem_move_action:to("current_player", "offhand", 1)

local function update_count()
    if minetest.localplayer ~= nil then
        local count = 0
        for k, v in ipairs(minetest.get_inventory("current_player").main) do
            if v:get_name() == last_item then
                count = count + v:get_count()
            end
        end
        last_count = count
        if last_count > 0 then
            if totem_stack:get_name()=="mcl_totems:totem" then
                minetest.update_infotext("AutoTotem", "Combat", "autototem", last_count.."+1")
                else
                minetest.update_infotext("AutoTotem", "Combat", "autototem", last_count)
            end
        else
            if totem_stack:get_name()=="mcl_totems:totem" then
                minetest.update_infotext("AutoTotem", "Combat", "autototem", "0+1")
            else
                minetest.update_infotext("AutoTotem", "Combat", "autototem", "0")
            end
        end
    end
end

minetest.register_globalstep(function(dtime)
    if not minetest.settings:get_bool("autototem") then return end
    local player = minetest.localplayer
    if not player then return end
    local inventory = minetest.get_inventory("current_player")
    if inventory and inventory["offhand"] and inventory.main then
        totem_stack = inventory.offhand[1]
        local totem_index = nil
        for idx, stack in ipairs(inventory.main) do
            if stack:get_name() == "mcl_totems:totem" then
                totem_index = idx
                break
            end
        end
        if totem_stack and totem_stack:get_name() ~= "mcl_totems:totem" and totem_index then
            totem_move_action:from("current_player", "main", totem_index)
            totem_move_action:apply()
        end
        if os.time() > epoch then
            update_count()
            epoch = os.time()
        end
    else
        if invalid_game == false then
            minetest.update_infotext("AutoTotem", "Combat", "autototem", "Invalid Game")
            invalid_game = true
        end
    end
end)

minetest.register_cheat_with_infotext("AutoTotem", "Combat", "autototem", "0")