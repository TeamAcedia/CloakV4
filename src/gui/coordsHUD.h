#pragma once

#include "gui/cheatUIElement.h"
#include <iostream>
#include <chrono>
#include <locale> 
#include <codecvt> 
#include "settings.h"
#include "client/client.h"
#include "client/localplayer.h"
#include <sstream>
#include <iomanip> 

class coordsHUD : public CheatUIElement {
public:
	coordsHUD(const core::rect<s32>& rect);
    void draw(video::IVideoDriver* driver, gui::IGUIFont* font, float dtime, ClientEnvironment &env, bool editing) override;
};
