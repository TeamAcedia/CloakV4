// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#include <iostream>
#include <IGUIEnvironment.h>
#include "core.h"
#include "plain.h"
#include "client/shadows/dynamicshadowsrender.h"
#include "settings.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/content_cao.h"
#include "client/clientmap.h"
#include "client/hud.h"
#include "nodedef.h"
#include "client/mapblock_mesh.h"	
#include "script/scripting_client.h"

u16 RenderingCore::combat_target = 0;
video::SColor RenderingCore::target_esp_color = video::SColor(255, 255, 0, 0);
std::map<std::string, std::vector<int>> RenderingCore::ESPplayersNames = {};

RenderingCore::RenderingCore(IrrlichtDevice *_device, Client *_client, Hud *_hud,
		std::unique_ptr<ShadowRenderer> _shadow_renderer,
		std::unique_ptr<RenderPipeline> _pipeline,
		v2f _virtual_size_scale)
	: device(_device), client(_client), hud(_hud), shadow_renderer(std::move(_shadow_renderer)),
	pipeline(std::move(_pipeline)), virtual_size_scale(_virtual_size_scale)
{
}

RenderingCore::~RenderingCore() = default;

v2u32 RenderingCore::getVirtualSize() const
{
	return virtual_size;
}

void RenderingCore::draw(video::SColor _skycolor, bool _show_hud,
		bool _draw_wield_tool, bool _draw_crosshair, float dtime)
{
	v2u32 screensize = device->getVideoDriver()->getScreenSize();
	virtual_size = v2u32(screensize.X * virtual_size_scale.X, screensize.Y * virtual_size_scale.Y);

	PipelineContext context(device, client, hud, shadow_renderer, _skycolor, screensize);
	context.dtime = dtime;
	context.draw_crosshair = _draw_crosshair;
	context.draw_wield_tool = _draw_wield_tool;
	context.show_hud = _show_hud;
	pipeline->reset(context);
	pipeline->run(context);
}

void RenderingCore::drawCombatTargetHUD()
{
    auto driver = device->getVideoDriver();
    ClientEnvironment &env = client->getEnv();

    std::unordered_map<u16, ClientActiveObject*> allObjects;
    env.getAllActiveObjectsLegacy(allObjects);

    for (auto &ao_it : allObjects) {
        ClientActiveObject *cao = ao_it.second;
        GenericCAO *obj = dynamic_cast<GenericCAO *>(cao);

        if (!obj || combat_target == NULL || obj->getId() != combat_target)
            continue;

        // Get screen size
        core::dimension2d<u32> screen_size = driver->getScreenSize();
		double health_percentage = obj->getProperties().hp_max > 0 ? static_cast<double>(obj->getHp()) / obj->getProperties().hp_max : 0.0;
		health_percentage = std::max(0.0, std::min(1.0, health_percentage));

		video::SColor backgroundColor(255, 5, 10, 15);
		video::SColor borderColor(255, 0, 0, 0);

		// Health-based color interpolation
		u8 red = static_cast<u8>(255 * (1.0f - health_percentage));
		u8 green = static_cast<u8>(255 * health_percentage);
		video::SColor filledColor(255, red, green, 0);

		f32 scale = 1; 

		// Bar dimensions
		s32 barWidth = static_cast<s32>(150 * scale);
		s32 barHeight = static_cast<s32>(10 * scale);
		s32 baseBarOffset = static_cast<s32>(60 * scale);

		// Calculate screen position (centered, then offset slightly to the right)
		v2s32 screen_pos;
		screen_pos.X = screen_size.Width / 2;
		screen_pos.Y = screen_size.Height / 2;

		// Define health bar rectangle
		core::rect<s32> barRect(baseBarOffset, 0, baseBarOffset + barWidth, barHeight);
		core::rect<s32> filledRect(
			barRect.UpperLeftCorner.X,
			barRect.UpperLeftCorner.Y,                      
			barRect.UpperLeftCorner.X + static_cast<s32>(barRect.getWidth() * health_percentage),
			barRect.LowerRightCorner.Y                     
		);
		

		// Draw health bar
		driver->draw2DRectangle(backgroundColor, barRect + screen_pos);
		driver->draw2DRectangle(filledColor, filledRect + screen_pos);
		driver->draw2DRectangleOutline(barRect + screen_pos, borderColor, barHeight * 0.2);
    }
}

