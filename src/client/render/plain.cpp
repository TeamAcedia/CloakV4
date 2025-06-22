// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#include "plain.h"
#include "secondstage.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/clientmap.h"
#include "client/hud.h"
#include "client/minimap.h"
#include "client/shadows/dynamicshadowsrender.h"
#include <IGUIEnvironment.h>
#include "client/content_cao.h"
#include "nodedef.h"
#include "settings.h"
#include "client/mapblock_mesh.h"	
#include "script/scripting_client.h"
#include "core.h"

// Function to check each neighbor and return the flags of different ones.


/// Draw3D pipeline step
void Draw3D::run(PipelineContext &context)
{
	if (m_target)
		m_target->activate(context);

	context.device->getSceneManager()->drawAll();
	context.device->getVideoDriver()->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	if (!context.show_hud)
		return;
	context.hud->drawBlockBounds();
	context.hud->drawSelectionMesh();
}

void DrawWield::run(PipelineContext &context)
{
	if (m_target)
		m_target->activate(context);

	if (context.draw_wield_tool)
		context.client->getCamera()->drawWieldedTool();
}

void DrawHUD::run(PipelineContext &context)
{
	if (context.show_hud) {
		if (context.shadow_renderer)
			context.shadow_renderer->drawDebug();

		context.hud->resizeHotbar();

		if (context.draw_crosshair)
			context.hud->drawCrosshair();

		context.hud->drawLuaElements(context.client->getCamera()->getOffset());
		context.client->getCamera()->drawNametags();
		if (g_settings->getBool("enable_health_esp")) {	
			context.client->getCamera()->drawHealthESP();
		}
	}
	context.device->getGUIEnvironment()->drawAll();
}

u8 DrawTracersAndESP::getDifferentNeighborFlags(v3s16 p, Map &map, const MapNode &node) {
	u8 flags = 0;
	for (int i = 0; i < 6; ++i) {
		MapNode neighbor = map.getNode(p + directions[i]);
		if (neighbor.getContent() != node.getContent()) {
			flags |= (1 << i);
		}
	}
	return flags;
}

