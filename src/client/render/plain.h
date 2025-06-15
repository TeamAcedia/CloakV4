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

// Task node
struct TaskNode {
	v3f position;
	video::SColor color = video::SColor(255, 255, 255, 255);
};

// Tracer line
struct TaskTracer {
	v3f start;
	v3f end;
	video::SColor color = video::SColor(255, 255, 255, 255);

	bool operator==(const TaskTracer &other) const {
		return start == other.start && end == other.end;
	}
};

class DrawTaskBlocksAndTracers : public RenderStep
{
public:
	virtual void setRenderSource(RenderSource *) override {}
	virtual void setRenderTarget(RenderTarget *) override {}

	virtual void reset(PipelineContext &context) override {}
	virtual void run(PipelineContext &context) override;

	static void addTaskNode(const TaskNode &node) {
		task_nodes.push_back(node);
	}
	static void addTaskTracer(const TaskTracer &tracer) {
		task_tracers.push_back(tracer);
	}

	static void removeTaskNode(const TaskNode &node) {
		task_nodes.erase(std::remove_if(task_nodes.begin(), task_nodes.end(), [&node](const TaskNode &n) {
			return n.position == node.position;
		}), task_nodes.end());
	}

	static void removeTaskTracer(const TaskTracer &tracer) {
		task_tracers.erase(std::remove_if(task_tracers.begin(), task_tracers.end(), [&tracer](const TaskTracer &t) {
			return t.start == tracer.start && t.end == tracer.end;
		}), task_tracers.end());
	}
private:
	bool draw_task_blocks;
	bool draw_task_tracers;

	u32 last_time = 0;

	static std::vector<TaskNode> task_nodes;
	static std::vector<TaskTracer> task_tracers;
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
