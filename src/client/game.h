// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

#pragma once
#include "irrlichttypes.h"
#include "config.h"
#include <csignal>
#include "gui/cheatMenu.h"
#include <string>

#include "client.h"
#include "log_internal.h"
#include "client/renderingengine.h"
#include "client/clientevent.h"
#include "client/inputhandler.h"
#include "client/keys.h"
#include "client/event_manager.h"
#include "sky.h"
#include "clouds.h"
#include "server.h"
#include "gui/guiChatConsole.h"
#include "gui/profilergraph.h"
#include "util/pointedthing.h"
#include "util/quicktune_shortcutter.h"
#include "irr_ptr.h"
#include <IAnimatedMeshSceneNode.h>

#include <iomanip>
#include <cmath>
#include "gameui.h"
#include "camera.h"
#include "client/texturepaths.h"
#include "client/joystick_controller.h"
#include "client/mapblock_mesh.h"
#include "client/sound.h"
#include "clientmap.h"
#include "clientmedia.h" // For clientMediaUpdateCacheCopy
#include "config.h"
#include "content_cao.h"
#include "content/subgames.h"
#include "fontengine.h"
#include "itemdef.h"
#include "log.h"
#include "filesys.h"
#include "gameparams.h"
#include "gettext.h"
#include "gui/touchcontrols.h"
#include "gui/guiFormSpecMenu.h"
#include "client/game_formspec.h"
#include "gui/guiPasswordChange.h"
#include "gui/guiOpenURL.h"
#include "gui/guiVolumeChange.h"
#include "gui/mainmenumanager.h"
#include "mapblock.h"
#include "minimap.h"
#include "nodedef.h"         // Needed for determining pointing to nodes
#include "nodemetadata.h"
#include "particles.h"
#include "porting.h"
#include "profiler.h"
#include "raycast.h"
#include "settings.h"
#include "shader.h"
#include "gui/cheatMenu.h"
#include "threading/lambda.h"
#include "translation.h"
#include "util/basic_macros.h"
#include "util/directiontables.h"
#include "irrlicht_changes/static_text.h"
#include "version.h"
#include "script/scripting_client.h"
#include "clientdynamicinfo.h"


#if !IS_CLIENT_BUILD
#error Do not include in server builds
#endif

class InputHandler;
class ChatBackend;
class RenderingEngine;
struct GameStartData;
class GameUI;

struct Jitter {
	f32 max, min, avg, counter, max_sample, min_sample, max_fraction;
};

struct RunStats {
	u64 drawtime; // (us)

	Jitter dtime_jitter, busy_time_jitter;
};

struct CameraOrientation {
	f32 camera_yaw;    // "right/left"
	f32 camera_pitch;  // "up/down"
};
//HERE COMES THE BIG LOAD


class NodeDugEvent : public MtEvent
{
public:
	v3s16 p;
	MapNode n;

	NodeDugEvent(v3s16 p, MapNode n):
		p(p),
		n(n)
	{}
	Type getType() const { return NODE_DUG; }
};

class SoundMaker
{
	ISoundManager *m_sound;
	const NodeDefManager *m_ndef;

public:
	bool makes_footstep_sound = true;
	float m_player_step_timer = 0.0f;
	float m_player_jump_timer = 0.0f;

	SoundSpec m_player_step_sound;
	SoundSpec m_player_leftpunch_sound;
	// Second sound made on left punch, currently used for item 'use' sound
	SoundSpec m_player_leftpunch_sound2;
	SoundSpec m_player_rightpunch_sound;

	SoundMaker(ISoundManager *sound, const NodeDefManager *ndef) :
		m_sound(sound), m_ndef(ndef) {}

	void playPlayerStep()
	{
		if (m_player_step_timer <= 0 && m_player_step_sound.exists()) {
			m_player_step_timer = 0.03;
			if (makes_footstep_sound)
				m_sound->playSound(0, m_player_step_sound);
		}
	}

	void playPlayerJump()
	{
		if (m_player_jump_timer <= 0.0f) {
			m_player_jump_timer = 0.2f;
			m_sound->playSound(0, SoundSpec("player_jump", 0.5f));
		}
	}

