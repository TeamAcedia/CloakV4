/*
Dragonfire
Copyright (C) 2020 Elias Fleckenstein <eliasfleckenstein@web.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <iostream>

#include "script/scripting_client.h"
#include "client/client.h"
#include "client/fontengine.h"
#include "cheatMenu.h"
#include "client/minimap.h"
#include "settings.h"
#include <cstddef>


FontMode CheatMenu::fontStringToEnum(std::string str)
{
	if (str == "FM_Standard")
		return FM_Standard;
	else if (str == "FM_HD")
		return FM_HD;
	else if (str == "FM_Mono")
		return FM_Mono;
	else if (str == "FM_Fallback")
		return _FM_Fallback;
	else if (str == "FM_MaxMode")
		return FM_MaxMode;
	else if (str == "FM_Unspecified")
		return FM_Unspecified;
	else
		return FM_Standard;
}

CheatMenu::CheatMenu(Client *client) : m_client(client)
{
	FontMode fontMode = fontStringToEnum(g_settings->get("cheat_menu_font"));
	v3f bg_color, active_bg_color, font_color, selected_font_color;

	bg_color = g_settings->getV3F("cheat_menu_bg_color").value_or(v3f(10, 15, 20));
    active_bg_color = g_settings->getV3F("cheat_menu_active_bg_color").value_or(v3f(50, 80, 175));
    font_color = g_settings->getV3F("cheat_menu_font_color").value_or(v3f(255, 255, 255));
    selected_font_color = g_settings->getV3F("cheat_menu_selected_font_color").value_or(v3f(0, 0, 0));


	m_bg_color = video::SColor(g_settings->getU32("cheat_menu_bg_color_alpha"),
			bg_color.X, bg_color.Y, bg_color.Z);

	m_active_bg_color = video::SColor(
			g_settings->getU32("cheat_menu_active_bg_color_alpha"),
			active_bg_color.X, active_bg_color.Y, active_bg_color.Z);

	m_font_color = video::SColor(g_settings->getU32("cheat_menu_font_color_alpha"),
			font_color.X, font_color.Y, font_color.Z);

	m_selected_font_color = video::SColor(
			g_settings->getU32("cheat_menu_selected_font_color_alpha"),
			selected_font_color.X, selected_font_color.Y,
			selected_font_color.Z);

	m_head_height = g_settings->getU32("cheat_menu_head_height");
	m_entry_height = g_settings->getU32("cheat_menu_entry_height");
	m_entry_width = g_settings->getU32("cheat_menu_entry_width");

	FontSpec fontSpec(g_fontengine->getDefaultFontSize(), fontMode, true /* bold */, false /* italic */);
	m_font = g_fontengine->getFont(fontSpec);

	if (!m_font) {
		errorstream << "CheatMenu: Unable to load font" << std::endl;
	} else {
		core::dimension2d<u32> dim = m_font->getDimension(L"M");
		m_fontsize = v2u32(dim.Width, dim.Height);
		m_font->grab();
	}
	m_fontsize.X = MYMAX(m_fontsize.X, 1);
	m_fontsize.Y = MYMAX(m_fontsize.Y, 1);
}

void CheatMenu::draw2DRectangleOutline(video::IVideoDriver *driver, const core::recti& pos, video::SColor color) {
	driver->draw2DLine(pos.UpperLeftCorner, core::position2di(pos.LowerRightCorner.X, pos.UpperLeftCorner.Y), color);
	driver->draw2DLine(core::position2di(pos.LowerRightCorner.X, pos.UpperLeftCorner.Y), pos.LowerRightCorner, color);
	driver->draw2DLine(pos.LowerRightCorner, core::position2di(pos.UpperLeftCorner.X, pos.LowerRightCorner.Y), color);
	driver->draw2DLine(core::position2di(pos.UpperLeftCorner.X, pos.LowerRightCorner.Y), pos.UpperLeftCorner, color);
}

