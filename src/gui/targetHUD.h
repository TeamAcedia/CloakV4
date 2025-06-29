#pragma once

#include "gui/cheatUIElement.h"
#include <iostream>
#include <chrono>
#include <locale> 
#include <codecvt> 
#include "client/content_cao.h"
#include "settings.h"
#include "client/clientenvironment.h"
#include "client/render/core.h"
#include "client/client.h"

class TargetHUD : public CheatUIElement {
public:
	TargetHUD(const core::rect<s32>& rect);
    void draw(video::IVideoDriver* driver, gui::IGUIFont* font, float dtime, ClientEnvironment &env, bool editing) override;
private:
	
	std::unordered_map<u16, double> m_interpolated_entity_health;

	double getInterpolatedHealth(const GenericCAO *obj, float dtime);
};