	static void viewBobbingStep(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->playPlayerStep();
	}

	static void playerRegainGround(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->playPlayerStep();
	}

	static void playerJump(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->playPlayerJump();
	}

	static void cameraPunchLeft(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->m_sound->playSound(0, sm->m_player_leftpunch_sound);
		sm->m_sound->playSound(0, sm->m_player_leftpunch_sound2);
	}

	static void cameraPunchRight(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->m_sound->playSound(0, sm->m_player_rightpunch_sound);
	}

	static void nodeDug(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		NodeDugEvent *nde = (NodeDugEvent *)e;
		sm->m_sound->playSound(0, sm->m_ndef->get(nde->n).sound_dug);
	}

	static void playerDamage(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->m_sound->playSound(0, SoundSpec("player_damage", 0.5));
	}

	static void playerFallingDamage(MtEvent *e, void *data)
	{
		SoundMaker *sm = (SoundMaker *)data;
		sm->m_sound->playSound(0, SoundSpec("player_falling_damage", 0.5));
	}

	void registerReceiver(MtEventManager *mgr)
	{
		mgr->reg(MtEvent::VIEW_BOBBING_STEP, SoundMaker::viewBobbingStep, this);
		mgr->reg(MtEvent::PLAYER_REGAIN_GROUND, SoundMaker::playerRegainGround, this);
		mgr->reg(MtEvent::PLAYER_JUMP, SoundMaker::playerJump, this);
		mgr->reg(MtEvent::CAMERA_PUNCH_LEFT, SoundMaker::cameraPunchLeft, this);
		mgr->reg(MtEvent::CAMERA_PUNCH_RIGHT, SoundMaker::cameraPunchRight, this);
		mgr->reg(MtEvent::NODE_DUG, SoundMaker::nodeDug, this);
		mgr->reg(MtEvent::PLAYER_DAMAGE, SoundMaker::playerDamage, this);
		mgr->reg(MtEvent::PLAYER_FALLING_DAMAGE, SoundMaker::playerFallingDamage, this);
	}

	void step(float dtime)
	{
		m_player_step_timer -= dtime;
		m_player_jump_timer -= dtime;
	}
};


typedef s32 SamplerLayer_t;


class GameGlobalShaderUniformSetter : public IShaderUniformSetter
{
	Sky *m_sky;
	Client *m_client;
	CachedVertexShaderSetting<float> m_animation_timer_vertex{"animationTimer"};
	CachedPixelShaderSetting<float> m_animation_timer_pixel{"animationTimer"};
	CachedVertexShaderSetting<float>
		m_animation_timer_delta_vertex{"animationTimerDelta"};
	CachedPixelShaderSetting<float>
		m_animation_timer_delta_pixel{"animationTimerDelta"};
	CachedPixelShaderSetting<float, 3> m_day_light{"dayLight"};
	CachedPixelShaderSetting<float, 3> m_minimap_yaw{"yawVec"};
	CachedPixelShaderSetting<float, 3> m_camera_offset_pixel{"cameraOffset"};
	CachedVertexShaderSetting<float, 3> m_camera_offset_vertex{"cameraOffset"};
	CachedPixelShaderSetting<float, 3> m_camera_position_pixel{ "cameraPosition" };
	CachedVertexShaderSetting<float, 3> m_camera_position_vertex{ "cameraPosition" };
	CachedVertexShaderSetting<float, 2> m_texel_size0_vertex{"texelSize0"};
	CachedPixelShaderSetting<float, 2> m_texel_size0_pixel{"texelSize0"};
	v2f m_texel_size0;
	CachedStructPixelShaderSetting<float, 7> m_exposure_params_pixel{
		"exposureParams",
		std::array<const char*, 7> {
			"luminanceMin", "luminanceMax", "exposureCorrection",
			"speedDarkBright", "speedBrightDark", "centerWeightPower",
			"compensationFactor"
		}};
	float m_user_exposure_compensation;
	bool m_bloom_enabled;
	CachedPixelShaderSetting<float> m_bloom_intensity_pixel{"bloomIntensity"};
	CachedPixelShaderSetting<float> m_bloom_strength_pixel{"bloomStrength"};
	CachedPixelShaderSetting<float> m_bloom_radius_pixel{"bloomRadius"};
	CachedPixelShaderSetting<float> m_saturation_pixel{"saturation"};
	bool m_volumetric_light_enabled;
	CachedPixelShaderSetting<float, 3>
		m_sun_position_pixel{"sunPositionScreen"};
	CachedPixelShaderSetting<float> m_sun_brightness_pixel{"sunBrightness"};
	CachedPixelShaderSetting<float, 3>
		m_moon_position_pixel{"moonPositionScreen"};
	CachedPixelShaderSetting<float> m_moon_brightness_pixel{"moonBrightness"};
	CachedPixelShaderSetting<float>
		m_volumetric_light_strength_pixel{"volumetricLightStrength"};

