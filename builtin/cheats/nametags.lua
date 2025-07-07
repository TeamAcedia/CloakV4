--Credits to Lizzy Fleckenstein
nametags = {}

function nametags.update_nametag(obj)
    if obj:is_player() and not obj:is_local_player() then
        local props = obj:get_properties()
        if not props then return end
        local nametag = props.nametag or ""        
        local hp = obj:get_hp()
        local idx = nametag:find(" ")
        local nametag_color = ""
        local status = ""

        if not core.settings:get_bool("colored_nametags") then
            nametag_color = "#FFFFFF"
            status = ""
        end
        local player = core.localplayer
        if not player then return end
        local obj_id = obj:get_id()
        if not obj_id then return end
        local relationship = player:get_entity_relationship(obj_id)
        if relationship == core.EntityRelationship.ENEMY then
            nametag_color = "#FF0000"
            status = "ENEMY"
        elseif relationship == core.EntityRelationship.FRIEND then
            nametag_color = "#FF00FF"
            status = "FRIEND"
        elseif relationship == core.EntityRelationship.ALLY then
            nametag_color = "#00FF00"
            status = "ALLY"
        elseif relationship == core.EntityRelationship.STAFF then
            nametag_color = "#0000FF"
            status = "STAFF"
        else
            nametag_color = "#FFFFFF"
            status = "LOADING..."
        end

        if idx then
            if core.settings:get_bool("nametags.hp") and core.settings:get_bool("nametags") and core.settings:get_bool("nametags.status") then
                nametag = nametag:sub(1, idx + 2) .. hp .. core.get_color_escape_sequence(nametag_color) .. " [" .. status .. "]"
            elseif core.settings:get_bool("nametags.hp") and core.settings:get_bool("nametags") then
                nametag = nametag:sub(1, idx + 2) .. hp
            elseif core.settings:get_bool("nametags.status") and core.settings:get_bool("nametags") then
                nametag = nametag:sub(1, idx + 2) .. core.get_color_escape_sequence(nametag_color) .. " [" .. status .. "]"
            else
                nametag = nametag:sub(1, idx - 1)
            end
        else
            if core.settings:get_bool("nametags.hp") and core.settings:get_bool("nametags") and core.settings:get_bool("nametags.status") then
                nametag = nametag .. core.get_color_escape_sequence("#00FF00") .. " " .. hp .. core.get_color_escape_sequence(nametag_color) .. " [" .. status .. "]"
            elseif core.settings:get_bool("nametags.hp") and core.settings:get_bool("nametags") then
                nametag = nametag .. core.get_color_escape_sequence("#00FF00") .. " " .. hp
            elseif core.settings:get_bool("nametags.status") and core.settings:get_bool("nametags") then
                nametag = nametag .. core.get_color_escape_sequence("#00FF00") .. " " .. core.get_color_escape_sequence(nametag_color) .. " [" .. status .. "]"    
            else
                nametag = nametag
            end
        end

        obj:set_properties({nametag = nametag})
    end
end

core.register_globalstep(function(dtime)
    local player = core.localplayer
    if not player then return end

    local objects = core.get_objects_inside_radius(player:get_pos(), 100)
    for _, obj in ipairs(objects) do
        if obj:is_player() and not obj:is_local_player() then
            nametags.update_nametag(obj)
        end
    end
end)

minetest.register_on_object_properties_change(nametags.update_nametag)
minetest.register_on_object_hp_change(nametags.update_nametag)