void CheatMenu::drawRect(video::IVideoDriver *driver, std::string name,
		int x, int y,
		int width, int height,
		bool active, bool selected)
{
		video::SColor *bgcolor = &m_bg_color,
								  *fontcolor = &m_font_color;

		if (active)
				bgcolor = &m_active_bg_color;
		if (selected)
				fontcolor = &m_selected_font_color;

		driver->draw2DRectangle(*bgcolor, core::rect<s32>(x, y, x + width, y + height));

		if (selected)
				driver->draw2DRectangleOutline(
					core::rect<s32>(x - 1, y - 1, x + width, y + height),
					*fontcolor);

		int fx = x + 5,
				fy = y + (height - m_fontsize.Y) / 2;

		core::rect<s32> fontbounds(
						fx, fy, fx + m_fontsize.X * name.size(), fy + m_fontsize.Y);
		m_font->draw(name.c_str(), fontbounds, *fontcolor, false, false);
}

void CheatMenu::drawEntry(video::IVideoDriver *driver, std::string name, int number,
		bool selected, bool active, CheatMenuEntryType entry_type)
{
	int x = m_gap+5, y = m_gap+5, width = m_entry_width, height = m_entry_height;
	video::SColor *bgcolor = &m_bg_color, *fontcolor = &m_font_color;
	if (entry_type == CHEAT_MENU_ENTRY_TYPE_HEAD) {
		bgcolor = &m_active_bg_color;
		height = m_head_height;
	} else {
		bool is_category = entry_type == CHEAT_MENU_ENTRY_TYPE_CATEGORY;
		y += m_head_height +	
			 (number + (is_category ? 0 : m_selected_category)) *
					 (m_entry_height);
		x += (is_category ? 0 : m_gap + m_entry_width);
		if (active)
			bgcolor = &m_active_bg_color;
		if (selected)
			fontcolor = &m_selected_font_color;
        if (selected && is_category)
            bgcolor = &m_active_bg_color;
	}
	driver->draw2DRectangle(*bgcolor, core::rect<s32>(x, y, x + width, y + height));
	if (selected)
		driver->draw2DRectangleOutline(
			core::rect<s32>(x, y, x, y),
			*fontcolor);
	int fx = x + 5, fy = y + (height - m_fontsize.Y) / 2;
	core::rect<s32> fontbounds(
			fx, fy, fx + m_fontsize.X * name.size(), fy + m_fontsize.Y);
	m_font->draw(name.c_str(), fontbounds, *fontcolor, false, false);
}

int negmod(int n, int base)
{
	n = n % base;
	return (n < 0) ? base + n : n;
}