	static constexpr std::array<const char*, 1> SETTING_CALLBACKS = {
		"exposure_compensation",
	};

public:
	void onSettingsChange(const std::string &name)
	{
		if (name == "exposure_compensation")
			m_user_exposure_compensation = g_settings->getFloat("exposure_compensation", -1.0f, 1.0f);
	}

	static void settingsCallback(const std::string &name, void *userdata)
	{
		reinterpret_cast<GameGlobalShaderUniformSetter*>(userdata)->onSettingsChange(name);
	}

	void setSky(Sky *sky) { m_sky = sky; }

	GameGlobalShaderUniformSetter(Sky *sky, Client *client) :
		m_sky(sky),
		m_client(client)
	{
		for (auto &name : SETTING_CALLBACKS)
			g_settings->registerChangedCallback(name, settingsCallback, this);

		m_user_exposure_compensation = g_settings->getFloat("exposure_compensation", -1.0f, 1.0f);
		m_bloom_enabled = g_settings->getBool("enable_bloom");
		m_volumetric_light_enabled = g_settings->getBool("enable_volumetric_lighting") && m_bloom_enabled;
	}

	~GameGlobalShaderUniformSetter()
	{
		g_settings->deregisterAllChangedCallbacks(this);
	}

	void onSetUniforms(video::IMaterialRendererServices *services) override
	{
		u32 daynight_ratio = (float)m_client->getEnv().getDayNightRatio();
		video::SColorf sunlight;
		get_sunlight_color(&sunlight, daynight_ratio);
		m_day_light.set(sunlight, services);

		u32 animation_timer = m_client->getEnv().getFrameTime() % 1000000;
		float animation_timer_f = (float)animation_timer / 100000.f;
		m_animation_timer_vertex.set(&animation_timer_f, services);
		m_animation_timer_pixel.set(&animation_timer_f, services);

		float animation_timer_delta_f = (float)m_client->getEnv().getFrameTimeDelta() / 100000.f;
		m_animation_timer_delta_vertex.set(&animation_timer_delta_f, services);
		m_animation_timer_delta_pixel.set(&animation_timer_delta_f, services);

		if (m_client->getMinimap()) {
			v3f minimap_yaw = m_client->getMinimap()->getYawVec();
			m_minimap_yaw.set(minimap_yaw, services);
		}

		v3f offset = intToFloat(m_client->getCamera()->getOffset(), BS);
		m_camera_offset_pixel.set(offset, services);
		m_camera_offset_vertex.set(offset, services);

		v3f camera_position = m_client->getCamera()->getPosition();
		m_camera_position_pixel.set(camera_position, services);
		m_camera_position_pixel.set(camera_position, services);

		m_texel_size0_vertex.set(m_texel_size0, services);
		m_texel_size0_pixel.set(m_texel_size0, services);

		const auto &lighting = m_client->getEnv().getLocalPlayer()->getLighting();

		const AutoExposure &exposure_params = lighting.exposure;
		std::array<float, 7> exposure_buffer = {
			std::pow(2.0f, exposure_params.luminance_min),
			std::pow(2.0f, exposure_params.luminance_max),
			exposure_params.exposure_correction,
			exposure_params.speed_dark_bright,
			exposure_params.speed_bright_dark,
			exposure_params.center_weight_power,
			powf(2.f, m_user_exposure_compensation)
		};
		m_exposure_params_pixel.set(exposure_buffer.data(), services);

		if (m_bloom_enabled) {
			float intensity = std::max(lighting.bloom_intensity, 0.0f);
			m_bloom_intensity_pixel.set(&intensity, services);
			float strength_factor = std::max(lighting.bloom_strength_factor, 0.0f);
			m_bloom_strength_pixel.set(&strength_factor, services);
			float radius = std::max(lighting.bloom_radius, 0.0f);
			m_bloom_radius_pixel.set(&radius, services);
		}

		float saturation = lighting.saturation;
		m_saturation_pixel.set(&saturation, services);

		if (m_volumetric_light_enabled) {
			// Map directional light to screen space
			auto camera_node = m_client->getCamera()->getCameraNode();
			core::matrix4 transform = camera_node->getProjectionMatrix();
			transform *= camera_node->getViewMatrix();

			if (m_sky->getSunVisible()) {
				v3f sun_position = camera_node->getAbsolutePosition() +
						10000.f * m_sky->getSunDirection();
				transform.transformVect(sun_position);
				sun_position.normalize();

				m_sun_position_pixel.set(sun_position, services);

				float sun_brightness = core::clamp(107.143f * m_sky->getSunDirection().Y, 0.f, 1.f);
				m_sun_brightness_pixel.set(&sun_brightness, services);
			} else {
				m_sun_position_pixel.set(v3f(0.f, 0.f, -1.f), services);

				float sun_brightness = 0.f;
				m_sun_brightness_pixel.set(&sun_brightness, services);
			}

			if (m_sky->getMoonVisible()) {
				v3f moon_position = camera_node->getAbsolutePosition() +
						10000.f * m_sky->getMoonDirection();
				transform.transformVect(moon_position);
				moon_position.normalize();

				m_moon_position_pixel.set(moon_position, services);

				float moon_brightness = core::clamp(107.143f * m_sky->getMoonDirection().Y, 0.f, 1.f);
				m_moon_brightness_pixel.set(&moon_brightness, services);
			} else {
				m_moon_position_pixel.set(v3f(0.f, 0.f, -1.f), services);

				float moon_brightness = 0.f;
				m_moon_brightness_pixel.set(&moon_brightness, services);
			}

			float volumetric_light_strength = lighting.volumetric_light_strength;
			m_volumetric_light_strength_pixel.set(&volumetric_light_strength, services);
		}
	}

