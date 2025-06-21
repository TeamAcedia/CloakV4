-- Credits to Lizzy Fleckenstein

local drop_action = InventoryAction("drop")

local strip_move_act = InventoryAction("move")
strip_move_act:to("current_player", "craft", 1)
local strip_craft_act = InventoryAction("craft")
strip_craft_act:craft("current_player")
local strip_move_back_act = InventoryAction("move")
strip_move_back_act:from("current_player", "craftresult", 1)

core.register_globalstep(function(dtime)
	local player = core.localplayer
	if not player then return end
	local item = player:get_wielded_item()
	local itemdef = core.get_item_def(item:get_name())
	local wieldindex = player:get_wield_index()
	-- AutoRefill
	if (core.settings:get_bool("autorefill") or core.settings:get_bool("scaffold") or core.settings:get_bool("scaffold_plus")) and itemdef then
		local space = item:get_free_space()
		local i = core.find_additional_items(item:get_name(), wieldindex+1)
		local total_items = core.get_total_items(item:get_name())
		if i and space > 0 then
			local move_act = InventoryAction("move")
			move_act:to("current_player", "main", wieldindex + 1)
			move_act:from("current_player", "main", i)
			move_act:set_count(space)
			move_act:apply()
		end
	end
	-- AutoPlanks (Strip in DF)
	if core.settings:get_bool("autoplanks") then
		if itemdef and itemdef.groups.tree and player:get_control().place then
			strip_move_act:from("current_player", "main", wieldindex)
			strip_move_back_act:to("current_player", "main", wieldindex)
			strip_move_act:apply()
			strip_craft_act:apply()
			strip_move_back_act:apply()
		end
	end
	-- AutoEject
	if core.settings:get_bool("autoeject") then
		local list = (core.settings:get("eject_items") or ""):split(",")
		local inventory = core.get_inventory("current_player")
		for index, stack in pairs(inventory.main) do
			if table.indexof(list, stack:get_name()) ~= -1 then
				drop_action:from("current_player", "main", index)
				drop_action:apply()
			end
		end
	end
end)

core.register_list_command("eject", "Configure AutoEject", "eject_items")

-- Enderchest

function get_itemslot_bg(x, y, w, h)
	local out = ""
	for i = 0, w - 1, 1 do
		for j = 0, h - 1, 1 do
			out = out .."image["..x+i..","..y+j..";1,1;mcl_formspec_itemslot.png]"
		end
	end
	return out
end

local enderchest_formspec = "size[9,8.75]"..
	"label[0,0;"..core.formspec_escape(core.colorize("#313131", "Ender Chest")).."]"..
	"list[current_player;enderchest;0,0.5;9,3;]"..
	get_itemslot_bg(0,0.5,9,3)..
	"label[0,4.0;"..core.formspec_escape(core.colorize("#313131", "Inventory")).."]"..
	"list[current_player;main;0,4.5;9,3;9]"..
	get_itemslot_bg(0,4.5,9,3)..
	"list[current_player;main;0,7.74;9,1;]"..
	get_itemslot_bg(0,7.74,9,1)..
	"listring[current_player;enderchest]"..
	"listring[current_player;main]"

function core.open_enderchest()
	core.show_formspec("inventory:enderchest", enderchest_formspec)
end

-- HandSlot

local hand_formspec = "size[9,8.75]"..
	"label[0,0;"..core.formspec_escape(core.colorize("#313131", "Hand")).."]"..
	"list[current_player;hand;0,0.5;1,1;]"..
	get_itemslot_bg(0,0.5,1,1)..
	"label[0,4.0;"..core.formspec_escape(core.colorize("#313131", "Inventory")).."]"..
	"list[current_player;main;0,4.5;9,3;9]"..
	get_itemslot_bg(0,4.5,9,3)..
	"list[current_player;main;0,7.74;9,1;]"..
	get_itemslot_bg(0,7.74,9,1)..
	"listring[current_player;hand]"..
	"listring[current_player;main]"
	
function core.open_handslot()
	core.show_formspec("inventory:hand", hand_formspec)
end

core.register_cheat("AutoEject", "Misc", "autoeject")
core.register_cheat("Hand", "Misc", core.open_handslot)
core.register_cheat("Enderchest", "Misc", core.open_enderchest)
core.register_cheat("AutoPlanks", "Misc", "autoplanks")
core.register_cheat("AutoRefill", "Misc", "autorefill")