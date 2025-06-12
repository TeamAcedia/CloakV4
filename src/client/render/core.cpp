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


RenderingCore::RenderingCore(IrrlichtDevice *_device, Client *_client, Hud *_hud,
		std::unique_ptr<ShadowRenderer> _shadow_renderer,
		std::unique_ptr<RenderPipeline> _pipeline,
		v2f _virtual_size_scale)
	: device(_device), client(_client), hud(_hud), shadow_renderer(std::move(_shadow_renderer)),
	pipeline(std::move(_pipeline)), virtual_size_scale(_virtual_size_scale)
{
}

RenderingCore::~RenderingCore() = default;

void RenderingCore::draw(video::SColor _skycolor, bool _show_hud,
		bool _draw_wield_tool, bool _draw_crosshair)
{
	v2u32 screensize = device->getVideoDriver()->getScreenSize();
	virtual_size = v2u32(screensize.X * virtual_size_scale.X, screensize.Y * virtual_size_scale.Y);

	PipelineContext context(device, client, hud, shadow_renderer.get(), _skycolor, screensize);
	context.draw_crosshair = _draw_crosshair;
	context.draw_wield_tool = _draw_wield_tool;
	context.show_hud = _show_hud;

	pipeline->reset(context);
	//pipeline->run(context);
	Draw3D(context);
	drawTracersAndESP();
	DrawWield(context);
	DrawHUD(context);
	//MapPostFxStep(context);
	//RenderShadowMapStep(context);
}

void RenderingCore::draw_HUD(video::SColor _skycolor, bool _show_hud,
	bool _draw_wield_tool, bool _draw_crosshair) {
v2u32 screensize = device->getVideoDriver()->getScreenSize();
virtual_size = v2u32(screensize.X * virtual_size_scale.X, screensize.Y * virtual_size_scale.Y);

PipelineContext context(device, client, hud, shadow_renderer, _skycolor, screensize);
context.draw_crosshair = _draw_crosshair;
context.draw_wield_tool = _draw_wield_tool;
context.show_hud = _show_hud;

DrawHUD(context);
MapPostFxStep(context);
}

v2u32 RenderingCore::getVirtualSize() const
{
	return virtual_size;
}