	void onSetMaterial(const video::SMaterial &material) override
	{
		video::ITexture *texture = material.getTexture(0);
		if (texture) {
			core::dimension2du size = texture->getSize();
			m_texel_size0 = v2f(1.f / size.Width, 1.f / size.Height);
		} else {
			m_texel_size0 = v2f();
		}
	}
};


class GameGlobalShaderUniformSetterFactory : public IShaderUniformSetterFactory
{
	Sky *m_sky = nullptr;
	Client *m_client;
	std::vector<GameGlobalShaderUniformSetter *> created_nosky;
public:
	GameGlobalShaderUniformSetterFactory(Client *client) :
		m_client(client)
	{}

	void setSky(Sky *sky)
	{
		m_sky = sky;
		for (GameGlobalShaderUniformSetter *ggscs : created_nosky) {
			ggscs->setSky(m_sky);
		}
		created_nosky.clear();
	}

	virtual IShaderUniformSetter* create()
	{
		auto *scs = new GameGlobalShaderUniformSetter(m_sky, m_client);
		if (!m_sky)
			created_nosky.push_back(scs);
		return scs;
	}
};

class NodeShaderConstantSetter : public IShaderConstantSetter
{
public:
	NodeShaderConstantSetter() = default;
	~NodeShaderConstantSetter() = default;

