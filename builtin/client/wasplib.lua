-- Credits to cora

ws = {}
autotool = {}
ws.registered_globalhacks = {}
ws.displayed_wps={}

ws.c = core

local nextact = {}
local ghwason={}

function ws.s(name,value)
    if value == nil then
        return ws.c.settings:get(name)
    else
        ws.c.settings:set(name,value)
        return ws.c.settings:get(name)
    end
end
function ws.sb(name,value)
    if value == nil then
        return ws.c.settings:get_bool(name)
    else
        ws.c.settings:set_bool(name,value)
        return ws.c.settings:get_bool(name)
    end
end

function ws.dcm(msg)
    return minetest.display_chat_message(msg)
end
function ws.set_bool_bulk(settings,value)
    if type(settings) ~= 'table' then return false end
    for k,v in pairs(settings) do
        minetest.settings:set_bool(v,value)
    end
    return true
end

function ws.shuffle(tbl)
  for i = #tbl, 2, -1 do
    local j = math.random(i)
    tbl[i], tbl[j] = tbl[j], tbl[i]
  end
  return tbl
end

function ws.in_list(val, list)
    if type(list) ~= "table" then return false end
    for i, v in ipairs(list) do
        if v == val then
            return true
        end
    end
    return false
end

function ws.random_table_element(tbl)
    local ks = {}
    for k in pairs(tbl) do
        table.insert(ks, k)
    end
    return tbl[ks[math.random(#ks)]]
end

function ws.globalhacktemplate(setting,func,funcstart,funcstop,daughters)
    funcstart = funcstart or function() end
    funcstop = funcstop or function() end
    return function()
        if not minetest.localplayer then return end
        if minetest.settings:get_bool(setting) then
            if nextact[setting] and nextact[setting] > os.clock() then return end
            nextact[setting] = os.clock() + 0.1
            if not ghwason[setting] then
                if not funcstart() then
                    ws.set_bool_bulk(daughters,true)
                    ghwason[setting] = true
                    ws.dcm(setting.. " activated")
                else minetest.settings:set_bool(setting,false)
                end
            else
                func()
            end

        elseif ghwason[setting] then
            ghwason[setting] = false
            ws.set_bool_bulk(daughters,false)
            funcstop()
            ws.dcm(setting.. " deactivated")
        end
    end
end

function ws.register_globalhack(func)
    table.insert(ws.registered_globalhacks,func)
end

function ws.register_globalhacktemplate(name,category,setting,func,funcstart,funcstop,daughters)
    ws.register_globalhack(ws.globalhacktemplate(setting,func,funcstart,funcstop,daughters))
    minetest.register_cheat(name,category,setting)
end

ws.rg=ws.register_globalhacktemplate

function ws.step_globalhacks()
    for i, v in ipairs(ws.registered_globalhacks) do
        v()
    end
end

minetest.register_globalstep(ws.step_globalhacks)

-- COORD MAGIC
function ws.get_reachable_positions(range,under)
    under=under or false
    range=range or 2
    local rt={}
    local lp=minetest.localplayer:get_pos()
    local ylim=range
    if under then ylim=-1 end
    for x = -range,range,1 do
        for y = -range,ylim,1 do
            for z = -range,range,1 do
                table.insert(rt,vector.add(lp,vector.new(x,y,z)))
            end
        end
    end
    return rt
end

function ws.do_area(radius,func,plane)
    for k,v in pairs(ws.get_reachable_positions(range)) do
        if not plane or v.y == minetest.localplayer:get_pos().y -1 then
            func(v)
        end
    end
end


function ws.display_wp(pos,name)
    table.insert(ws.displayed_wps,minetest.localplayer:hud_add({
            hud_elem_type = 'waypoint',
            name          = name,
            text          = name,
            number        = 0x00ff00,
            world_pos     = pos
        }))
end

function ws.clear_wps()
    for k,v in pairs(ws.displayed_wps) do
        minetest.localplayer:hud_remove(v)
        table.remove(ws.displayed_wps,k)
    end
end

function ws.register_chatcommand_alias(old, ...)
      local def = assert(minetest.registered_chatcommands[old])
      def.name = nil
     for i = 1, select('#', ...) do
         minetest.register_chatcommand(select(i, ...), table.copy(def))
     end
end

function ws.round2(num, numDecimalPlaces)
    return tonumber(string.format("%." .. (numDecimalPlaces or 0) .. "f", num))
end

function ws.pos_to_string(pos)
     if type(pos) == 'table' then
         pos = minetest.pos_to_string(vector.round(pos))
     end
     if type(pos) == 'string' then
         return pos
     end
     return pos
end

function ws.string_to_pos(pos)
     if type(pos) == 'string' then
         pos = minetest.string_to_pos(pos)
     end
     if type(pos) == 'table' then
         return vector.round(pos)
     end
     return pos
end

function ws.on_connect(func)
	if not minetest.localplayer then minetest.after(0,function() ws.on_connect(func) end) return end
	if func then func() end
end

--ITEMS
function ws.find_item_in_table(items,rnd)
    if type(items) == 'string' then
        return minetest.find_item(items)
    end
    if type(items) ~= 'table' then return end
    if rnd then items=ws.shuffle(items) end
    for i, v in pairs(items) do
        local n = minetest.find_item(v)
        if n then
            return n
        end
    end
    return false
end

function ws.find_named(inv, name)
	if not inv then return -1 end
    if not name then return end
    for i, v in ipairs(inv) do
        if v:get_name():find(name) then
            return i
        end
    end
end
local hotbar_slot=8
function ws.to_hotbar(it,hslot)
    local tpos=nil
    local plinv = minetest.get_inventory("current_player")
    if hslot and hslot < 10 then
        tpos=hslot
    else
        for i, v in ipairs(plinv.main) do
            if i<10 and v:is_empty() then
                tpos = i
                break
            end
        end
    end
    if tpos == nil then tpos=hotbar_slot end
	local mv = InventoryAction("move")
	mv:from("current_player", "main", it)
	mv:to("current_player", "main", tpos)
	mv:apply()
    return tpos
end

function ws.switch_to_item(itname,hslot)
    if not minetest.localplayer then return false end
    local plinv = minetest.get_inventory("current_player")
    for i, v in ipairs(plinv.main) do
        if i<10 and v:get_name() == itname then
            minetest.localplayer:set_wield_index(i)
            return true
        end
    end
    local pos = ws.find_named(plinv.main, itname)
    if pos then
        minetest.localplayer:set_wield_index(ws.to_hotbar(pos,hslot))
        return true
    end
    return false
end

function core.switch_to_item(item) return ws.switch_to_item(item) end

function ws.switch_inv_or_echest(name,max_count)
	if not minetest.localplayer then return false end
    local plinv = minetest.get_inventory("current_player")
    if ws.switch_to_item(name) then return true end

    local epos = ws.find_named(plinv.enderchest, name)
    if epos then
        local tpos
        for i, v in ipairs(plinv.main) do
            if i < 9 and v:is_empty() then
                tpos = i
                break
            end
        end
        if not tpos then tpos=hotbar_slot end

        if tpos then
            local mv = InventoryAction("move")
            mv:from("current_player", "enderchest", epos)
            mv:to("current_player", "main", tpos)
            if max_count then
                mv:set_count(max_count)
            end
            mv:apply()
            minetest.localplayer:set_wield_index(tpos)
            return true
        end
    end
    return false
end
-- TOOLS


local function check_tool(stack, node_groups, old_best_time)
	local toolcaps = stack:get_tool_capabilities()
	if not toolcaps then return end
	local best_time = old_best_time
	for group, groupdef in pairs(toolcaps.groupcaps) do
		local level = node_groups[group]
		if level then
			local this_time = groupdef.times[level]
			if this_time and this_time < best_time then
				best_time = this_time
			end
		end
	end
	return best_time < old_best_time, best_time
end

local function find_best_tool(nodename, switch)
	local player = minetest.localplayer
	local inventory = minetest.get_inventory("current_player")
	local node_groups = minetest.get_node_def(nodename).groups
	local new_index = player:get_wield_index()
	local is_better, best_time = false, math.huge

	is_better, best_time = check_tool(player:get_wielded_item(), node_groups, best_time)
	if inventory.hand then
	    is_better, best_time = check_tool(inventory.hand[1], node_groups, best_time)
    end

	for index, stack in ipairs(inventory.main) do
		is_better, best_time = check_tool(stack, node_groups, best_time)
		if is_better then
			new_index = index
		end
	end

	return new_index,best_time
end

function ws.get_digtime(nodename)
    local idx,tm=find_best_tool(nodename)
    return tm
end

function ws.select_best_tool(pos)
    local nd=minetest.get_node_or_nil(pos)
    local nodename='air'
    if nd then nodename=nd.name end
    local t=find_best_tool(nodename)
    minetest.localplayer:set_wield_index(ws.to_hotbar(t,hotbar_slot))
	--minetest.localplayer:set_wield_index(find_best_tool(nodename))
end

--- COORDS
function ws.coord(x, y, z)
    return vector.new(x,y,z)
end
function ws.ordercoord(c)
    if c.x == nil then
        return {x = c[1], y = c[2], z = c[3]}
    else
        return c
    end
end

-- x or {x,y,z} or {x=x,y=y,z=z}
function ws.optcoord(x, y, z)
    if y and z then
        return ws.coord(x, y, z)
    else
        return ws.ordercoord(x)
    end
end
function ws.cadd(c1, c2)
    return vector.add(c1,c2)
    --return ws.coord(c1.x + c2.x, c1.y + c2.y, c1.z + c2.z)
end

function ws.relcoord(x, y, z, rpos)
    local pos = rpos or minetest.localplayer:get_pos()
    pos.y=math.ceil(pos.y)
    --math.floor(pos.y) + 0.5
    return ws.cadd(pos, ws.optcoord(x, y, z))
end

local function between(x, y, z) -- x is between y and z (inclusive)
    return y <= x and x <= z
end

function ws.getdir() --
    local rot = minetest.localplayer:get_yaw() % 360
    if between(rot, 315, 360) or between(rot, 0, 45) then
        return "north"
    elseif between(rot, 135, 225) then
        return "south"
    elseif between(rot, 225, 315) then
        return "east"
    elseif between(rot, 45, 135) then
        return "west"
    end
end
function ws.setdir(dir) --
    if dir == "north" then
        minetest.localplayer:set_yaw(0)
    elseif dir == "south" then
        minetest.localplayer:set_yaw(180)
    elseif dir == "east" then
        minetest.localplayer:set_yaw(270)
    elseif dir == "west" then
        minetest.localplayer:set_yaw(90)
    end
end

function ws.dircoord(f, y, r ,rpos)
    local dir=ws.getdir()
    local coord = ws.optcoord(f, y, r)
    local f = coord.x
    local y = coord.y
    local r = coord.z
    local lp= rpos or minetest.localplayer:get_pos()
    if dir == "north" then
        return ws.relcoord(r, y, f,rpos)
    elseif dir == "south"  then
        return ws.relcoord(-r, y, -f,rpos)
    elseif dir == "east" then
        return ws.relcoord(f, y, -r,rpos)
    elseif dir== "west" then
        return ws.relcoord(-f, y, r,rpos)
    end
    return ws.relcoord(0, 0, 0,rpos)
end

function ws.get_dimension(pos)
    if pos.y > -65 then return "overworld"
    elseif pos.y > -8000 then return "void"
    elseif pos.y > -27000 then return "end"
    elseif pos.y > -28930 then return "void"
    elseif pos.y > -31000 then return "nether"
    else return "void"
    end
end

function ws.aim(tpos)
    local ppos=minetest.localplayer:get_pos()
    local dir=vector.direction(ppos,tpos)
    local yyaw=0;
    local pitch=0;
    if dir.x < 0 then
        yyaw = math.atan2(-dir.x, dir.z) + (math.pi * 2)
    else
        yyaw = math.atan2(-dir.x, dir.z)
    end
    yyaw = ws.round2(math.deg(yyaw),2)
    pitch = ws.round2(math.deg(math.asin(-dir.y) * 1),2);
    minetest.localplayer:set_yaw(yyaw)
    minetest.localplayer:set_pitch(pitch)
end

function ws.gaim(tpos,v,g)
    local v = v or 40
    local g = g or 9.81
    local ppos=minetest.localplayer:get_pos()
    local dir=vector.direction(ppos,tpos)
    local yyaw=0;
    local pitch=0;
    if dir.x < 0 then
        yyaw = math.atan2(-dir.x, dir.z) + (math.pi * 2)
    else
        yyaw = math.atan2(-dir.x, dir.z)
    end
    yyaw = ws.round2(math.deg(yyaw),2)
    local y = dir.y
	dir.y = 0
    local x = vector.length(dir)
    pitch=math.atan(math.pow(v, 2) / (g * x) + math.sqrt(math.pow(v, 4)/(math.pow(g, 2) * math.pow(x, 2)) - 2 * math.pow(v, 2) * y/(g * math.pow(x, 2)) - 1))
    --pitch = ws.round2(math.deg(math.asin(-dir.y) * 1),2);
    minetest.localplayer:set_yaw(yyaw)
    minetest.localplayer:set_pitch(math.deg(pitch))
end

local function tablearg(arg)
    local tb={}
    if type(arg) == 'string' then
        tb={arg}
    elseif type(arg) == 'table' then
        tb=arg
    elseif type(arg) == 'function' then
        tb=arg()
    end
    return tb
end

function ws.isnode(pos,arg)--arg is either an itemstring, a table of itemstrings or a function returning an itemstring
    local nodename=tablearg(arg)
    local nd=minetest.get_node_or_nil(pos)
    if nd and nodename and ws.in_list(nd.name,nodename) then
        return true
    end
end

function ws.can_place_at(pos)
    local node = minetest.get_node_or_nil(pos)
    return (node and (node.name == "air" or node.name=="mcl_core:water_source" or node.name=="mcl_core:water_flowing" or node.name=="mcl_core:lava_source" or node.name=="mcl_core:lava_flowing" or minetest.get_node_def(node.name).buildable_to))
end

-- should check if wield is placeable
-- minetest.get_node(wielded:get_name()) ~= nil should probably work
-- otherwise it equips armor and eats food
function ws.can_place_wielded_at(pos)
    local wield_empty = minetest.localplayer:get_wielded_item():is_empty()
    return not wield_empty and ws.can_place_at(pos)
end


function ws.find_any_swap(items,hslot)
    hslot=hslot or 8
    for i, v in ipairs(items) do
        local n = minetest.find_item(v)
        if n then
            ws.switch_to_item(v,hslot)
            return true
        end
    end
    return false
end


-- swaps to any of the items and places if need be
-- returns true if placed and in inventory or already there, false otherwise

local lastact=0
local lastplc=0
local lastdig=0
local actint=10
function ws.place(pos,items,hslot, place)
    --if not inside_constraints(pos) then return end
    if not pos then return end
    items=tablearg(items)

    place = place or minetest.place_node

    local node = minetest.get_node_or_nil(pos)
    if not node then return end
    -- already there
    if ws.isnode(pos,items) then
        return true
    else
        local swapped = ws.find_any_swap(items,hslot)

        -- need to place
        if swapped and ws.can_place_at(pos) then
            --minetest.after("0.05",place,pos)
            place(pos)
            return true
        -- can't place
        else
            return false
        end
    end
end

function ws.place_if_able(pos)
    if not pos then return end
    if not inside_constraints(pos) then return end
    if ws.can_place_wielded_at(pos) then
        minetest.place_node(pos)
    end
end

function ws.is_diggable(pos)
    if not pos then return false end
    local nd=minetest.get_node_or_nil(pos)
    if not nd then return false end
    local n = minetest.get_node_def(nd.name)
    if n and n.diggable then return true end
    return false
end

function ws.dig(pos,condition,autotool)
    --if not inside_constraints(pos) then return end
    if autotool == nil then autotool = true end
    if condition and not condition(pos) then return false end
    if not ws.is_diggable(pos) then return end
    local nd=minetest.get_node_or_nil(pos)
    if nd and minetest.get_node_def(nd.name).diggable then
        if autotool then ws.select_best_tool(pos) end
        minetest.dig_node(pos)
    end
    return true
end

function ws.chunk_loaded()
	local ign=minetest.find_nodes_near(ws.dircoord(0,0,0),10,{'ignore'},true)
	if #ign == 0 then return true end
	return false
end

function ws.get_near(nodes,range)
    range=range or 5
    local nds=minetest.find_nodes_near(ws.dircoord(0,0,0),rang,nodes,true)
    if #nds > 0 then return nds end
    return false
end


local dn_i=0
function ws.dignodes(poss,condition)
    if dn_i > 8 then minetest.after("1.0",function()
            ws.dignodes(poss,condition)
            dn_i=0
            return
        end)
    end
    dn_i = dn_i + 1
    for k,v in pairs(poss) do
        if not condition or condition(v) then ws.dig(v) end
    end
end
function ws.replace(pos,arg)
    if not isnode(pos,arg) then ws.dig(pos,function() return ws.isnode(pos,arg) end) end
    ws.place(pos,arg)
end

function ws.playeron(p)
	local pls=minetest.get_player_names()
	for k,v in pairs(pls) do
		if v == p then return true end
	end
	return false
end


function autotool.select_best_tool(nodename)
	return ws.select_best_tool(nodename)
end
