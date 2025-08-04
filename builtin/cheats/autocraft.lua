
--------------------------------------------------------------------------------
-- Auto Craft, Credits to @nathan422 on discord
--------------------------------------------------------------------------------


-- Auto Crafting Client-side Mod with Recipe Queue for Minetest
-- When the mod is enabled (i.e. when the "autocraft" setting is true),
-- it opens a formspec that lets the user design a 3×3 recipe (using nine text fields).
-- The user may enqueue recipes, reorder or edit them.
-- The auto‑crafting globalstep uses the crafted item’s actual stack size (from the
-- "craftresult" inventory) once the craft has been performed, rather than using
-- minetest.get_craft_result().

--------------------------------------------------------------------------------
-- Global Variables & Configuration
--------------------------------------------------------------------------------
local COOLDOWN = 0.2  -- seconds between auto-crafting attempts
local timer = 0

-- The queue of recipes; each entry is a table with key "grid",
-- a list of 9 strings corresponding to the 3×3 grid. Empty strings are allowed.
local queued_recipes = {}  
local current_recipe_index = 1  -- which queued recipe should be auto-crafted next
local editing_index = nil       -- if set, indicates that a particular recipe is being edited

--------------------------------------------------------------------------------
-- Helper Function: Create a move action (shifting items between inventories)
--------------------------------------------------------------------------------
local function move_item(from_list, from_index, to_list, to_index, count)
  local action = InventoryAction("move")
  action:from("current_player", from_list, from_index)
  action:to("current_player", to_list, to_index)
  action:set_count(count)
  return action
end

local function move_all_crafting_items_to_main()

  local inv = minetest.get_inventory("current_player")
  local craft_list = "craft"
  local main_list = "main"
  local craft_size = #inv.craftresult
  local main_size = #inv.main

  -- Loop over each slot in the crafting grid
  for craft_index = 1, craft_size do
    local stack = inv.craft[craft_index]
    if not stack:is_empty() then
      local item_name = stack:get_name()
      local moving_count = stack:get_count()

      -- First, try to merge into partially filled stacks in the main inventory.
      
	  for main_index, main_stck in pairs(inv.main) do
	    local main_stack = main_stck
		    -- Check if the slot is not empty and contains the same item.
		    if not main_stack:is_empty() and main_stack:get_name() == item_name then
		      local stack_max = main_stack:get_stack_max()  -- Maximum allowed in the stack.
		      local free_space = stack_max - main_stack:get_count()
		      if free_space > 0 then
		        local to_move = math.min(moving_count, free_space)
		        local action = move_item(craft_list, craft_index, main_list, main_index, to_move)
		        -- Execute the inventory action as appropriate.
		        -- For example: inventory_manager:apply_action(action)
		        moving_count = moving_count - to_move
		      end
		    end
		end

      -- Next, if items remain, try to move them into a completely empty slot.
      if moving_count > 0 then
        for main_index, main_stck in pairs(inv.main) do
          if moving_count <= 0 then
            break
          end
          local main_stack = main_stck
          if main_stack:is_empty() then
            -- Move all remaining items into this empty slot.
            local action = move_item(craft_list, craft_index, main_list, main_index, moving_count)
            -- Execute the inventory action as appropriate.
            moving_count = 0
            break  -- Exit once all remaining items are moved.
          end
        end
      end
    end
  end
end

--------------------------------------------------------------------------------
-- Helper Function: Build the Formspec for recipe management
--------------------------------------------------------------------------------
local function build_formspec(prefill)
  -- prefill is a table of 9 strings; if not provided, use nine blanks.
  prefill = prefill or {"", "", "", "", "", "", "", "", ""}
  local formspec =
    "size[8,12]" ..
    "label[0,0;Auto Crafting Recipe Manager]" ..
    "label[0,0.5;Enter Recipe (3x3 Grid):]" ..
    "field[0.5,1;2,0.5;slot1;Slot 1;" .. prefill[1] .. "]" ..
    "field[3,1;2,0.5;slot2;Slot 2;" .. prefill[2] .. "]" ..
    "field[6,1;2,0.5;slot3;Slot 3;" .. prefill[3] .. "]" ..
    "field[0.5,2;2,0.5;slot4;Slot 4;" .. prefill[4] .. "]" ..
    "field[3,2;2,0.5;slot5;Slot 5;" .. prefill[5] .. "]" ..
    "field[6,2;2,0.5;slot6;Slot 6;" .. prefill[6] .. "]" ..
    "field[0.5,3;2,0.5;slot7;Slot 7;" .. prefill[7] .. "]" ..
    "field[3,3;2,0.5;slot8;Slot 8;" .. prefill[8] .. "]" ..
    "field[6,3;2,0.5;slot9;Slot 9;" .. prefill[9] .. "]" ..
    -- Hidden field to track which recipe is being edited.
    (editing_index and "field[0,3.5;0,0;edit_index;;" .. editing_index .. "]" or "field[0,3.5;0,0;edit_index;;]") ..
    "button[0.5,4.2;3,0.8;enqueue_recipe;" .. (editing_index and "Update Recipe" or "Enqueue Recipe") .. "]" ..
    "button[4.5,4.2;3,0.8;clear_form;Clear]" ..
    "label[0,5.2;Queued Recipes:]"
  local y = 6
  if #queued_recipes == 0 then
    formspec = formspec .. "label[0," .. y .. ";(No recipes enqueued)]"
    y = y + 1
  else
    for i, recipe in ipairs(queued_recipes) do
      local recipe_line = "Recipe " .. i .. ": ["
      for j = 1, 9 do
        local item = recipe.grid[j] or ""
        if item == "" then item = "-" end
        recipe_line = recipe_line .. item .. (j < 9 and ", " or "")
      end
      recipe_line = recipe_line .. "]"
      formspec = formspec ..
        "label[0," .. y .. ";" .. minetest.formspec_escape(recipe_line) .. "]" ..
        "button[6.5," .. y .. ";0.8,0.5;moveup_" .. i .. ";Up]" ..
        "button[7.4," .. y .. ";0.8,0.5;movedown_" .. i .. ";Down]" ..
        "button[6.5," .. (y+0.6) .. ";0.8,0.5;edit_" .. i .. ";Edit]" ..
        "button[7.4," .. (y+0.6) .. ";0.8,0.5;delete_" .. i .. ";Del]"
      y = y + 1.3
    end
  end
  formspec = formspec ..
    "button[0.5,10.5;3,0.8;refresh;Refresh]" ..
    "button[4.5,10.5;3,0.8;close;Close]"
  return formspec
