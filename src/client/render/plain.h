// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#pragma once
#include "core.h"
#include "pipeline.h"
#include "map.h"
#include "mapnode.h"

/**
 * Implements a pipeline step that renders the 3D scene
 */
class Draw3D : public RenderStep
{
public:
	virtual void setRenderSource(RenderSource *) override {}
	virtual void setRenderTarget(RenderTarget *target) override { m_target = target; }

	virtual void reset(PipelineContext &context) override {}
	virtual void run(PipelineContext &context) override;

private:
	RenderTarget *m_target {nullptr};
};

class DrawWield : public RenderStep
{
public:
	virtual void setRenderSource(RenderSource *) override {}
	virtual void setRenderTarget(RenderTarget *target) override { m_target = target; }

	virtual void reset(PipelineContext &context) override {}
	virtual void run(PipelineContext &context) override;

private:
	RenderTarget *m_target {nullptr};
};

/**
 * Implements a pipeline step that renders the game HUD
 */
class DrawHUD : public RenderStep
{
public:
	virtual void setRenderSource(RenderSource *) override {}
	virtual void setRenderTarget(RenderTarget *) override {}

	virtual void reset(PipelineContext &context) override {}
	virtual void run(PipelineContext &context) override;
};

/**
 * Implements a pipeline step that renders Tracers and ESP
*/

class DrawTracersAndESP : public RenderStep
{
public:
	virtual void setRenderSource(RenderSource *) override {}
	virtual void setRenderTarget(RenderTarget *) override {}

	virtual void reset(PipelineContext &context) override {}
	virtual void run(PipelineContext &context) override;

private:
	u8 getDifferentNeighborFlags(v3s16 p, Map &map, const MapNode &node);

	static inline const v3s16 directions[6] = {
		v3s16(0, 0, -1),  // Front
		v3s16(0, 0, 1), // Back
		v3s16(-1, 0, 0), // Left
		v3s16(1, 0, 0),  // Right
		v3s16(0, 1, 0),  // Top
		v3s16(0, -1, 0)  // Bottom
	};

	bool draw_entity_esp;
	bool draw_entity_tracers;
	bool draw_player_esp;
	bool draw_player_tracers;
	bool draw_node_esp;
	bool draw_node_tracers;

	video::SColor entity_esp_color;
	video::SColor player_esp_color;
	video::SColor self_esp_color;
	
	int playerDT;
	int playerEO;
	int playerFO;
	int entityDT;
	int entityEO;
	int entityFO;
	int nodeDT;
	int nodeEO;
	int nodeFO;
};

class MapPostFxStep : public TrivialRenderStep
{
public:
	virtual void setRenderTarget(RenderTarget *) override;
	virtual void run(PipelineContext &context) override;
private:
	RenderTarget *target;
};

class RenderShadowMapStep : public TrivialRenderStep
{
public:
	virtual void run(PipelineContext &context) override;
};

/**
 * UpscaleStep step performs rescaling of the image
 * in the source texture 0 to the size of the target.
 */
class UpscaleStep : public RenderStep
{
public:

	virtual void setRenderSource(RenderSource *source) override { m_source = source; }
	virtual void setRenderTarget(RenderTarget *target) override { m_target = target; }
	virtual void reset(PipelineContext &context) override {};
	virtual void run(PipelineContext &context) override;
private:
	RenderSource *m_source;
	RenderTarget *m_target;
};

std::unique_ptr<RenderStep> create3DStage(Client *client, v2f scale);
RenderStep* addUpscaling(RenderPipeline *pipeline, RenderStep *previousStep, v2f downscale_factor, Client *client);

void populatePlainPipeline(RenderPipeline *pipeline, Client *client);

video::ECOLOR_FORMAT selectColorFormat(video::IVideoDriver *driver);
video::ECOLOR_FORMAT selectDepthFormat(video::IVideoDriver *driver);