void CheatMenu::draw(video::IVideoDriver *driver, bool show_debug)
{
    CHEAT_MENU_GET_SCRIPTPTR

    m_head_height = (!show_debug ? 1 : g_settings->getU32("cheat_menu_head_height"));
    int category_count = 0;

    // Calculate dimensions for the category section outline
    int category_section_x = m_gap+5; // Starting X position
    int category_section_y = m_gap+5+m_head_height; // Starting Y position
    int category_section_width = m_entry_width; // Width of categories
    int category_section_height = ((m_head_height + (m_entry_height * script->m_cheat_categories.size()))-m_head_height)-m_entry_height; // Total height based on number of categories

    // Define padding for the outline and thickness
    const int padding = 0; // Space between outline and categories
    const int outline_thickness = 3; // Thickness of the outline

    // Draw multiple rectangles to create a thicker outline
    for (int i = 0; i < outline_thickness; ++i) {
        driver->draw2DRectangleOutline(
            core::rect<s32>(
                category_section_x - padding - i, 
                category_section_y - padding - i,
                category_section_x + category_section_width + padding + i,
                category_section_y + category_section_height + padding + i),
            m_active_bg_color); // Use selected font color for outline
    }

    for (auto category = script->m_cheat_categories.begin();
             category != script->m_cheat_categories.end(); category++) {
		if ((*category)->m_name == "Client")
		continue;
        bool is_selected = category_count == m_selected_category;
        drawEntry(driver, (*category)->m_name, category_count, is_selected, false,
                    CHEAT_MENU_ENTRY_TYPE_CATEGORY);
        
        if (is_selected && m_cheat_layer) {
            int cheat_n = (*category)->m_cheats.size();
            int height = driver->getScreenSize().Height;
            int target = height / (m_entry_height) + 1; // +1 for "and more" effect
            int target_normal =
                (height - (m_selected_category * (m_entry_height)))
                / (m_entry_height);

            if (cheat_n < target_normal) {
                int cheat_count = 0;
                for (auto cheat = (*category)->m_cheats.begin();
                             cheat != (*category)->m_cheats.end(); cheat++) {
                    drawEntry(driver, (*cheat)->m_name, cheat_count,
                                    cheat_count == m_selected_cheat,
                                    (*cheat)->is_enabled());
                    cheat_count++;
                }
            } else {
                int base = m_selected_cheat - m_selected_category - 1;
                int drawn = 0;
                for (int i = base; i < base + target; i++, drawn++) {
                    int idx = negmod(i, cheat_n);
                    ScriptApiCheatsCheat *cheat = (*category)->m_cheats[idx];
                    int y = (drawn * (m_entry_height));
                    drawRect(driver, cheat->m_name,
                                    m_gap * 2 + m_entry_width, y,
                                    m_entry_width, m_entry_height,
                                    cheat->is_enabled(),
                                    idx == m_selected_cheat);
                }
            }
        }
        category_count++;
    }
}




