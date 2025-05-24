-- Minetest
-- Copyright (C) 2013 sapier
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation; either version 2.1 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

local function is_dialog_visible(dlg)
	local this = ui.find_by_name("about_dialog")
	return this == dlg and not dlg.hidden
end
--Minetest
--Copyright (C) 2013 sapier
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 2.1 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

-- https://github.com/orgs/minetest/teams/engine/members

local client_core_developers = {
	"ProunceDev <prouncedev@gmail.com> [Project founder]",
	"plus22 [Core Developer]", 
	"Maintainer_ (FoxLoveFire) <ivanskatov672@gmail.com> [Core Developer]",
	"Astra0081X [Core Developer]",
	"Pl608 [Developer]",
	"Burrowing_Owl [Contributor]"
}

local core_developers = {
	"Perttu Ahola (celeron55) <celeron55@gmail.com> [Project founder]",
	"sfan5 <sfan5@live.de>",
	"ShadowNinja <shadowninja@minetest.net>",
	"Nathanaëlle Courant (Nore/Ekdohibs) <nore@mesecons.net>",
	"Loic Blot (nerzhul/nrz) <loic.blot@unix-experience.fr>",
	"Andrew Ward (rubenwardy) <rw@rubenwardy.com>",
	"Krock/SmallJoker <mk939@ymail.com>",
	"Lars Hofhansl <larsh@apache.org>",
	"v-rob <robinsonvincent89@gmail.com>",
	"Desour/DS",
	"srifqi",
	"Gregor Parzefall (grorp)",
	"Lars Müller (luatic)",
}

-- currently only https://github.com/orgs/minetest/teams/triagers/members

local core_team = {
	"Zughy [Issue triager]",
	"wsor [Issue triager]",
	"Hugo Locurcio (Calinou) [Issue triager]",
}

-- For updating active/previous contributors, see the script in ./util/gather_git_credits.py

local active_contributors = {
	"cx384",
	"numzero",
	"AFCMS",
	"sfence",
	"Wuzzy",
	"ROllerozxa",
	"JosiahWI",
	"OgelGames",
	"David Heidelberg",
	"1F616EMO",
	"HybridDog",
	"Bradley Pierce (Thresher)",
	"savilli",
	"Stvk imension",
	"y5nw",
	"chmodsayshello",
	"jordan4ibanez",
	"superfloh247",
}

local previous_core_developers = {
	"BlockMen",
	"Maciej Kasatkin (RealBadAngel) [RIP]",
	"Lisa Milne (darkrose) <lisa@ltmnet.com>",
	"proller",
	"Ilya Zhuravlev (xyz) <xyz@minetest.net>",
	"PilzAdam <pilzadam@minetest.net>",
	"est31 <MTest31@outlook.com>",
	"kahrl <kahrl@gmx.net>",
	"Ryan Kwolek (kwolekr) <kwolekr@minetest.net>",
	"sapier",
	"Zeno",
	"Auke Kok (sofar) <sofar@foo-projects.org>",
	"Aaron Suen <warr1024@gmail.com>",
	"paramat",
	"Pierre-Yves Rollo <dev@pyrollo.com>",
	"hecks",
	"Jude Melton-Houghton (TurkeyMcMac) [RIP]",
	"Hugues Ross <hugues.ross@gmail.com>",
	"Dmitry Kostenko (x2048) <codeforsmile@gmail.com>",
}

local previous_contributors = {
	"Nils Dagsson Moskopp (erlehmann) <nils@dieweltistgarnichtso.net> [Minetest logo]",
	"red-001 <red-001@outlook.ie>",
	"Giuseppe Bilotta",
	"HybridDog",
	"ClobberXD",
	"Dániel Juhász (juhdanad) <juhdanad@gmail.com>",
	"MirceaKitsune <mirceakitsune@gmail.com>",
	"Jean-Patrick Guerrero (kilbith)",
	"MoNTE48",
	"Constantin Wenger (SpeedProg)",
	"Ciaran Gultnieks (CiaranG)",
	"Paul Ouellette (pauloue)",
	"stujones11",
	"Rogier <rogier777@gmail.com>",
	"Gregory Currie (gregorycu)",
	"JacobF",
	"Jeija <jeija@mesecons.net>",
}