end

--------------------------------------------------------------------------------
-- Formspect Input Handler: Manage recipe queue (enqueue, edit, reorder, delete)
--------------------------------------------------------------------------------
minetest.register_on_formspec_input(function(formname, fields)
  if formname ~= "autocraft:main" then
    return
  end

  if fields.enqueue_recipe then
    -- When the "Enqueue Recipe" (or Update) button is pressed, collect the 9 grid fields.
    local new_recipe = { grid = {} }
    for i = 1, 9 do
      new_recipe.grid[i] = fields["slot" .. i] or ""
    end
    local idx = tonumber(fields.edit_index) or editing_index
    if idx then
      queued_recipes[idx] = new_recipe
      editing_index = nil
    else
      table.insert(queued_recipes, new_recipe)
    end
    minetest.show_formspec("autocraft:main", build_formspec())
    return true
  end

  if fields.clear_form then
    editing_index = nil
    minetest.show_formspec("autocraft:main", build_formspec())
    return true
  end

  -- Process buttons for editing, deleting, or reordering queued recipes.
  for key, _ in pairs(fields) do
    if key:find("edit_") then
      local idx = tonumber(key:sub(6))
      if idx and queued_recipes[idx] then
        editing_index = idx
        minetest.show_formspec("autocraft:main", build_formspec(queued_recipes[idx].grid))
      end
      return true
    elseif key:find("delete_") then
      local idx = tonumber(key:sub(8))
      if idx and queued_recipes[idx] then
        table.remove(queued_recipes, idx)
        if current_recipe_index > #queued_recipes then
          current_recipe_index = 1
        end
        minetest.show_formspec("autocraft:main", build_formspec())
      end
      return true
    elseif key:find("moveup_") then
      local idx = tonumber(key:sub(7))
      if idx and idx > 1 then
        queued_recipes[idx], queued_recipes[idx - 1] = queued_recipes[idx - 1], queued_recipes[idx]
        if current_recipe_index == idx or current_recipe_index == idx - 1 then
          current_recipe_index = (current_recipe_index == idx) and idx - 1 or idx
        end
        minetest.show_formspec("autocraft:main", build_formspec())
      end
      return true
    elseif key:find("movedown_") then
      local idx = tonumber(key:sub(9))
      if idx and idx < #queued_recipes then
        queued_recipes[idx], queued_recipes[idx + 1] = queued_recipes[idx + 1], queued_recipes[idx]
        if current_recipe_index == idx or current_recipe_index == idx + 1 then
          current_recipe_index = (current_recipe_index == idx) and idx + 1 or idx
        end
        minetest.show_formspec("autocraft:main", build_formspec())
      end
      return true
    end
  end

  if fields.refresh then
    minetest.show_formspec("autocraft:main", build_formspec())
    return true
  end

  if fields.close then
    minetest.close_formspec(nil, "autocraft:main")
    return true
  end

  return false
end)

--------------------------------------------------------------------------------
-- Open the formspec at startup if auto-crafting is enabled
--------------------------------------------------------------------------------
--[[minetest.after(0.1, function()
  if core.settings:get_bool("autocraft") then
    minetest.show_formspec("autocraft:main", build_formspec())
  end
end)]]--

minetest.register_globalstep(function(dtime) 

	if core.settings:get_bool("autocraft_menu") then
	
		core.settings:set_bool("autocraft_menu", false)
		minetest.show_formspec("autocraft:main", build_formspec())
	end


end)