void CheatMenu::drawHUD(video::IVideoDriver *driver, double dtime)
{
	CHEAT_MENU_GET_SCRIPTPTR

	m_rainbow_offset += dtime;

	m_rainbow_offset = fmod(m_rainbow_offset, 6.0f);

	std::vector<std::pair<std::string, core::dimension2d<u32>>> enabled_cheats;

	int cheat_count = 0;

	for (auto category = script->m_cheat_categories.begin();
			category != script->m_cheat_categories.end(); category++) {
		for (auto cheat = (*category)->m_cheats.begin();
				cheat != (*category)->m_cheats.end(); cheat++) {
			if ((*cheat)->is_enabled()) {
				std::string cheat_str = (*cheat)->m_name;
				std::string info_text = (*cheat)->get_info_text();
				if (!info_text.empty()) {
					cheat_str += " [" + info_text + "]";
				}
				core::dimension2d<u32> dim =
							m_font->getDimension(utf8_to_wide(cheat_str).c_str());
				enabled_cheats.push_back(std::make_pair(cheat_str, dim));
				cheat_count++;
			}
		}
	}

	if (enabled_cheats.empty())
		return;

	core::dimension2d<u32> screensize = driver->getScreenSize();
	u32 y = 5 + g_settings->getS32("cheat_hud.offset");
	
	std::sort(enabled_cheats.begin(), enabled_cheats.end(),
			  [](const auto &a, const auto &b) {
				  return a.second.Width > b.second.Width;
			  });

	Minimap *mapper = m_client->getMinimap();

	bool renderBottom = (mapper != nullptr && mapper->getModeIndex() != 0) || g_settings->get("cheat_hud.position") == "Bottom";

	if (renderBottom) {
		y = (screensize.Height - 18) - g_settings->getS32("cheat_hud.offset");
	}

	std::vector<video::SColor> colors;

	for (int i = 0; i < cheat_count; i++) {
		video::SColor color = video::SColor(255, 0, 0, 0);
		f32 h = (f32)i * 2.0f / (f32)cheat_count - m_rainbow_offset;
		if (h < 0)
			h = 6.0f + h;
		f32 x = (1 - fabs(fmod(h, 2.0f) - 1.0f)) * 255.0f;
		switch ((int)h) {
		case 0:
			color = video::SColor(255, 255, x, 0);
			break;
		case 1:
			color = video::SColor(255, x, 255, 0);
			break;
		case 2:
			color = video::SColor(255, 0, 255, x);
			break;
		case 3:
			color = video::SColor(255, 0, x, 255);
			break;
		case 4:
			color = video::SColor(255, x, 0, 255);
			break;
		case 5:
			color = video::SColor(255, 255, 0, x);
			break;
		}
		colors.push_back(color);
	}

	int i = 0;
	video::SColor infoColor(230, 230, 230, 230);
	for (std::pair<std::string, core::dimension2d<u32>> &cheat : enabled_cheats) {
		std::string cheat_full_str = cheat.first;
		core::dimension2d<u32> dim = cheat.second;

		size_t brace_position = cheat_full_str.find('[');
		if (brace_position != std::string::npos) {
			std::string cheat_str = cheat_full_str.substr(0, brace_position);
			std::string info_str = cheat_full_str.substr(brace_position);

			core::dimension2d<u32> cheat_dim = m_font->getDimension(utf8_to_wide(cheat_str).c_str());
			core::dimension2d<u32> info_dim = m_font->getDimension(utf8_to_wide(info_str).c_str());

			u32 x_cheat = screensize.Width - 5 - dim.Width;
			u32 x_info = x_cheat + cheat_dim.Width;

			core::rect<s32> cheat_bounds(x_cheat, y, x_cheat + cheat_dim.Width, y + cheat_dim.Height);
			m_font->draw(cheat_str.c_str(), cheat_bounds, colors[i], false, false);

			core::rect<s32> info_bounds(x_info, y, x_info + info_dim.Width, y + info_dim.Height);
			m_font->draw(info_str.c_str(), info_bounds, infoColor, false, false);

		} else {
			u32 x = screensize.Width - 5 - dim.Width;

			core::rect<s32> cheat_bounds(x, y, x + dim.Width, y + dim.Height);
			m_font->draw(cheat_full_str.c_str(), cheat_bounds, colors[i], false, false);
		}

		if (renderBottom) {
			y -= dim.Height;
		} else {
			y += dim.Height;
		}

		i++;
	}
}

void CheatMenu::selectUp()
{
    CHEAT_MENU_GET_SCRIPTPTR

    int max = (m_cheat_layer ? script->m_cheat_categories[m_selected_category]->m_cheats.size()
                             : script->m_cheat_categories.size()) - 1;
    int *selected = m_cheat_layer ? &m_selected_cheat : &m_selected_category;
    do {
        --*selected;
        if (*selected < 0)
            *selected = max;
    } while (!m_cheat_layer && script->m_cheat_categories[*selected]->m_name == "Client");
}


void CheatMenu::selectDown()
{
    CHEAT_MENU_GET_SCRIPTPTR

    int max = (m_cheat_layer ? script->m_cheat_categories[m_selected_category]->m_cheats.size()
                             : script->m_cheat_categories.size()) - 1;
    int *selected = m_cheat_layer ? &m_selected_cheat : &m_selected_category;
    do {
        ++*selected;
        if (*selected > max)
            *selected = 0;
    } while (!m_cheat_layer && script->m_cheat_categories[*selected]->m_name == "Client");
}


void CheatMenu::selectRight()
{
	if (m_cheat_layer)
		return;
	m_cheat_layer = true;
	m_selected_cheat = 0;
}

void CheatMenu::selectLeft()
{
	if (!m_cheat_layer)
		return;
	m_cheat_layer = false;
}

void CheatMenu::selectConfirm()
{
	CHEAT_MENU_GET_SCRIPTPTR

	if (m_cheat_layer)
		script->toggle_cheat(script->m_cheat_categories[m_selected_category]
							 ->m_cheats[m_selected_cheat]);
}