/*
Cloak V4
Copyright (C) 2025 Maintainer_(Ivan Shkatov) <ivanskatov672@gmail.com>
Copyright (C) 2025 Prounce <prouncedev@gmail.com>
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
#ifndef NEWMENU_H
#define NEWMENU_H

#include <irrlicht.h>
#include "settings.h"
#include <iostream>
#include "gui/modalMenu.h"
#include <vector>
#include "script/scripting_client.h"
#include "client/client.h"
#include "client/fontengine.h"
#include <codecvt> 
#include <locale> 
#include "log.h"
#include <IGUIEditBox.h>
#include <IVideoDriver.h>
#include <IGUIFont.h>
#include <chrono>

struct Sprite
{
    int x = 0, y = 0;
    int width = 0;
    int height = 0;

    core::rect<s32> get_rect() {return core::rect<s32>(x, y, x + width, y + height);}
    bool isDragging = false;

    bool changedPos = false;

    void save(s32 screenWidth, s32 screenHeight) {
        if (x < 0) {
            x = 0;
        } else if (y + height > screenHeight) {
            y = screenHeight - height;
        }

        g_settings->setV2F("coords_sprite", v2f(x, y));
    }
};

using namespace irr;
using namespace gui;

#define GET_SCRIPT_POINTER                                                   \
    ClientScripting *script = m_client->getScript();                         \
    if (!script || !script->m_cheats_loaded)                                 \
        return;

#define GET_SCRIPT_POINTER_S32                                               \
    ClientScripting *script = m_client->getScript();                         \
    if (!script || !script->m_cheats_loaded)                                 \
        return s32(0);

#define GET_SCRIPT_POINTER_BOOL                                              \
    ClientScripting *script = m_client->getScript();                         \
    if (!script || !script->m_cheats_loaded)                                 \
        return true;     

class NewMenu: public IGUIElement
{
public:
    NewMenu(gui::IGUIEnvironment* env, gui::IGUIElement* parent, s32 id, IMenuManager* menumgr, Client *client);

    void create();
    void close();

    virtual bool OnEvent(const irr::SEvent& event);
    virtual void draw() override;
    void drawHints(video::IVideoDriver* driver, gui::IGUIFont* font, const size_t i);
    void drawCategory(video::IVideoDriver* driver, gui::IGUIFont* font, const size_t category_index, float dtime);
    void drawSelectionBox(video::IVideoDriver* driver, gui::IGUIFont* font, const size_t i, const size_t c, const size_t s);
    bool isOpen() { return m_is_open; }
    
    ~NewMenu();
    static Sprite coords_sprite;

private:
    double roundToNearestStep(double number, double m_min, double m_max, double m_steps);
    void calculateSliderSplit(const core::rect<s32>& sliderRect, double value, double minValue, double maxValue, core::rect<s32>& filledRect, core::rect<s32>& remainingRect);
    double calculateSliderValueFromPosition(const core::rect<s32>& sliderBarRect, const core::position2d<s32>& pointerPosition, double m_min, double m_max, double m_steps);
    void drawInterpolatedRectangle(video::IVideoDriver* driver, const core::rect<s32>& rect, video::SColor innerColor, video::SColor outerColor, float interpolation);
    
    core::vector2d<s32> offset; 
    IMenuManager* m_menumgr; 
    bool isDragging = false;
    bool isSliding = false;
    bool isSelecting = false;
    bool m_initialized = false;
    const s32 category_height = 34;
    const s32 category_width = category_height * 5;
    const s32 setting_width = category_height * 4.6;
    const s32 setting_bar_width = category_height * 0.1;
    const s32 setting_bar_padding = category_height * 0.1;
    const s32 slider_height_padding = category_height * 0.4;
    const s32 slider_width_padding = category_height * 0.3;
    const s32 selection_box_padding = category_height * 0.1;
    bool m_is_open = false; 
    int draggedRectIndex = 0;
    int draggedSliderCategoryIndex = 0;
    int draggedSliderCheatIndex = 0;
    int draggedSliderSettingIndex = 0;
    int selectingCategoryIndex = 0;
    int selectingCheatIndex = 0;
    int selectingSettingIndex = 0;
    std::vector<bool> selectedCategory;
    std::vector<std::vector<bool>> selectedCheat;
    std::vector<bool> dropdownHovered;
    std::vector<bool> textHovered;
    std::vector<std::vector<bool>> cheatDropdownHovered;
    std::vector<std::vector<bool>> cheatTextHovered;
    std::vector<std::vector<float>> cheatRectAnimationProgress;
    std::vector<std::vector<std::vector<bool>>> cheatSettingTextHovered;
    std::vector<std::vector<std::vector<bool>>> cheatSliderHovered;
    std::vector<std::vector<std::vector<bool>>> selectionBoxHovered;
    std::vector<core::rect<s32>> categoryRects;
    std::vector<core::rect<s32>> dropdownRects;
    std::vector<core::rect<s32>> textRects;
    std::vector<std::vector<core::rect<s32>>> cheatRects;
    std::vector<std::vector<std::vector<core::rect<s32>>>> cheatSettingRects;
    std::vector<std::vector<std::vector<core::rect<s32>>>> cheatSliderRects;
    std::vector<std::vector<std::vector<IGUIEditBox*>>> cheatSettingTextFields;
    std::vector<std::vector<std::vector<std::wstring>>> cheatSettingTextLasts;
    std::vector<std::vector<std::vector<core::rect<s32>>>> cheatSliderBarRects;
    std::vector<std::vector<std::vector<core::rect<s32>>>> cheatSettingTextRects;
    std::vector<std::vector<std::vector<core::rect<s32>>>> selectionBoxRects;
    std::vector<std::vector<core::rect<s32>>> cheatTextRects;
    std::vector<std::vector<core::rect<s32>>> cheatDropdownRects;
    std::vector<std::vector<std::vector<core::position2d<s32>>>> cheat_setting_positions;
    std::vector<core::position2d<s32>> category_positions;
    std::vector<std::vector<core::position2d<s32>>> cheat_positions;
    std::vector<std::vector<std::vector<std::vector<bool>>>> cheatSettingOptionHovered;
    std::vector<std::vector<std::vector<std::vector<core::rect<s32>>>>> cheatSettingOptionRects;
    core::dimension2d<u32> lastScreenSize;
    s32 respaceMenu(size_t i);
    void moveMenu(size_t i, core::position2d<s32> new_position);
    Client* m_client;

    gui::IGUIEnvironment* env;
    video::SColor settingBackgroundColor = video::SColor(230, 1, 2, 0);
    video::SColor settingBarColor = video::SColor(255, 80, 120, 220);
    video::SColor sliderColor = video::SColor(255, 200, 200, 200);
    video::SColor sliderBarColor = video::SColor(255, 255, 255, 255);
    video::SColor sliderColorActive = video::SColor(255, 125, 125, 125);
    video::SColor sliderBarColorActive = video::SColor(255, 66, 111, 195);
    video::SColor option_color = video::SColor(255, 2, 5, 8);
    
    s32 subCategoryHeight = category_height * 3;

    static float getDeltaTime();

    static std::chrono::high_resolution_clock::time_point lastTime;
};

#endif