	void onGenerate(const std::string &name, ShaderConstants &constants) override
	{
		if (constants.find("DRAWTYPE") == constants.end())
			return; // not a node shader
		[[maybe_unused]] const auto drawtype =
			static_cast<NodeDrawType>(std::get<int>(constants["DRAWTYPE"]));
		[[maybe_unused]] const auto material_type =
			static_cast<MaterialType>(std::get<int>(constants["MATERIAL_TYPE"]));

#define PROVIDE(constant) constants[ #constant ] = (int)constant

		PROVIDE(NDT_NORMAL);
		PROVIDE(NDT_AIRLIKE);
		PROVIDE(NDT_LIQUID);
		PROVIDE(NDT_FLOWINGLIQUID);
		PROVIDE(NDT_GLASSLIKE);
		PROVIDE(NDT_ALLFACES);
		PROVIDE(NDT_ALLFACES_OPTIONAL);
		PROVIDE(NDT_TORCHLIKE);
		PROVIDE(NDT_SIGNLIKE);
		PROVIDE(NDT_PLANTLIKE);
		PROVIDE(NDT_FENCELIKE);
		PROVIDE(NDT_RAILLIKE);
		PROVIDE(NDT_NODEBOX);
		PROVIDE(NDT_GLASSLIKE_FRAMED);
		PROVIDE(NDT_FIRELIKE);
		PROVIDE(NDT_GLASSLIKE_FRAMED_OPTIONAL);
		PROVIDE(NDT_PLANTLIKE_ROOTED);

		PROVIDE(TILE_MATERIAL_BASIC);
		PROVIDE(TILE_MATERIAL_ALPHA);
		PROVIDE(TILE_MATERIAL_LIQUID_TRANSPARENT);
		PROVIDE(TILE_MATERIAL_LIQUID_OPAQUE);
		PROVIDE(TILE_MATERIAL_WAVING_LEAVES);
		PROVIDE(TILE_MATERIAL_WAVING_PLANTS);
		PROVIDE(TILE_MATERIAL_OPAQUE);
		PROVIDE(TILE_MATERIAL_WAVING_LIQUID_BASIC);
		PROVIDE(TILE_MATERIAL_WAVING_LIQUID_TRANSPARENT);
		PROVIDE(TILE_MATERIAL_WAVING_LIQUID_OPAQUE);
		PROVIDE(TILE_MATERIAL_PLAIN);
		PROVIDE(TILE_MATERIAL_PLAIN_ALPHA);

#undef PROVIDE

		bool enable_waving_water = g_settings->getBool("enable_waving_water");
		constants["ENABLE_WAVING_WATER"] = enable_waving_water ? 1 : 0;
		if (enable_waving_water) {
			constants["WATER_WAVE_HEIGHT"] = g_settings->getFloat("water_wave_height");
			constants["WATER_WAVE_LENGTH"] = g_settings->getFloat("water_wave_length");
			constants["WATER_WAVE_SPEED"] = g_settings->getFloat("water_wave_speed");
		}
		switch (material_type) {
			case TILE_MATERIAL_WAVING_LIQUID_TRANSPARENT:
			case TILE_MATERIAL_WAVING_LIQUID_OPAQUE:
			case TILE_MATERIAL_WAVING_LIQUID_BASIC:
				constants["MATERIAL_WAVING_LIQUID"] = 1;
				break;
			default:
				constants["MATERIAL_WAVING_LIQUID"] = 0;
				break;
		}
		switch (material_type) {
			case TILE_MATERIAL_WAVING_LIQUID_TRANSPARENT:
			case TILE_MATERIAL_WAVING_LIQUID_OPAQUE:
			case TILE_MATERIAL_WAVING_LIQUID_BASIC:
			case TILE_MATERIAL_LIQUID_TRANSPARENT:
				constants["MATERIAL_WATER_REFLECTIONS"] = 1;
				break;
			default:
				constants["MATERIAL_WATER_REFLECTIONS"] = 0;
				break;
		}

		constants["ENABLE_WAVING_LEAVES"] = g_settings->getBool("enable_waving_leaves") ? 1 : 0;
		constants["ENABLE_WAVING_PLANTS"] = g_settings->getBool("enable_waving_plants") ? 1 : 0;
	}
};

/****************************************************************************
 ****************************************************************************/

const static float object_hit_delay = 0.2;

const static u16 bbox_debug_flag = scene::EDS_BBOX_ALL;