void RenderingCore::drawTracersAndESP()
{
	auto driver = device->getVideoDriver();

	draw_entity_esp = g_settings->getBool("enable_entity_esp");
	draw_entity_tracers = g_settings->getBool("enable_entity_tracers");
	draw_player_esp = g_settings->getBool("enable_player_esp");
	draw_player_tracers = g_settings->getBool("enable_player_tracers");
	draw_node_esp = g_settings->getBool("enable_node_esp");
	draw_node_tracers = g_settings->getBool("enable_node_tracers");
	entity_esp_color = video::SColor(255, 255, 255, 255);
	player_esp_color = video::SColor(255, 0, 255, 0);
	self_esp_color = video::SColor(255, 255, 255, 0);

	ClientEnvironment &env = client->getEnv();
	Camera *camera = client->getCamera();

	v3f camera_offset = intToFloat(camera->getOffset(), BS);

	v3f eye_pos = (camera->getPosition() + 1000.0f*camera->getDirection() - camera_offset);

	video::SMaterial material, oldmaterial;
	oldmaterial = driver->getMaterial2D();
	material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA; //mark 1
	material.forEachTexture([] (video::SMaterialLayer &tex) {
		tex.MinFilter = irr::video::ETMINF_NEAREST_MIPMAP_NEAREST;
		tex.MagFilter = irr::video::ETMAGF_NEAREST;
	});
	material.ZBuffer = irr::video::ECFN_ALWAYS; //mark 2
	material.ZWriteEnable = irr::video::EZW_OFF;
	driver->setMaterial(material);

 	if (draw_entity_esp || draw_entity_tracers || draw_player_esp || draw_player_tracers) {
 		v3f current_pos = client->getEnv().getLocalPlayer()->getPosition();
 		std::vector<DistanceSortedActiveObject> allObjects;
		env.getAllActiveObjects(current_pos, allObjects);
		for (auto &clientobject : allObjects) {
			ClientActiveObject *cao = clientobject.obj;
			if ((cao->isLocalPlayer() && !g_settings->getBool("freecam")) || cao->getParent())
				continue;
			GenericCAO *obj = dynamic_cast<GenericCAO *>(cao);
			if (!obj) {
				continue;
			}
			//v3f velocity = obj->getVelocity();
			//v3f rotation = obj->getRotation();
			bool is_self = obj->isLocalPlayer();
			bool is_player = obj->isPlayer();
			bool draw_esp = is_player ? draw_player_esp : draw_entity_esp;
			bool draw_tracers = is_player ? draw_player_tracers : draw_entity_tracers;
			video::SColor color = is_player 
				? (is_self  
					? self_esp_color
			 		: player_esp_color)
				: entity_esp_color;
			if (! (draw_esp || draw_tracers))
				continue;
			aabb3f box(v3f(0,0,0), v3f(0,0,0));
			if (!obj->getSelectionBox(&box)) {
				continue;
			}

			v3f pos = obj->getPosition() - camera_offset;
			box.MinEdge += pos;
			box.MaxEdge += pos;

			if (draw_esp)
				driver->draw3DBox(box, color);
			if (draw_tracers)
				driver->draw3DLine(eye_pos, box.getCenter(), color);
		}
	}
	if (draw_node_esp || draw_node_tracers) {
		Map &map = env.getMap();
		std::vector<v3s16> positions;
		map.listAllLoadedBlocks(positions);
		for (v3s16 blockp : positions) {
			MapBlock *block = map.getBlockNoCreate(blockp);
			if (!block->mesh)
				continue;
			for (v3s16 p : block->mesh->esp_nodes) {
				v3f pos = intToFloat(p, BS) - camera_offset;
				MapNode node = map.getNode(p);
				std::vector<aabb3f> boxes;
				node.getSelectionBoxes(client->getNodeDefManager(), &boxes, node.getNeighbors(p, &map));
				video::SColor color = client->getNodeDefManager()->get(node).minimap_color;
				for (aabb3f box : boxes) {
					box.MinEdge += pos;
					box.MaxEdge += pos;
					if (draw_node_esp)
						driver->draw3DBox(box, color);
					if (draw_node_tracers)
						driver->draw3DLine(eye_pos, box.getCenter(), color);
				}
			}
		}
	}

	driver->setMaterial(oldmaterial);
}
void RenderingCore::Draw3D(PipelineContext &context)
{
	context.device->getSceneManager()->drawAll();
	context.device->getVideoDriver()->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	if (!context.show_hud)
		return;
	context.hud->drawBlockBounds();
	context.hud->drawSelectionMesh();
}

void RenderingCore::DrawWield(PipelineContext &context)
{
	if (context.draw_wield_tool)
		context.client->getCamera()->drawWieldedTool();
}

void RenderingCore::DrawHUD(PipelineContext &context)
{
	if (context.show_hud) {
		if (context.shadow_renderer)
			context.shadow_renderer->drawDebug();

		context.hud->resizeHotbar();

		if (context.draw_crosshair)
			context.hud->drawCrosshair();

		//ontext.hud->drawHotbar(v2s32(0,0), v2f(0.0f,0.0f), context.client->getEnv().getLocalPlayer()->getWieldIndex(), v2f(0.0f,0.0f));
		context.hud->drawLuaElements(context.client->getCamera()->getOffset());
		context.client->getCamera()->drawNametags();
	}
	context.device->getGUIEnvironment()->drawAll();
}

void RenderingCore::MapPostFxStep(PipelineContext &context)
{
	context.client->getEnv().getClientMap().renderPostFx(context.client->getCamera()->getCameraMode());
}

void RenderingCore::RenderShadowMapStep(PipelineContext &context)
{
	// This is necessary to render shadows for animations correctly
	context.device->getSceneManager()->getRootSceneNode()->OnAnimate(context.device->getTimer()->getTime());
	context.shadow_renderer->update();
}
