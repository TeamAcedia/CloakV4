core.register_cheat_setting("min HP", "Combat", "combatlog", "combatlog.hp", {type="slider_int", min=1, max=10, steps=10})

minetest.register_on_damage_taken(function(hp)
	if minetest.settings:get_bool("combatlog") then
		local hhp=minetest.localplayer:get_hp()
		if (hhp <= tonumber(core.settings:get("combatlog.hp")) ) then
			sh=true
                minetest.disconnect()
		end
    end
end)
minetest.register_cheat_with_infotext("CombatLog", "Combat", "combatlog", "Min hp:")