/* The reason the following structs are not anonymous structs within the
 * class is that they are not used by the majority of member functions and
 * many functions that do require objects of thse types do not modify them
 * (so they can be passed as a const qualified parameter)
 */

struct GameRunData {
	u16 dig_index;
	u16 new_playeritem;
	PointedThing pointed_old;
	bool digging;
	bool punching;
	bool btn_down_for_dig;
	bool dig_instantly;
	bool digging_blocked;
	bool reset_jump_timer;
	float nodig_delay_timer;
	float dig_time;
	float dig_time_complete;
	float repeat_place_timer;
	float object_hit_delay_timer;
	float time_from_last_punch;
	ClientActiveObject *selected_object;

	float jump_timer_up;          // from key up until key down
	float jump_timer_down;        // since last key down
	float jump_timer_down_before; // from key down until key down again

	float damage_flash;
	float update_draw_list_timer;
	float touch_blocks_timer;

	f32 fog_range;

	v3f update_draw_list_last_cam_dir;

	float time_of_day_smooth;
};

class Game;

struct ClientEventHandler
{
	void (Game::*handler)(ClientEvent *, CameraOrientation *);
};

/****************************************************************************
 THE GAME
 ****************************************************************************/

using PausedNodesList = std::vector<std::pair<irr_ptr<scene::IAnimatedMeshSceneNode>, float>>;

/* This is not intended to be a public class. If a public class becomes
 * desirable then it may be better to create another 'wrapper' class that
 * hides most of the stuff in this class (nothing in this class is required
 * by any other file) but exposes the public methods/data only.
 */
class Game {
public:
	Game();
	~Game();

	bool startup(bool *kill,
			InputHandler *input,
			RenderingEngine *rendering_engine,
			const GameStartData &game_params,
			std::string &error_message,
			bool *reconnect,
			ChatBackend *chat_backend);

	void run();
	void shutdown();

	const PointedThing &getPointedOld() const { return runData.pointed_old; }

protected:

	// Basic initialisation
	bool init(const std::string &map_dir, const std::string &address,
			u16 port, const SubgameSpec &gamespec);
	bool initSound();
	bool createServer(const std::string &map_dir,
			const SubgameSpec &gamespec, u16 port);
	void copyServerClientCache();

	// Client creation
	bool createClient(const GameStartData &start_data);
	bool initGui();

	// Client connection
	bool connectToServer(const GameStartData &start_data,
			bool *connect_ok, bool *aborted);
	bool getServerContent(bool *aborted);

	// Main loop

	void updateInteractTimers(f32 dtime);
	bool checkConnection();
	void processQueues();
	void updateProfilers(const RunStats &stats, const FpsControl &draw_times, f32 dtime);
	void updateDebugState();
	void updateStats(RunStats *stats, const FpsControl &draw_times, f32 dtime);
	void updateProfilerGraphs(ProfilerGraph *graph);

	// Input related
	void processUserInput(f32 dtime);
	void processKeyInput();
	void processItemSelection(u16 *new_playeritem);
	bool shouldShowTouchControls();

	void dropSelectedItem(bool single_item = false);
	void openConsole(float scale, const wchar_t *line=NULL);
	void toggleFreeMove();
	void toggleFreeMoveAlt();
	void togglePitchMove();
	void toggleFast();
	void toggleNoClip();
	void toggleCinematic();
	void toggleBlockBounds();
	void toggleAutoforward();

	void toggleMinimap(bool shift_pressed);
	void toggleFog();
	void toggleDebug();
	void toggleUpdateCamera();

	void increaseViewRange();
	void decreaseViewRange();
	void toggleFullViewRange();
	void checkZoomEnabled();

