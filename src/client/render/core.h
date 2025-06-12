// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#pragma once

#include "irr_v2d.h"
#include <SColor.h>
#include <memory>
#include "pipeline.h"

class IrrlichtDevice;

class ShadowRenderer;
class Client;
class Hud;
class RenderPipeline;

class RenderingCore
{
protected:
	IrrlichtDevice *device;
	Client *client;
	Hud *hud;
	std::unique_ptr<ShadowRenderer> shadow_renderer;

	std::unique_ptr<RenderPipeline> pipeline;

	v2f virtual_size_scale;
	v2u32 virtual_size { 0, 0 };

	bool draw_entity_esp;
	bool draw_entity_tracers;
	bool draw_player_esp;
	bool draw_player_tracers;
	bool draw_node_esp;
	bool draw_node_tracers;
	video::SColor entity_esp_color;
	video::SColor player_esp_color;
	video::SColor self_esp_color;

public:
	RenderingCore(IrrlichtDevice *device, Client *client, Hud *hud,
			std::unique_ptr<ShadowRenderer> shadow_renderer,
			std::unique_ptr<RenderPipeline> pipeline,
			v2f virtual_size_scale);
	RenderingCore(const RenderingCore &) = delete;
	RenderingCore(RenderingCore &&) = delete;
	virtual ~RenderingCore();

	RenderingCore &operator=(const RenderingCore &) = delete;
	RenderingCore &operator=(RenderingCore &&) = delete;

	void draw(video::SColor _skycolor, bool _show_hud,
			bool _draw_wield_tool, bool _draw_crosshair);
	void draw_HUD(video::SColor _skycolor, bool _show_hud,
			bool _draw_wield_tool, bool _draw_crosshair);
	void drawTracersAndESP();
	void Draw3D(PipelineContext &context);
	void DrawWield(PipelineContext &context);
	void DrawHUD(PipelineContext &context);
	void MapPostFxStep(PipelineContext &context);
	void RenderShadowMapStep(PipelineContext &context);
	v2u32 getVirtualSize() const;

	ShadowRenderer *get_shadow_renderer() { return shadow_renderer.get(); };
};
