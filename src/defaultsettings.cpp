/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "settings.h"
#include "porting.h"
#include "filesys.h"
#include "config.h"
#include "constants.h"
#include "porting.h"
#include "mapgen/mapgen.h" // Mapgen::setDefaultSettings
#include "util/string.h"


/*
 * inspired by https://github.com/systemd/systemd/blob/7aed43437175623e0f3ae8b071bbc500c13ce893/src/hostname/hostnamed.c#L406
 * this could be done in future with D-Bus using query:
 * busctl get-property org.freedesktop.hostname1 /org/freedesktop/hostname1 org.freedesktop.hostname1 Chassis
 */
static bool detect_touch()
{
#if defined(__ANDROID__)
	return true;
#elif defined(__linux__)
	std::string chassis_type;

	// device-tree platforms (non-X86)
	std::ifstream dtb_file("/proc/device-tree/chassis-type");
	if (dtb_file.is_open()) {
		std::getline(dtb_file, chassis_type);
		chassis_type.pop_back();

		if (chassis_type == "tablet" ||
		    chassis_type == "handset" ||
		    chassis_type == "watch")
			return true;

		if (!chassis_type.empty())
			return false;
	}
	// SMBIOS
	std::ifstream dmi_file("/sys/class/dmi/id/chassis_type");
	if (dmi_file.is_open()) {
		std::getline(dmi_file, chassis_type);

		if (chassis_type == "11" /* Handheld */ ||
		    chassis_type == "30" /* Tablet */)
			return true;

		return false;
	}

	// ACPI-based platforms
	std::ifstream acpi_file("/sys/firmware/acpi/pm_profile");
	if (acpi_file.is_open()) {
		std::getline(acpi_file, chassis_type);

		if (chassis_type == "8" /* Tablet */)
			return true;

		return false;
	}

	return false;
#else
	// we don't know, return default
	return false;
#endif
}