void DrawTracersAndESP::run(PipelineContext &context)
{
	auto driver = context.device->getVideoDriver();

	draw_entity_esp = g_settings->getBool("enable_entity_esp");
	draw_entity_tracers = g_settings->getBool("enable_entity_tracers");
	draw_player_esp = g_settings->getBool("enable_player_esp");
	draw_player_tracers = g_settings->getBool("enable_player_tracers");
	draw_node_esp = g_settings->getBool("enable_node_esp");
	draw_node_tracers = g_settings->getBool("enable_node_tracers");

	v3f entity_color = g_settings->getV3F("entity_esp_color").value();
	v3f friend_color = g_settings->getV3F("friend_esp_color").value();
	v3f enemy_color = g_settings->getV3F("enemy_esp_color").value();
	v3f allied_color = g_settings->getV3F("allied_esp_color").value();

	entity_esp_color = video::SColor(255, entity_color.X, entity_color.Y, entity_color.Z);
	friend_esp_color = video::SColor(255, friend_color.X, friend_color.Y, friend_color.Z);
	enemy_esp_color = video::SColor(255, enemy_color.X, enemy_color.Y, enemy_color.Z);
	allied_esp_color = video::SColor(255, allied_color.X, allied_color.Y, allied_color.Z);

	playerDT = g_settings->getU32("esp.player.drawType");
	playerEO = g_settings->getU32("esp.player.edgeOpacity");
	playerFO = g_settings->getU32("esp.player.faceOpacity");
	entityDT = g_settings->getU32("esp.entity.drawType");
	entityEO = g_settings->getU32("esp.entity.edgeOpacity");
	entityFO = g_settings->getU32("esp.entity.faceOpacity");
	nodeDT = g_settings->getU32("esp.node.drawType");
	nodeEO = g_settings->getU32("esp.node.edgeOpacity");
	nodeFO = g_settings->getU32("esp.node.faceOpacity");

	LocalPlayer *player = context.client->getEnv().getLocalPlayer();
	ClientEnvironment &env = context.client->getEnv();
	ClientMap &clientMap = env.getClientMap();
	Camera *camera = context.client->getCamera();

	u8 wanted_range  = std::fmin(255.0f, clientMap.getWantedRange());

	v3f camera_offset = intToFloat(camera->getOffset(), BS);

	v3f eye_pos = (camera->getPosition() + 1000.0f*camera->getDirection() - camera_offset);

	video::SMaterial material, oldmaterial;
	oldmaterial = driver->getMaterial2D();
	material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
	material.forEachTexture([] (video::SMaterialLayer &tex) {
		tex.MinFilter = irr::video::ETMINF_NEAREST_MIPMAP_NEAREST;
		tex.MagFilter = irr::video::ETMAGF_NEAREST;
	});
	material.ZBuffer = irr::video::ECFN_ALWAYS;
	material.ZWriteEnable = irr::video::EZW_OFF;
	driver->setMaterial(material);

	//int pCnt = 0, eCnt = 0, nCnt = 0;

 	if (draw_entity_esp || draw_entity_tracers || draw_player_esp || draw_player_tracers) {
 		v3f current_pos = context.client->getEnv().getLocalPlayer()->getPosition();
 		std::vector<DistanceSortedActiveObject> allObjects;
		env.getAllActiveObjects(current_pos, allObjects);
		for (auto &clientobject : allObjects) {
			ClientActiveObject *cao = clientobject.obj;
			if ((cao->isLocalPlayer() && !g_settings->getBool("freecam")))
				continue;
			GenericCAO *obj = dynamic_cast<GenericCAO *>(cao);
			if (!obj) {
				continue;
			}
			//v3f velocity = obj->getVelocity();
			//v3f rotation = obj->getRotation();
			EntityRelationship relationship = player->getEntityRelationship(obj);
			bool is_player = obj->isPlayer();
			bool draw_esp = is_player ? draw_player_esp : draw_entity_esp;
			bool draw_tracers = is_player ? draw_player_tracers : draw_entity_tracers;
			video::SColor color;
						switch (relationship) {
								case EntityRelationship::FRIEND:		
								color = friend_esp_color;		
								break;	
							case EntityRelationship::ENEMY:		
								color = enemy_esp_color;		
								break;		
							case EntityRelationship::ALLY:					
								color = allied_esp_color;						
								break;						
							default:						
								color = entity_esp_color;						
								break;	
						}
			if (! (draw_esp || draw_tracers)) {
				continue;
			}
			aabb3f box(v3f(0,0,0), v3f(0,0,0));
			if (!obj->getSelectionBox(&box)) {
				continue;
			}

			v3f pos = obj->getPosition() - camera_offset;
			box.MinEdge += pos;
			box.MaxEdge += pos;

			if (draw_esp) {
				if (is_player) {
					//pCnt += 1;
					driver->draw3DBox(box, color, playerDT, playerEO, playerFO);
				} else if (!cao->getParent()) {
					//eCnt += 1;				
					driver->draw3DBox(box, color, entityDT, entityEO, entityFO);				
				}
			}
			if (draw_tracers) {
				driver->draw3DLine(eye_pos, box.getCenter(), color);
			}
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
				if ((intToFloat(p, BS) - player->getLegitPosition()).getLengthSQ() > (wanted_range*BS) * (wanted_range*BS))
					continue;
				MapNode node = map.getNode(p);
				//nCnt += 1;
				u8 diffNeighbors = getDifferentNeighborFlags(p, map, node);
				if (!diffNeighbors)
					continue;
				std::vector<aabb3f> boxes;
				node.getSelectionBoxes(context.client->getNodeDefManager(), &boxes, node.getNeighbors(p, &map));
				video::SColor color = context.client->getNodeDefManager()->get(node).getNodeEspColor();
				for (aabb3f box : boxes) {
					box.MinEdge += pos;
					box.MaxEdge += pos;
					if (draw_node_esp) {
						driver->draw3DBox(box, color, nodeDT, nodeEO, nodeFO, diffNeighbors);
					}
					if (draw_node_tracers)
						driver->draw3DLine(eye_pos, box.getCenter(), color);
				}
			}
		}
	}

	driver->setMaterial(oldmaterial);
}

std::vector<TaskNode> DrawTaskBlocksAndTracers::task_nodes;
std::vector<TaskTracer> DrawTaskBlocksAndTracers::task_tracers;

void DrawTaskBlocksAndTracers::run(PipelineContext &context)
{
	auto driver = context.device->getVideoDriver();

	draw_task_blocks = g_settings->getBool("enable_task_nodes");
	draw_task_tracers = g_settings->getBool("enable_task_tracers");

	float time = context.device->getTimer()->getTime() / 1000.0f;

	Camera *camera = context.client->getCamera();

	v3f camera_offset = intToFloat(camera->getOffset(), BS);

	video::SMaterial material, oldmaterial;
	oldmaterial = driver->getMaterial2D();
	material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
	material.forEachTexture([] (video::SMaterialLayer &tex) {
		tex.MinFilter = irr::video::ETMINF_NEAREST_MIPMAP_NEAREST;
		tex.MagFilter = irr::video::ETMAGF_NEAREST;
	});
	material.ZBuffer = irr::video::ECFN_ALWAYS;
	material.ZWriteEnable = irr::video::EZW_OFF;
	driver->setMaterial(material);

	if (draw_task_blocks) {
	float anim = (std::sin(time * 2.f) + 1.f) / 2.f;

	for (const TaskNode &node : task_nodes) {
		v3f pos = node.position - camera_offset;

		// Outer box
		aabb3f outer_box(pos - v3f(BS / 2.f), pos + v3f(BS / 2.f));
		driver->draw3DBox(outer_box, node.color);

		v3f min_pos = pos - v3f(BS / 2.f, BS / 2.f, BS / 2.f);
		v3f max_pos = pos + v3f(BS / 2.f, -BS / 2.f + BS * anim, BS / 2.f);

		driver->draw3DBox(aabb3f(min_pos, max_pos), node.color);
	}
}

	if (draw_task_tracers) {
		for (const TaskTracer &tracer : task_tracers) {
			v3f start = tracer.start - camera_offset;
			v3f end = tracer.end - camera_offset;
			driver->draw3DLine(start, end, tracer.color);
		}
	}

	driver->setMaterial(oldmaterial);
}


