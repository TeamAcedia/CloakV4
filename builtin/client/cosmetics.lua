
core.load_media("custom_character.b3d")
core.load_media("empty.png")
core.load_media("cape.png")

local update_interval = 0.25
local timer = 0
local updated_once = false

core.register_globalstep(function(dtime)
	if updated_once then return end

	timer = timer + dtime
	if timer >= update_interval then
		timer = 0

		if core.localplayer then
			local obj = core.localplayer:get_object()
			if obj then
				local props = obj:get_properties()
				local current_textures = props.textures or {}
				local current_texture = current_textures[1] or "character.png"
				local overlay_texture = "cape.png"
				local combined_texture = current_texture .. "^" .. overlay_texture

				-- Preserve texture slots 2–4 if already set
				local t2 = current_textures[2] or "empty.png"
				local t3 = current_textures[3] or "empty.png"
				local t4 = current_textures[4] or "empty.png"

				obj:set_properties({
					textures = {
						combined_texture,
						t2,
						t3,
						t4,
					},
					mesh = "custom_character.b3d",
				})

				-- Only update once
				updated_once = true
			end
		end
	end
end)