void set_default_settings()
{
	Settings *settings = Settings::createLayer(SL_DEFAULTS);
	bool has_touch = detect_touch();

	// Client and server
	settings->setDefault("language", "");
	settings->setDefault("name", "");
	settings->setDefault("bind_address", "");
	settings->setDefault("serverlist_url", "servers.minetest.net");

	// Client
	settings->setDefault("address", "");
	settings->setDefault("enable_sound", "true");
	settings->setDefault("enable_touch", bool_to_cstr(has_touch));
	settings->setDefault("sound_volume", "0.8");
	settings->setDefault("sound_volume_unfocused", "0.3");
	settings->setDefault("mute_sound", "false");
	settings->setDefault("sound_extensions_blacklist", "");
	settings->setDefault("enable_mesh_cache", "false");
	settings->setDefault("mesh_generation_interval", "0");
	settings->setDefault("mesh_generation_threads", "0");
	settings->setDefault("free_move", "false");
	settings->setDefault("pitch_move", "false");
	settings->setDefault("fast_move", "false");
	settings->setDefault("noclip", "false");
	settings->setDefault("screenshot_path", "screenshots");
	settings->setDefault("screenshot_format", "png");
	settings->setDefault("screenshot_quality", "0");
	settings->setDefault("client_unload_unused_data_timeout", "600");
	settings->setDefault("client_mapblock_limit", "7500");
	settings->setDefault("enable_build_where_you_stand", "false");
	settings->setDefault("curl_timeout", "20000");
	settings->setDefault("curl_parallel_limit", "8");
	settings->setDefault("curl_file_download_timeout", "300000");
	settings->setDefault("curl_verify_cert", "true");
	settings->setDefault("enable_remote_media_server", "true");
	settings->setDefault("enable_client_modding", "true");
	settings->setDefault("max_out_chat_queue_size", "20");
	settings->setDefault("pause_on_lost_focus", "false");
	settings->setDefault("enable_split_login_register", "true");
	settings->setDefault("occlusion_culler", "bfs");
	settings->setDefault("enable_raytraced_culling", "true");
	settings->setDefault("chat_weblink_color", "#8888FF");

	// Cheat Menu
	settings->setDefault("cheat_menu_font", "FM_HD");
	settings->setDefault("cheat_menu_bg_color", "(10, 15, 20)"); //greyish
	settings->setDefault("cheat_menu_bg_color_alpha", "173");
	settings->setDefault("cheat_menu_active_bg_color", "(50, 80, 175)"); //blueish
	settings->setDefault("cheat_menu_active_bg_color_alpha", "210");
	settings->setDefault("cheat_menu_font_color", "(255, 255, 255)"); //white
	settings->setDefault("cheat_menu_font_color_alpha", "195");
	settings->setDefault("cheat_menu_selected_font_color", "(0, 0, 0)"); //black
	settings->setDefault("cheat_menu_selected_font_color_alpha", "235");
	settings->setDefault("cheat_menu_head_height", "40");
	settings->setDefault("cheat_menu_entry_height", "25");
	settings->setDefault("cheat_menu_entry_width", "165");
	settings->setDefault("cheat_menu_debug_on", "false");
    settings->setDefault("use_old_menu", "false");
    settings->setDefault("use_menu_grid", "true");
	settings->setDefault("use_hints", "true");
    settings->setDefault("save_menu_category_positions", "true");
    settings->setDefault("newmenu_draw_type_full", "false");
    settings->setDefault("newmenu_draw_type_small", "false");
    settings->setDefault("newmenu_draw_type_meteor", "true");

	// Cheats
	settings->setDefault("blink", "false");
	settings->setDefault("xray", "false");
	settings->setDefault("xray.nodes", "default:stone,mcl_core:stone");
	settings->setDefault("fov_setting", "true");
	settings->setDefault("fov_step", "72.0f");
	settings->setDefault("autosneak", "false");
	settings->setDefault("instant_break", "false");
	settings->setDefault("antiknockback", "false");
	settings->setDefault("float_above_parent", "false");
	settings->setDefault("freecam", "false");
	settings->setDefault("norender.particles", "true");
	settings->setDefault("norender.particle_spawners", "true");
	settings->setDefault("jesus", "false");
	settings->setDefault("no_slow", "true");
	settings->setDefault("jetpack", "false");
	settings->setDefault("antislip", "false");
	settings->setDefault("airjump", "false");
	settings->setDefault("spider", "false");
	settings->setDefault("hud_flags_bypass", "true");
	settings->setDefault("enable_entity_esp", "true");
	settings->setDefault("appleaura.range", "3");
	settings->setDefault("enable_entity_tracers", "false");
	settings->setDefault("enable_combat_target_hud", "true");
	settings->setDefault("enable_player_esp.type", "Above Head");
	settings->setDefault("enable_player_esp", "true");
	settings->setDefault("enable_player_tracers", "true");
	settings->setDefault("enable_node_esp", "false");
	settings->setDefault("enable_node_tracers", "false");
	settings->setDefault("enable_tunnel_esp", "false");
	settings->setDefault("enable_tunnel_tracers", "false");
	settings->setDefault("node_esp_nodes", "mcl_chests:chest_small,mcl_chests:chest_left,mcl_chests:black_shulker_box_small,mcl_chests:blue_shulker_box_small,mcl_chests:brown_shulker_box_small,mcl_chests:cyan_shulker_box_small,mcl_chests:dark_green_shulker_box_small,mcl_chests:dark_grey_shulker_box_small,mcl_chests:lightblue_shulker_box_small,mcl_chests:green_shulker_box_small,mcl_chests:orange_shulker_box_small,mcl_chests:magenta_shulker_box_small,mcl_chests:pink_shulker_box_small,mcl_chests:violet_shulker_box_small,mcl_chests:red_shulker_box_small,mcl_chests:grey_shulker_box_small,mcl_chests:white_shulker_box_small,mcl_chests:yellow_shulker_box_small");
	settings->setDefault("autorespawn", "false");
	settings->setDefault("cheat_hud", "true");
	settings->setDefault("cheat_hud.offset", "0");
	settings->setDefault("cheat_hud.position", "Top");
	settings->setDefault("fastdig", "false");
	settings->setDefault("autodig", "false");
	settings->setDefault("spamclick", "false");
	settings->setDefault("autohit", "false");
	settings->setDefault("fastplace", "false");
	settings->setDefault("autoplace", "false");
	settings->setDefault("prevent_natural_damage", "true");
	settings->setDefault("coords", "false");
	settings->setDefault("fullbright", "true");
	settings->setDefault("friends", "{}");
	settings->setDefault("enemies", "{}");
	settings->setDefault("allies", "{}");
	settings->setDefault("staff", "{}");
	settings->setDefault("autostaff", "true");
	settings->setDefault("autostaff.warn_staff", "true");
	settings->setDefault("autoteam", "false");
	settings->setDefault("auto_heal", "true");
	settings->setDefault("auto_heal.hp", "15");
	settings->setDefault("auto_heal.delay", "0.1");
	settings->setDefault("auto_heal.cooldown", "0.5");
	settings->setDefault("orbit", "false");
    settings->setDefault("orbit.radius", "2");
	settings->setDefault("tpaura", "false");
	settings->setDefault("tpaura.distance", "10");
	settings->setDefault("tpaura.delay", "0.1");
	settings->setDefault("crystalspam", "false");
	settings->setDefault("combatlog", "false");
	settings->setDefault("critical_hits", "false");
	settings->setDefault("killaura", "false");
	settings->setDefault("killaura.players", "false");
	settings->setDefault("killaura.entities", "false");
	settings->setDefault("killaura.assist", "false");
	settings->setDefault("punch_interval", "auto");
	settings->setDefault("spamclick_multiplier", "6");
	settings->setDefault("reach", "false");
	settings->setDefault("randomize_version", "false"); //randomize the version the client sends to the server on-join
	settings->setDefault("priv_bypass_extra", "true"); //other priv bypass stuff. misc.
    settings->setDefault("priv_bypass", "false");
	settings->setDefault("autotool", "false");
    settings->setDefault("dont_point_nodes", "false");
	settings->setDefault("small_post_effect_color", "false");
    settings->setDefault("no_hurt_cam", "false");
    settings->setDefault("autoaim", "false");
    settings->setDefault("autoaim.distance", "2");
    settings->setDefault("velocity", "false");
    settings->setDefault("velocity.jump", "1");
    settings->setDefault("velocity.speed", "1");
    settings->setDefault("velocity.speed_crouch", "1");
    settings->setDefault("velocity.gravity", "1");
    settings->setDefault("overrides", "false");
    settings->setDefault("overrides.old_movement", "false");
    settings->setDefault("overrides.sneak_glitch", "false");
	settings->setDefault("be_a_bitch.hp", "4");
    settings->setDefault("targeting.distance", "4");
    settings->setDefault("targeting.target_mode", "Lowest HP");
    settings->setDefault("targeting.target_type", "Players");
    settings->setDefault("targeting.enemies_only", "true");
    settings->setDefault("no_force_rotate", "false");
    settings->setDefault("no_night", "false");
    settings->setDefault("nobob", "false");
	settings->setDefault("use_colored_nametags", "false");
	settings->setDefault("BHOP", "false");
	settings->setDefault("enable_health_esp", "false");
	settings->setDefault("enable_health_esp.players_only", "false");
	settings->setDefault("anti_afk", "false");
	settings->setDefault("panic", "false");
	settings->setDefault("step", "false");
	settings->setDefault("scaffold.active", "false");
	settings->setDefault("scaffold.jump_delay", "0.2");
	settings->setDefault("step.mult", "2.0f");
	settings->setDefault("enable_entity_esp.custom_color", "true");
	settings->setDefault("autoaim.y_offset", "0");
	settings->setDefault("fov_setting", "true");
	settings->setDefault("fov.step", "72.0f");
	settings->setDefault("appleaura.range", "5");
	settings->setDefault("flagaura", "true");
	settings->setDefault("flagaura.range", "5");
	settings->setDefault("background", "default");
	settings->setDefault("spammer", "false");
	settings->setDefault("spammer.cooldown", "5");
	settings->setDefault("spammer.message", "message");

	settings->setDefault("esp.player.drawType", "0");
	settings->setDefault("esp.player.edgeOpacity", "255");
	settings->setDefault("esp.player.faceOpacity", "100");

	settings->setDefault("esp.entity.drawType", "0");
	settings->setDefault("esp.entity.edgeOpacity", "255");
	settings->setDefault("esp.entity.faceOpacity", "100");

	settings->setDefault("esp.node.drawType", "2");
	settings->setDefault("esp.node.edgeOpacity", "255");
	settings->setDefault("esp.node.faceOpacity", "180");

	settings->setDefault("esp.tunnel.drawType", "1");
	settings->setDefault("esp.tunnel.edgeOpacity", "255");
	settings->setDefault("esp.tunnel.faceOpacity", "150");

	settings->setDefault("entity_esp_color", "(255, 255, 255)");
	settings->setDefault("friend_esp_color", "(255, 0, 255)");
	settings->setDefault("allied_esp_color", "(0, 255, 0)");
	settings->setDefault("staff_esp_color", "(0, 0, 255)");
	settings->setDefault("enemy_esp_color", "(255, 0, 0)");

	settings->setDefault("silence", "false");

	settings->setDefault("XrayNodes", "false");
	settings->setDefault("ESPNodes", "false");

	settings->setDefault("check_velocity", "false");

	// Keymap
	settings->setDefault("remote_port", "30000");
	settings->setDefault("keymap_forward", "KEY_KEY_W");
	settings->setDefault("keymap_autoforward", "");
	settings->setDefault("keymap_backward", "KEY_KEY_S");
	settings->setDefault("keymap_left", "KEY_KEY_A");
	settings->setDefault("keymap_right", "KEY_KEY_D");
	settings->setDefault("keymap_jump", "KEY_SPACE");
	settings->setDefault("keymap_sneak", "KEY_LSHIFT");
	settings->setDefault("keymap_dig", "KEY_LBUTTON");
	settings->setDefault("keymap_place", "KEY_RBUTTON");
	settings->setDefault("keymap_drop", "KEY_KEY_Q");
	settings->setDefault("keymap_zoom", "KEY_KEY_Z");
	settings->setDefault("keymap_inventory", "KEY_KEY_I");
	settings->setDefault("keymap_aux1", "KEY_KEY_E");
	settings->setDefault("keymap_chat", "KEY_KEY_T");
	settings->setDefault("keymap_cmd", "/");
	settings->setDefault("keymap_cmd_local", ".");
	settings->setDefault("keymap_minimap", "KEY_KEY_V");
	settings->setDefault("keymap_console", "KEY_F10");

	settings->setDefault("keymap_toggle_freecam", "KEY_KEY_G");
	settings->setDefault("keymap_toggle_killaura", "KEY_KEY_X");
	settings->setDefault("keymap_toggle_autoaim", "KEY_KEY_N");
	settings->setDefault("keymap_toggle_scaffold", "KEY_KEY_Y");
	settings->setDefault("keymap_toggle_blink", "KEY_KEY_B");

	settings->setDefault("keymap_select_up", "KEY_UP");
	settings->setDefault("keymap_select_down", "KEY_DOWN");
	settings->setDefault("keymap_select_left", "KEY_LEFT");
	settings->setDefault("keymap_select_right", "KEY_RIGHT");
	settings->setDefault("keymap_select_confirm", "KEY_RETURN");

	// See https://github.com/minetest/minetest/issues/12792
	settings->setDefault("keymap_rangeselect", has_touch ? "" : "");

	settings->setDefault("keymap_freemove", "KEY_KEY_K");
	settings->setDefault("keymap_pitchmove", "P");
	settings->setDefault("keymap_fastmove", "KEY_KEY_J");
	settings->setDefault("keymap_noclip", "KEY_KEY_H");
	settings->setDefault("keymap_hotbar_next", "KEY_KEY_N");
	settings->setDefault("keymap_hotbar_previous", "KEY_KEY_B");
	settings->setDefault("keymap_mute", "KEY_KEY_M");
	settings->setDefault("keymap_increase_volume", "");
	settings->setDefault("keymap_decrease_volume", "");
	settings->setDefault("keymap_cinematic", "");
	settings->setDefault("keymap_toggle_block_bounds", "");
	settings->setDefault("keymap_toggle_hud", "KEY_F1");
	settings->setDefault("keymap_toggle_chat", "KEY_F2");
	settings->setDefault("keymap_toggle_fog", "KEY_F3");
	settings->setDefault("keymap_toggle_cheat_menu", "KEY_F8");
#ifndef NDEBUG
	settings->setDefault("keymap_toggle_update_camera", "KEY_F4");
#else
	settings->setDefault("keymap_toggle_update_camera", "");
#endif
	settings->setDefault("keymap_toggle_debug", "KEY_F5");
	settings->setDefault("keymap_toggle_profiler", "KEY_F6");
	settings->setDefault("keymap_camera_mode", "KEY_KEY_C");
	settings->setDefault("keymap_screenshot", "KEY_F12");
	settings->setDefault("keymap_fullscreen", "KEY_F11");
	settings->setDefault("keymap_increase_viewing_range_min", "+");
	settings->setDefault("keymap_decrease_viewing_range_min", "-");
	settings->setDefault("keymap_slot1", "KEY_KEY_1");
	settings->setDefault("keymap_slot2", "KEY_KEY_2");
	settings->setDefault("keymap_slot3", "KEY_KEY_3");
	settings->setDefault("keymap_slot4", "KEY_KEY_4");
	settings->setDefault("keymap_slot5", "KEY_KEY_5");
	settings->setDefault("keymap_slot6", "KEY_KEY_6");
	settings->setDefault("keymap_slot7", "KEY_KEY_7");
	settings->setDefault("keymap_slot8", "KEY_KEY_8");
	settings->setDefault("keymap_slot9", "KEY_KEY_9");
	settings->setDefault("keymap_slot10", "KEY_KEY_0");
	settings->setDefault("keymap_slot11", "");
	settings->setDefault("keymap_slot12", "");
	settings->setDefault("keymap_slot13", "");
	settings->setDefault("keymap_slot14", "");
	settings->setDefault("keymap_slot15", "");
	settings->setDefault("keymap_slot16", "");
	settings->setDefault("keymap_slot17", "");
	settings->setDefault("keymap_slot18", "");
	settings->setDefault("keymap_slot19", "");
	settings->setDefault("keymap_slot20", "");
	settings->setDefault("keymap_slot21", "");
	settings->setDefault("keymap_slot22", "");
	settings->setDefault("keymap_slot23", "");
	settings->setDefault("keymap_slot24", "");
	settings->setDefault("keymap_slot25", "");
	settings->setDefault("keymap_slot26", "");
	settings->setDefault("keymap_slot27", "");
	settings->setDefault("keymap_slot28", "");
	settings->setDefault("keymap_slot29", "");
	settings->setDefault("keymap_slot30", "");
	settings->setDefault("keymap_slot31", "");
	settings->setDefault("keymap_slot32", "");

#ifndef NDEBUG
	// Default keybinds for quicktune in debug builds
	settings->setDefault("keymap_quicktune_prev", "KEY_HOME");
	settings->setDefault("keymap_quicktune_next", "KEY_END");
	settings->setDefault("keymap_quicktune_dec", "KEY_NEXT");
	settings->setDefault("keymap_quicktune_inc", "KEY_PRIOR");
#else
	settings->setDefault("keymap_quicktune_prev", "");
	settings->setDefault("keymap_quicktune_next", "");
	settings->setDefault("keymap_quicktune_dec", "");
	settings->setDefault("keymap_quicktune_inc", "");
#endif

	// Visuals
#ifdef NDEBUG
	settings->setDefault("show_debug", "false");
	settings->setDefault("opengl_debug", "false");
#else
	settings->setDefault("show_debug", "true");
	settings->setDefault("opengl_debug", "true");
#endif
	settings->setDefault("fsaa", "2");
	settings->setDefault("undersampling", "1");
	settings->setDefault("world_aligned_mode", "enable");
	settings->setDefault("autoscale_mode", "disable");
	settings->setDefault("texture_min_size", "64");
	settings->setDefault("enable_fog", "true");
	settings->setDefault("fog_start", "0.4");
	settings->setDefault("3d_mode", "none");
	settings->setDefault("3d_paralax_strength", "0.025");
	settings->setDefault("tooltip_show_delay", "400");
	settings->setDefault("tooltip_append_itemname", "false");
	settings->setDefault("fps_max", "60");
	settings->setDefault("fps_max_unfocused", "20");
	settings->setDefault("viewing_range", "190");
	settings->setDefault("client_mesh_chunk", "1");
	settings->setDefault("screen_w", "1024");
	settings->setDefault("screen_h", "600");
	settings->setDefault("window_maximized", "false");
	settings->setDefault("autosave_screensize", "true");
	settings->setDefault("fullscreen", bool_to_cstr(has_touch));
	settings->setDefault("vsync", "false");
	settings->setDefault("fov", "72");
	settings->setDefault("leaves_style", "fancy");
	settings->setDefault("connected_glass", "false");
	settings->setDefault("smooth_lighting", "true");
	settings->setDefault("performance_tradeoffs", "false");
	settings->setDefault("lighting_alpha", "0.0");
	settings->setDefault("lighting_beta", "1.5");
	settings->setDefault("display_gamma", "1.0");
	settings->setDefault("lighting_boost", "0.2");
	settings->setDefault("lighting_boost_center", "0.5");
	settings->setDefault("lighting_boost_spread", "0.2");
	settings->setDefault("texture_path", "");
	settings->setDefault("shader_path", "");
	settings->setDefault("video_driver", "");
	settings->setDefault("cinematic", "false");
	settings->setDefault("camera_smoothing", "0.0");
	settings->setDefault("cinematic_camera_smoothing", "0.7");
	settings->setDefault("enable_clouds", "true");
	settings->setDefault("view_bobbing_amount", "1.0");
	settings->setDefault("fall_bobbing_amount", "0.03");
	settings->setDefault("enable_3d_clouds", "true");
	settings->setDefault("cloud_radius", "12");
	settings->setDefault("menu_clouds", "true");
	settings->setDefault("translucent_liquids", "true");
	settings->setDefault("console_height", "0.6");
	settings->setDefault("console_color", "(0,0,0)");
	settings->setDefault("console_alpha", "200");
	settings->setDefault("formspec_fullscreen_bg_color", "(0,0,0)");
	settings->setDefault("formspec_fullscreen_bg_opacity", "140");
	settings->setDefault("selectionbox_color", "(0,0,0)");
	settings->setDefault("selectionbox_width", "2");
	settings->setDefault("node_highlighting", "box");
	settings->setDefault("crosshair_color", "(255,255,255)");
	settings->setDefault("crosshair_alpha", "255");
	settings->setDefault("recent_chat_messages", "6");
	settings->setDefault("hud_scaling", "1.0");
	settings->setDefault("gui_scaling", "1.0");
	settings->setDefault("gui_scaling_filter", "false");
	settings->setDefault("gui_scaling_filter_txr2img", "true");
	settings->setDefault("smooth_scrolling", "true");
	settings->setDefault("desynchronize_mapblock_texture_animation", "false");
	settings->setDefault("hud_hotbar_max_width", "1.0");
	settings->setDefault("enable_local_map_saving", "false");
	settings->setDefault("show_entity_selectionbox", "false");
	settings->setDefault("ambient_occlusion_gamma", "1.8");
	settings->setDefault("enable_shaders", "true");
	settings->setDefault("enable_particles", "true");
	settings->setDefault("arm_inertia", "true");
	settings->setDefault("show_nametag_backgrounds", "true");
	settings->setDefault("show_block_bounds_radius_near", "4");
	settings->setDefault("transparency_sorting_distance", "16");

	settings->setDefault("enable_minimap", "true");
	settings->setDefault("minimap_shape_round", "true");
	settings->setDefault("minimap_double_scan_height", "true");

	// Effects
	settings->setDefault("enable_post_processing", "true");
	settings->setDefault("directional_colored_fog", "true");
	settings->setDefault("inventory_items_animations", "false");
	settings->setDefault("mip_map", "false");
	settings->setDefault("bilinear_filter", "false");
	settings->setDefault("trilinear_filter", "false");
	settings->setDefault("anisotropic_filter", "false");
	settings->setDefault("tone_mapping", "false");
	settings->setDefault("enable_waving_water", "false");
	settings->setDefault("water_wave_height", "1.0");
	settings->setDefault("water_wave_length", "20.0");
	settings->setDefault("water_wave_speed", "5.0");
	settings->setDefault("enable_waving_leaves", "false");
	settings->setDefault("enable_waving_plants", "false");
	settings->setDefault("exposure_compensation", "0.0");
	settings->setDefault("enable_auto_exposure", "false");
	settings->setDefault("debanding", "true");
	settings->setDefault("antialiasing", "none");
	settings->setDefault("enable_bloom", "false");
	settings->setDefault("enable_bloom_debug", "false");
	settings->setDefault("bloom_strength_factor", "1.0");
	settings->setDefault("bloom_intensity", "0.05");
	settings->setDefault("bloom_radius", "1");
	settings->setDefault("enable_volumetric_lighting", "false");

	// Effects Shadows
	settings->setDefault("enable_dynamic_shadows", "false");
	settings->setDefault("shadow_strength_gamma", "1.0");
	settings->setDefault("shadow_map_max_distance", "140.0");
	settings->setDefault("shadow_map_texture_size", "2048");
	settings->setDefault("shadow_map_texture_32bit", "true");
	settings->setDefault("shadow_map_color", "false");
	settings->setDefault("shadow_filters", "1");
	settings->setDefault("shadow_poisson_filter", "true");
	settings->setDefault("shadow_update_frames", "8");
	settings->setDefault("shadow_soft_radius", "5.0");
	settings->setDefault("shadow_sky_body_orbit_tilt", "0.0");

	// Input
	settings->setDefault("invert_mouse", "false");
	settings->setDefault("enable_hotbar_mouse_wheel", "true");
	settings->setDefault("invert_hotbar_mouse_wheel", "false");
	settings->setDefault("mouse_sensitivity", "0.2");
	settings->setDefault("repeat_place_time", "0.25");
	settings->setDefault("repeat_dig_time", "0.0");
	settings->setDefault("safe_dig_and_place", "false");
	settings->setDefault("random_input", "false");
	settings->setDefault("aux1_descends", "false");
	settings->setDefault("doubletap_jump", "false");
	settings->setDefault("always_fly_fast", "true");
	settings->setDefault("autojump", bool_to_cstr(has_touch));
	settings->setDefault("continuous_forward", "false");
	settings->setDefault("enable_joysticks", "false");
	settings->setDefault("joystick_id", "0");
	settings->setDefault("joystick_type", "auto");
	settings->setDefault("repeat_joystick_button_time", "0.17");
	settings->setDefault("joystick_frustum_sensitivity", "170");
	settings->setDefault("joystick_deadzone", "2048");

	// Main menu
	settings->setDefault("main_menu_path", "");
	settings->setDefault("serverlist_file", "favoriteservers.json");

	// General font settings
	settings->setDefault("font_path", porting::getDataPath("fonts" DIR_DELIM "Arimo-Regular.ttf"));
	settings->setDefault("font_path_hd", porting::getDataPath("fonts" DIR_DELIM "hd.ttf"));
	settings->setDefault("font_path_italic", porting::getDataPath("fonts" DIR_DELIM "Arimo-Italic.ttf"));
	settings->setDefault("font_path_bold", porting::getDataPath("fonts" DIR_DELIM "Arimo-Bold.ttf"));
	settings->setDefault("font_path_bold_italic", porting::getDataPath("fonts" DIR_DELIM "Arimo-BoldItalic.ttf"));
	settings->setDefault("font_bold", "false");
	settings->setDefault("font_italic", "false");
	settings->setDefault("font_shadow", "1");
	settings->setDefault("font_shadow_alpha", "127");
	settings->setDefault("font_size_divisible_by", "1");
	settings->setDefault("mono_font_path", porting::getDataPath("fonts" DIR_DELIM "Cousine-Regular.ttf"));
	settings->setDefault("mono_font_path_italic", porting::getDataPath("fonts" DIR_DELIM "Cousine-Italic.ttf"));
	settings->setDefault("mono_font_path_bold", porting::getDataPath("fonts" DIR_DELIM "Cousine-Bold.ttf"));
	settings->setDefault("mono_font_path_bold_italic", porting::getDataPath("fonts" DIR_DELIM "Cousine-BoldItalic.ttf"));
	settings->setDefault("mono_font_size_divisible_by", "1");
	settings->setDefault("fallback_font_path", porting::getDataPath("fonts" DIR_DELIM "DroidSansFallbackFull.ttf"));

	std::string font_size_str = std::to_string(TTF_DEFAULT_FONT_SIZE);
	settings->setDefault("font_size", font_size_str);
	settings->setDefault("mono_font_size", font_size_str);
    settings->setDefault("hd_font_size", font_size_str);
	settings->setDefault("chat_font_size", "0"); // Default "font_size"
	// ContentDB
	settings->setDefault("contentdb_url", "https://content.minetest.net");
	settings->setDefault("contentdb_enable_updates_indicator", "true");
	settings->setDefault("contentdb_max_concurrent_downloads", "3");

#ifdef __ANDROID__
	settings->setDefault("contentdb_flag_blacklist", "nonfree, android_default");
#else
	settings->setDefault("contentdb_flag_blacklist", "nonfree, desktop_default");
#endif

#if ENABLE_UPDATE_CHECKER
	settings->setDefault("update_information_url", "https://www.minetest.net/release_info.json");
#else
	settings->setDefault("update_information_url", "");
#endif

	// Server
	settings->setDefault("disable_escape_sequences", "false");
	settings->setDefault("strip_color_codes", "false");
#ifndef NDEBUG
	settings->setDefault("random_mod_load_order", "true");
#else
	settings->setDefault("random_mod_load_order", "false");
#endif
#if USE_PROMETHEUS
	settings->setDefault("prometheus_listener_address", "127.0.0.1:30000");
#endif

	// Network
	settings->setDefault("enable_ipv6", "true");
	settings->setDefault("ipv6_server", "false");
	settings->setDefault("max_packets_per_iteration", "1024");
	settings->setDefault("port", "30000");
	settings->setDefault("strict_protocol_version_checking", "false");
	settings->setDefault("protocol_version_min", "1");
	settings->setDefault("player_transfer_distance", "0");
	settings->setDefault("max_simultaneous_block_sends_per_client", "40");
	settings->setDefault("time_send_interval", "5");

	settings->setDefault("motd", "");
	settings->setDefault("max_users", "15");
	settings->setDefault("creative_mode", "false");
	settings->setDefault("enable_damage", "true");
	settings->setDefault("default_password", "");
	settings->setDefault("default_privs", "interact, shout");
	settings->setDefault("enable_pvp", "true");
	settings->setDefault("enable_mod_channels", "false");
	settings->setDefault("disallow_empty_password", "false");
	settings->setDefault("disable_anticheat", "false");
	settings->setDefault("enable_rollback_recording", "false");
	settings->setDefault("deprecated_lua_api_handling", "log");

	settings->setDefault("kick_msg_shutdown", "Server shutting down.");
	settings->setDefault("kick_msg_crash", "This server has experienced an internal error. You will now be disconnected.");
	settings->setDefault("ask_reconnect_on_crash", "false");

	settings->setDefault("chat_message_format", "<@name> @message");
	settings->setDefault("profiler_print_interval", "0");
	settings->setDefault("active_object_send_range_blocks", "8");
	settings->setDefault("active_block_range", "4");
	//settings->setDefault("max_simultaneous_block_sends_per_client", "1");
	// This causes frametime jitter on client side, or does it?
	settings->setDefault("max_block_send_distance", "12");
	settings->setDefault("block_send_optimize_distance", "4");
	settings->setDefault("block_cull_optimize_distance", "25");
	settings->setDefault("server_side_occlusion_culling", "true");
	settings->setDefault("csm_restriction_flags", "0"); //62
	settings->setDefault("csm_restriction_noderange", "0");
	settings->setDefault("max_clearobjects_extra_loaded_blocks", "4096");
	settings->setDefault("time_speed", "72");
	settings->setDefault("world_start_time", "6125");
	settings->setDefault("server_unload_unused_data_timeout", "29");
	settings->setDefault("max_objects_per_block", "256");
	settings->setDefault("server_map_save_interval", "5.3");
	settings->setDefault("chat_message_max_size", "500");
	settings->setDefault("chat_message_limit_per_10sec", "8.0");
	settings->setDefault("chat_message_limit_trigger_kick", "50");
	settings->setDefault("sqlite_synchronous", "2");
	settings->setDefault("map_compression_level_disk", "-1");
	settings->setDefault("map_compression_level_net", "-1");
	settings->setDefault("full_block_send_enable_min_time_from_building", "2.0");
	settings->setDefault("dedicated_server_step", "0.09");
	settings->setDefault("active_block_mgmt_interval", "2.0");
	settings->setDefault("abm_interval", "1.0");
	settings->setDefault("abm_time_budget", "0.2");
	settings->setDefault("nodetimer_interval", "0.2");
	settings->setDefault("ignore_world_load_errors", "false");
	settings->setDefault("remote_media", "");
	settings->setDefault("debug_log_level", "action");
	settings->setDefault("debug_log_size_max", "50");
	settings->setDefault("chat_log_level", "error");
	settings->setDefault("emergequeue_limit_total", "1024");
	settings->setDefault("emergequeue_limit_diskonly", "128");
	settings->setDefault("emergequeue_limit_generate", "128");
	settings->setDefault("num_emerge_threads", "1");
	settings->setDefault("secure.enable_security", "true");
	settings->setDefault("secure.trusted_mods", "");
	settings->setDefault("secure.http_mods", "");

	// Physics
	settings->setDefault("movement_acceleration_default", "3");
	settings->setDefault("movement_acceleration_air", "2");
	settings->setDefault("movement_acceleration_fast", "10");
	settings->setDefault("movement_speed_walk", "4");
	settings->setDefault("movement_speed_crouch", "4");
	settings->setDefault("movement_speed_fast", "20");
	settings->setDefault("movement_speed_climb", "4");
	settings->setDefault("movement_speed_jump", "6.5");
	settings->setDefault("movement_liquid_fluidity", "1");
	settings->setDefault("movement_liquid_fluidity_smooth", "0.5");
	settings->setDefault("movement_liquid_sink", "10");
	settings->setDefault("movement_gravity", "9.81");

	// Liquids
	settings->setDefault("liquid_loop_max", "100000");
	settings->setDefault("liquid_queue_purge_time", "0");
	settings->setDefault("liquid_update", "1.0");

	// Mapgen
	settings->setDefault("mg_name", "v7");
	settings->setDefault("water_level", "1");
	settings->setDefault("mapgen_limit", "31007");
	settings->setDefault("chunksize", "5");
	settings->setDefault("fixed_map_seed", "");
	settings->setDefault("max_block_generate_distance", "10");
	settings->setDefault("enable_mapgen_debug_info", "false");
	Mapgen::setDefaultSettings(settings);

	// Server list announcing
	settings->setDefault("server_announce", "false");
	settings->setDefault("server_url", "");
	settings->setDefault("server_address", "");
	settings->setDefault("server_name", "");
	settings->setDefault("server_description", "");
	settings->setDefault("server_announce_send_players", "true");

	settings->setDefault("enable_console", "false");
	settings->setDefault("display_density_factor", "1");
	settings->setDefault("dpi_change_notifier", "0");

	// Altered settings for CIrrDeviceOSX
#if !USE_SDL2 && defined(__MACH__) && defined(__APPLE__)
	settings->setDefault("keymap_sneak", "KEY_SHIFT");
#endif

	settings->setDefault("touchscreen_sensitivity", "0.2");
	settings->setDefault("touchscreen_threshold", "20");
	settings->setDefault("touch_long_tap_delay", "400");
	settings->setDefault("touch_use_crosshair", "false");
	settings->setDefault("fixed_virtual_joystick", "false");
	settings->setDefault("virtual_joystick_triggers_aux1", "false");
	settings->setDefault("touch_punch_gesture", "short_tap");
	settings->setDefault("clickable_chat_weblinks", "true");
	// Altered settings for Android
#ifdef __ANDROID__
	settings->setDefault("screen_w", "0");
	settings->setDefault("screen_h", "0");
	settings->setDefault("performance_tradeoffs", "true");
	settings->setDefault("max_simultaneous_block_sends_per_client", "10");
	settings->setDefault("emergequeue_limit_diskonly", "16");
	settings->setDefault("emergequeue_limit_generate", "16");
	settings->setDefault("max_block_generate_distance", "5");
	settings->setDefault("sqlite_synchronous", "1");
	settings->setDefault("server_map_save_interval", "15");
	settings->setDefault("client_mapblock_limit", "1000");
	settings->setDefault("active_block_range", "2");
	settings->setDefault("viewing_range", "50");
	settings->setDefault("leaves_style", "simple");
	settings->setDefault("enable_post_processing", "false");
	settings->setDefault("debanding", "false");
	settings->setDefault("curl_verify_cert", "false");

	// Apply settings according to screen size
	float x_inches = (float) porting::getDisplaySize().X /
			(160.f * porting::getDisplayDensity());

	if (x_inches < 3.7f) {
		settings->setDefault("hud_scaling", "0.6");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	} else if (x_inches < 4.5f) {
		settings->setDefault("hud_scaling", "0.7");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	} else if (x_inches < 6.0f) {
		settings->setDefault("hud_scaling", "0.85");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	}
	// Tablets >= 6.0 use non-Android defaults for these settings
#endif
}