--------------------------------------------------------------------------------
-- Globalstep: Process Auto-Crafting from the Recipe Queue
--------------------------------------------------------------------------------
minetest.register_globalstep(function(dtime)
  if not core.settings:get_bool("autocraft") then
    return
  end

  if #queued_recipes == 0 then
    return  -- nothing to craft
  end

  local player = core.localplayer
  if not player then
    return
  end

  timer = timer + dtime
  if timer < COOLDOWN then
    return
  end
  timer = 0

  -- Check that the player is within 5 nodes of a crafting table.
  local pos = player:get_pos()
  local near_table = minetest.find_node_near(pos, 5, {"mcl_crafting_table:crafting_table"})
  if not near_table then
    return
  end

  local inv = minetest.get_inventory("current_player")
  if not inv then
    return
  end

  -- Fetch the current queued recipe.
  local recipe = queued_recipes[current_recipe_index]
  if not recipe then
    return
  end
  local recipe_grid = recipe.grid

  -- Build a craft definition using the 3x3 grid from the recipe.
  local craft_def = { method = "normal", width = 3, items = {} }
  for i = 1, 9 do
    craft_def.items[i] = recipe_grid[i] or ""
  end

  --move any incomplete crafting ingredients to the player's inventory.
  move_all_crafting_items_to_main()

  -- Pre-check: verify sufficient ingredients across all inventory stacks.
  local required_counts = {}
  for i = 1, 9 do
    local item = recipe_grid[i]
    if item and item ~= "" then
      required_counts[item] = (required_counts[item] or 0) + 1
    end
  end

  local ingredient_sources = {}
  for i, stack in pairs(inv.main) do
    local name = stack:get_name()
    if required_counts[name] then
      ingredient_sources[name] = ingredient_sources[name] or {}
      table.insert(ingredient_sources[name], { index = i, count = stack:get_count() })
    end
  end

  for ingredient, req_count in pairs(required_counts) do
    local total = 0
    if ingredient_sources[ingredient] then
      for _, source in ipairs(ingredient_sources[ingredient]) do
        total = total + source.count
      end
    end
    if total < req_count then
      return  -- insufficient ingredients in inventory
    end
  end

  -- Stage ingredients into the crafting grid from any available stacks.
  for slot = 1, 9 do
    local item = recipe_grid[slot]
    if item and item ~= "" then
      local sources = ingredient_sources[item]
      local taken = false
      for _, source in ipairs(sources) do
        if source.count > 0 then
          source.count = source.count - 1
          local act = move_item("main", source.index, "craft", slot, 1)
          act:apply()
          taken = true
          break
        end
      end
      if not taken then
        return
      end
    end
  end

  -- Perform the crafting operation.
  local craft_act = InventoryAction("craft")
  craft_act:craft("current_player")
  craft_act:apply()

  -- *** New Step ***
  -- Check the crafted result from the "craftresult" inventory BEFORE moving it.
  local result_stack = inv.craftresult[1]
  if result_stack:is_empty() then
    return  -- The recipe produced no valid output.
  end

  local crafted_item = result_stack:get_name()
  local crafted_count = result_stack:get_count()

  -- Determine the maximum allowed stack size for the crafted item.
  local max_allowed
  if string.find(crafted_item, "tool") then
    max_allowed = 1
  elseif string.find(crafted_item, "pearl") then
    max_allowed = 16
  else
    max_allowed = 64
  end

  -- Determine a destination slot in the player's main inventory that can accept the entire crafted stack.
  local free_result_slot = nil
  local wield_index = player:get_wield_index() + 1

  if inv.main[wield_index] and (inv.main[wield_index]:is_empty() or
     (inv.main[wield_index]:get_name() == crafted_item and inv.main[wield_index]:get_count() + crafted_count <= max_allowed))
  then
    free_result_slot = wield_index
  end

  if not free_result_slot then
    for i, stack in pairs(inv.main) do
      if stack:is_empty() then
        free_result_slot = i
        break
      elseif stack:get_name() == crafted_item and stack:get_count() + crafted_count <= max_allowed then
        free_result_slot = i
        break
      end
    end
  end

  if not free_result_slot then
  
  	core.settings:set_bool("autocraft", false)
    return
  end

  -- Move the crafted result from "craftresult" into the chosen main inventory slot.
  local result_move = move_item("craftresult", 1, "main", free_result_slot, crafted_count)
  result_move:apply()


  -- Advance the pointer so the next auto-craft uses the next enqueued recipe.
  current_recipe_index = current_recipe_index + 1
  if current_recipe_index > #queued_recipes then
    current_recipe_index = 1
  end
end)

--------------------------------------------------------------------------------
-- Register a cheat command (optional) to enable the mod via settings
--------------------------------------------------------------------------------
core.register_cheat("AutoCraft", "Bots", "autocraft")
core.register_cheat("AutoCraft_Menu", "Bots", "autocraft_menu")
