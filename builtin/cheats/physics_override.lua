-- Credits to Lizzy Fleckenstein
-- lgpl license

local legit_override

core.register_globalstep(function()
	if not legit_override then return end
	if not core.localplayer then return end

	if not core.settings:get_bool("velocity") then
		core.localplayer:set_physics_override(legit_override)
	else
		local override = table.copy(legit_override)
		override.speed = override.speed * tonumber(core.settings:get("velocity.speed"))
		override.jump = override.jump * tonumber(core.settings:get("velocity.jump"))
		override.gravity = override.gravity * tonumber(core.settings:get("velocity.gravity"))
		override.speed_crouch = override.speed_crouch * tonumber(core.settings:get("velocity.speed_crouch"))
		core.localplayer:set_physics_override(override)
	end

	if core.settings:get_bool("overrides") then
		local override = core.localplayer:get_physics_override()
		override.sneak_glitch = core.settings:get_bool("overrides.sneak_glitch")
		override.new_move = not core.settings:get_bool("overrides.old_movement")
		core.localplayer:set_physics_override(override)
	else
		local override = core.localplayer:get_physics_override()
		override.sneak_glitch = legit_override.sneak_glitch
		override.new_move = legit_override.new_move
		core.localplayer:set_physics_override(override)
	end

	if core.settings:get_bool("scaffold.active") then
		local override = core.localplayer:get_physics_override()
		override.jump = 0
		core.localplayer:set_physics_override(override)
	end

	if core.settings:get_bool("overrides.old_movement") then
		core.settings:set_bool("overrides.sneak_glitch", true)
	end
end)

core.register_on_recieve_physics_override(function(override)
	legit_override = override
    return true
end) 

core.register_cheat("Velocity", "Movement", "velocity")
core.register_cheat_setting("Speed", "Movement", "velocity", "velocity.speed", {type="slider_float", min=0.1, max=5.0, steps=50})
core.register_cheat_setting("Sneak Spd", "Movement", "velocity", "velocity.speed_crouch", {type="slider_int", min=1, max=10, steps=10})
core.register_cheat_setting("Jump", "Movement", "velocity", "velocity.jump", {type="slider_float", min=0.1, max=5.0, steps=50})
core.register_cheat_setting("Gravity", "Movement", "velocity", "velocity.gravity", {type="slider_float", min=0.1, max=5.0, steps=50})
core.register_cheat("Overrides", "Movement", "overrides")
core.register_cheat_setting("Old Movement", "Movement", "overrides", "overrides.old_movement", {type="bool"})
core.register_cheat_setting("Sneak Glitch", "Movement", "overrides", "overrides.sneak_glitch", {type="bool"})