void MapPostFxStep::setRenderTarget(RenderTarget * _target)
{
	target = _target;
}

void MapPostFxStep::run(PipelineContext &context)
{
	if (target)
		target->activate(context);

	context.client->getEnv().getClientMap().renderPostFx(context.client->getCamera()->getCameraMode());
}

void RenderShadowMapStep::run(PipelineContext &context)
{
	// This is necessary to render shadows for animations correctly
	context.device->getSceneManager()->getRootSceneNode()->OnAnimate(context.device->getTimer()->getTime());
	context.shadow_renderer->update();
}

// class UpscaleStep

void UpscaleStep::run(PipelineContext &context)
{
	video::ITexture *lowres = m_source->getTexture(0);
	m_target->activate(context);
	context.device->getVideoDriver()->draw2DImage(lowres,
			core::rect<s32>(0, 0, context.target_size.X, context.target_size.Y),
			core::rect<s32>(0, 0, lowres->getSize().Width, lowres->getSize().Height));
}

std::unique_ptr<RenderStep> create3DStage(Client *client, v2f scale)
{
	RenderStep *step = new Draw3D();
	if (g_settings->getBool("enable_post_processing")) {
		RenderPipeline *pipeline = new RenderPipeline();
		pipeline->addStep(pipeline->own(std::unique_ptr<RenderStep>(step)));

		auto effect = addPostProcessing(pipeline, step, scale, client);
		effect->setRenderTarget(pipeline->getOutput());
		step = pipeline;
	}
	return std::unique_ptr<RenderStep>(step);
}

static v2f getDownscaleFactor()
{
	u16 undersampling = MYMAX(g_settings->getU16("undersampling"), 1);
	return v2f(1.0f / undersampling);
}

RenderStep* addUpscaling(RenderPipeline *pipeline, RenderStep *previousStep, v2f downscale_factor, Client *client)
{
	const int TEXTURE_LOWRES_COLOR = 0;
	const int TEXTURE_LOWRES_DEPTH = 1;

	if (downscale_factor.X == 1.0f && downscale_factor.Y == 1.0f)
		return previousStep;

	// post-processing pipeline takes care of rescaling
	if (g_settings->getBool("enable_post_processing"))
		return previousStep;

	auto driver = client->getSceneManager()->getVideoDriver();
	video::ECOLOR_FORMAT color_format = selectColorFormat(driver);
	video::ECOLOR_FORMAT depth_format = selectDepthFormat(driver);

	// Initialize buffer
	TextureBuffer *buffer = pipeline->createOwned<TextureBuffer>();
	buffer->setTexture(TEXTURE_LOWRES_COLOR, downscale_factor, "lowres_color", color_format);
	buffer->setTexture(TEXTURE_LOWRES_DEPTH, downscale_factor, "lowres_depth", depth_format);

	// Attach previous step to the buffer
	TextureBufferOutput *buffer_output = pipeline->createOwned<TextureBufferOutput>(
			buffer, std::vector<u8> {TEXTURE_LOWRES_COLOR}, TEXTURE_LOWRES_DEPTH);
	previousStep->setRenderTarget(buffer_output);

	// Add upscaling step
	RenderStep *upscale = pipeline->createOwned<UpscaleStep>();
	upscale->setRenderSource(buffer);
	pipeline->addStep(upscale);

	return upscale;
}

void populatePlainPipeline(RenderPipeline *pipeline, Client *client)
{
	auto downscale_factor = getDownscaleFactor();
	auto step3D = pipeline->own(create3DStage(client, downscale_factor));
	pipeline->addStep(step3D);
	pipeline->addStep<DrawTracersAndESP>();
	pipeline->addStep<DrawTaskBlocksAndTracers>();
	pipeline->addStep<DrawWield>();
	pipeline->addStep<MapPostFxStep>();

	step3D = addUpscaling(pipeline, step3D, downscale_factor, client);

	step3D->setRenderTarget(pipeline->createOwned<ScreenTarget>());

	pipeline->addStep<DrawHUD>();
}

video::ECOLOR_FORMAT selectColorFormat(video::IVideoDriver *driver)
{
	u32 bits = g_settings->getU32("post_processing_texture_bits");
	if (bits >= 16 && driver->queryTextureFormat(video::ECF_A16B16G16R16F))
		return video::ECF_A16B16G16R16F;
	if (bits >= 10 && driver->queryTextureFormat(video::ECF_A2R10G10B10))
		return video::ECF_A2R10G10B10;
	return video::ECF_A8R8G8B8;
}

video::ECOLOR_FORMAT selectDepthFormat(video::IVideoDriver *driver)
{
	if (driver->queryTextureFormat(video::ECF_D24))
		return video::ECF_D24;
	if (driver->queryTextureFormat(video::ECF_D24S8))
		return video::ECF_D24S8;
	return video::ECF_D16; // fallback depth format
}