	void updateCameraDirection(CameraOrientation *cam, float dtime);
	void updateCameraOrientation(CameraOrientation *cam, float dtime);
	bool getTogglableKeyState(GameKeyType key, bool toggling_enabled, bool prev_key_state);
	void updatePlayerControl(const CameraOrientation &cam);
	void updatePauseState();
	void step(f32 dtime);
	void processClientEvents(CameraOrientation *cam);
	void updateCameraMode(); // call after changing it
	void updateCameraOffset();
	void updateCamera(f32 dtime);
	void updateSound(f32 dtime);
	void processPlayerInteraction(f32 dtime, bool show_hud);
	/*!
	 * Returns the object or node the player is pointing at.
	 * Also updates the selected thing in the Hud.
	 *
	 * @param[in]  shootline         the shootline, starting from
	 * the camera position. This also gives the maximal distance
	 * of the search.
	 * @param[in]  liquids_pointable if false, liquids are ignored
	 * @param[in]  pointabilities    item specific pointable overriding
	 * @param[in]  look_for_object   if false, objects are ignored
	 * @param[in]  camera_offset     offset of the camera
	 * @param[out] selected_object   the selected object or
	 * NULL if not found
	 */
	PointedThing updatePointedThing(
			const core::line3d<f32> &shootline, bool liquids_pointable,
			const std::optional<Pointabilities> &pointabilities,
			bool look_for_object, const v3s16 &camera_offset);
	void handlePointingAtNothing(const ItemStack &playerItem);
	void handlePointingAtNode(const PointedThing &pointed,
			const ItemStack &selected_item, const ItemStack &hand_item, f32 dtime);
	void handlePointingAtObject(const PointedThing &pointed, const ItemStack &playeritem,
			const ItemStack &hand_item, const v3f &player_position, bool show_debug);
	void handleDigging(const PointedThing &pointed, const v3s16 &nodepos,
			const ItemStack &selected_item, const ItemStack &hand_item, f32 dtime);
	void updateFrame(ProfilerGraph *graph, RunStats *stats, f32 dtime,
			const CameraOrientation &cam);
	void updateClouds(float dtime);
	void updateShadows();
	void drawScene(ProfilerGraph *graph, RunStats *stats, float dtime);

	// Misc
	void showOverlayMessage(const char *msg, float dtime, int percent,
			float *indef_pos = nullptr);

	inline bool fogEnabled()
	{
		// Client setting only takes effect if fog distance unlimited or debug priv
		if (sky->getFogDistance() < 0 || client->checkPrivilege("debug"))
			return m_cache_enable_fog;
		return true;
	}

	static void settingChangedCallback(const std::string &setting_name, void *data);
	static void updateAllMapBlocksCallback(const std::string &setting_name, void *data);
	void readSettings();

	inline bool isKeyDown(GameKeyType k)
	{
		return input->isKeyDown(k);
	}
	inline bool wasKeyDown(GameKeyType k)
	{
		return input->wasKeyDown(k);
	}
	inline bool wasKeyPressed(GameKeyType k)
	{
		return input->wasKeyPressed(k);
	}
	inline bool wasKeyReleased(GameKeyType k)
	{
		return input->wasKeyReleased(k);
	}

#ifdef __ANDROID__
	void handleAndroidChatInput();
#endif

private:
	struct Flags {
		bool disable_camera_update = false;
		/// 0 = no debug text active, see toggleDebug() for the rest
		int debug_state = 0;
	};

	void pauseAnimation();
	void resumeAnimation();