local function prepare_credits(dest, source)
	local string = table.concat(source, "\n") .. "\n"

	local hypertext_escapes = {
		["\\"] = "\\\\",
		["<"] = "\\<",
		[">"] = "\\>",
	}
	string = string:gsub("[\\<>]", hypertext_escapes)
	string = string:gsub("%[.-%]", "<gray>%1</gray>")

	table.insert(dest, string)
end

local function get_formspec(dlgview, name, tabdata)
	local logofile = defaulttexturedir .. "logo.png"
	local version = core.get_version()

	local hypertext = {
		"<tag name=heading color=#ff0>",
		"<tag name=gray color=#aaa>",
	}

	table.insert_all(hypertext, {
		"<heading>", fgettext_ne("Team Acedia"), "</heading>\n",
	})
	prepare_credits(hypertext, client_core_developers)

	table.insert_all(hypertext, {
		"\n",
		"<heading>", fgettext_ne("Core Developers"), "</heading>\n",
	})
	prepare_credits(hypertext, core_developers)
	table.insert_all(hypertext, {
		"\n",
		"<heading>", fgettext_ne("Core Team"), "</heading>\n",
	})
	prepare_credits(hypertext, core_team)
	table.insert_all(hypertext, {
		"\n",
		"<heading>", fgettext_ne("Active Contributors"), "</heading>\n",
	})
	prepare_credits(hypertext, active_contributors)
	table.insert_all(hypertext, {
		"\n",
		"<heading>", fgettext_ne("Previous Core Developers"), "</heading>\n",
	})
	prepare_credits(hypertext, previous_core_developers)
	table.insert_all(hypertext, {
		"\n",
		"<heading>", fgettext_ne("Previous Contributors"), "</heading>\n",
	})
	prepare_credits(hypertext, previous_contributors)

	hypertext = table.concat(hypertext):sub(1, -2)

	local fs = "size[17.5,8.1,true]" .."image[1.0,0.1;4,4;" .. core.formspec_escape(logofile) .. "]" ..
		"style[label_button;border=false]" ..
		"button[0.1,3.4;5.3,0.5;label_button;" ..
		core.formspec_escape(version.project .. " " .. version.string) .. "]" ..
		"button_url[1.5,4.1;2.5,0.8;homepage;minetest.net;https://www.minetest.net/]" ..
		"hypertext[5.5,0.25;12.5,9;credits;" .. minetest.formspec_escape(hypertext) .. "]" ..
		"button[0.5,7.1;4.5,0.8;back;" .. fgettext("Back") .. "]"

	-- Render information
	local active_renderer_info = fgettext("Active renderer:") .. " " ..
		core.formspec_escape(core.get_active_renderer())
	fs = fs .. "style[label_button2;border=false]" ..
		"button[0.1,6;5.3,0.5;label_button2;" .. active_renderer_info .. "]"..
		"tooltip[label_button2;" .. active_renderer_info .. "]"

	-- Irrlicht device information
	local irrlicht_device_info = fgettext("Irrlicht device:") .. " " ..
		core.formspec_escape(core.get_active_irrlicht_device())
	fs = fs .. "style[label_button3;border=false]" ..
		"button[0.1,6.5;5.3,0.5;label_button3;" .. irrlicht_device_info .. "]"..
		"tooltip[label_button3;" .. irrlicht_device_info .. "]"

	if PLATFORM == "Android" then
		fs = fs .. "button[0.5,5.1;4.5,0.8;share_debug;" .. fgettext("Share debug log") .. "]"
	else
		fs = fs .. "tooltip[userdata;" ..
				fgettext("Opens the directory that contains user-provided worlds, games, mods,\n" ..
						"and texture packs in a file manager / explorer.") .. "]"
		fs = fs .. "button[0.5,5.1;4.5,0.8;userdata;" .. fgettext("Open User Data Directory") .. "]"
	end

	return fs
end

about_button_handler = function(this, fields, name, tabdata)
	if fields.share_debug then
		local path = core.get_user_path() .. DIR_DELIM .. "debug.txt"
		core.share_file(path)
	end

	if fields.userdata then
		core.open_dir(core.get_user_path())
	end

	if fields.back then
		this:delete()
		return true
	end

end

function create_about_dlg()
	mm_game_theme.set_engine()
	local retval = dialog_create("about",
					get_formspec,
					about_button_handler,
					nil)
	return retval
end