	// ClientEvent handlers
	void handleClientEvent_None(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_PlayerDamage(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_PlayerForceMove(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_DeathscreenLegacy(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_ShowFormSpec(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_ShowCSMFormSpec(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_ShowPauseMenuFormSpec(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_HandleParticleEvent(ClientEvent *event,
		CameraOrientation *cam);
	void handleClientEvent_HudAdd(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_HudRemove(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_HudChange(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_SetSky(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_SetSun(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_SetMoon(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_SetStars(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_OverrideDayNigthRatio(ClientEvent *event,
		CameraOrientation *cam);
	void handleClientEvent_CloudParams(ClientEvent *event, CameraOrientation *cam);
	void handleClientEvent_UpdateCamera(ClientEvent *event, CameraOrientation *cam);

	void updateChat(f32 dtime);

	bool nodePlacement(const ItemDefinition &selected_def, const ItemStack &selected_item,
		const v3s16 &nodepos, const v3s16 &neighborpos, const PointedThing &pointed,
		const NodeMetadata *meta);
	static const ClientEventHandler clientEventHandler[CLIENTEVENT_MAX];

	f32 getSensitivityScaleFactor() const;

	InputHandler *input = nullptr;

	Client *client = nullptr;
	Server *server = nullptr;

	ClientDynamicInfo client_display_info{};
	float dynamic_info_send_timer = 0;

	IWritableTextureSource *texture_src = nullptr;
	IWritableShaderSource *shader_src = nullptr;

	// When created, these will be filled with data received from the server
	IWritableItemDefManager *itemdef_manager = nullptr;
	NodeDefManager *nodedef_manager = nullptr;
	std::unique_ptr<ItemVisualsManager> m_item_visuals_manager;

	std::unique_ptr<ISoundManager> sound_manager;
	SoundMaker *soundmaker = nullptr;

	ChatBackend *chat_backend = nullptr;
	CaptureLogOutput m_chat_log_buf;

	EventManager *eventmgr = nullptr;
	QuicktuneShortcutter *quicktune = nullptr;

	std::unique_ptr<GameUI> m_game_ui;
	irr_ptr<GUIChatConsole> gui_chat_console;
	MapDrawControl *draw_control = nullptr;
	Camera *camera = nullptr;
	irr_ptr<Clouds> clouds;
	irr_ptr<Sky> sky;
	Hud *hud = nullptr;
	Minimap *mapper = nullptr;
	GameFormSpec m_game_formspec;

	// Map server hud ids to client hud ids
	std::unordered_map<u32, u32> m_hud_server_to_client;

	GameRunData runData;
	Flags m_flags;

	/* 'cache'
	   This class does take ownership/responsibily for cleaning up etc of any of
	   these items (e.g. device)
	*/
	IrrlichtDevice *device;
	RenderingEngine *m_rendering_engine;
	video::IVideoDriver *driver;
	scene::ISceneManager *smgr;
	bool *kill;
	std::string *error_message;
	bool *reconnect_requested;
	PausedNodesList paused_animated_nodes;

	bool simple_singleplayer_mode;
	/* End 'cache' */

	/* Pre-calculated values
	 */
	int crack_animation_length;

	IntervalLimiter profiler_interval;

	/*
	 * TODO: Local caching of settings is not optimal and should at some stage
	 *       be updated to use a global settings object for getting thse values
	 *       (as opposed to the this local caching). This can be addressed in
	 *       a later release.
	 */
	bool m_cache_doubletap_jump;
	bool m_cache_toggle_sneak_key;
	bool m_cache_toggle_aux1_key;
	bool m_cache_enable_joysticks;
	bool m_cache_enable_fog;
	bool m_cache_enable_noclip;
	bool m_cache_enable_free_move;
	f32  m_cache_mouse_sensitivity;
	f32  m_cache_joystick_frustum_sensitivity;
	f32  m_repeat_place_time;
	f32  m_repeat_dig_time;
	f32  m_cache_cam_smoothing;

	bool m_invert_mouse;
	bool m_enable_hotbar_mouse_wheel;
	bool m_invert_hotbar_mouse_wheel;

	bool m_first_loop_after_window_activation = false;
	bool m_camera_offset_changed = false;
	bool m_game_focused = false;

	bool m_does_lost_focus_pause_game = false;

	// if true, (almost) the whole game is paused
	// this happens in pause menu in singleplayer
	bool m_is_paused = false;

	bool m_touch_simulate_aux1 = false;
	inline bool isTouchShootlineUsed()
	{
		return g_touchcontrols && g_touchcontrols->isShootlineAvailable() &&
				camera->getCameraMode() == CAMERA_MODE_FIRST;
	}
#ifdef __ANDROID__
	bool m_android_chat_open;
#endif

	float m_shutdown_progress = 0.0f;
};


#define GAME_FALLBACK_TIMEOUT 1.8f
#define GAME_CONNECTION_TIMEOUT 10.0f

void the_game(volatile std::sig_atomic_t *kill,
		InputHandler *input,
		RenderingEngine *rendering_engine,
		const GameStartData &start_data,
		std::string &error_message,
		ChatBackend &chat_backend,
		bool *reconnect_requested);

extern Game *g_game;

inline CheatMenu *m_cheat_menu = nullptr;
