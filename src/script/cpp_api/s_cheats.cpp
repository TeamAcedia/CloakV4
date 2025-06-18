/*
Dragonfire
Copyright (C) 2020 Elias Fleckenstein <eliasfleckenstein@web.de>

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

#include "cpp_api/s_cheats.h"
#include "cpp_api/s_base.h"
#include "cpp_api/s_internal.h"
#include "settings.h"
#include "log.h"
#include <iostream>
#include "s_cheats.h"

ScriptApiCheatsCheat::ScriptApiCheatsCheat(
		const std::string &name, const std::string &setting, const std::string &info_text = "") :
		m_name(name),
		m_setting(setting), m_function_ref(0),
		m_info_text(info_text)
{
}

ScriptApiCheatsCheat::ScriptApiCheatsCheat(const std::string &name, const int &function, const std::string &info_text = "") :
		m_name(name), m_setting(""), m_function_ref(function)
{
}

ScriptApiCheatsCheat::~ScriptApiCheatsCheat()
{
	for (int i = 0; i < m_cheat_settings.size(); i++) 
		delete m_cheat_settings[i];
}

void ScriptApiCheatsCheat::set_info_text(std::string infoText) {
	m_info_text = infoText;
}

std::string ScriptApiCheatsCheat::get_info_text() {
	return m_info_text;
}

bool ScriptApiCheatsCheat::is_enabled()
{
	try {
		return !m_function_ref && g_settings->getBool(m_setting);
	} catch (SettingNotFoundException &) {
		return false;
	}
}

void ScriptApiCheatsCheat::toggle(lua_State *L, int error_handler)
{
	if (m_function_ref) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, m_function_ref);
		lua_pcall(L, 0, 0, error_handler);
	} else
		g_settings->setBool(m_setting, !is_enabled());
}

bool ScriptApiCheatsCheat::has_settings()
{
	return m_cheat_settings.size() > 0;
}

ScriptApiCheatsCategory::ScriptApiCheatsCategory(const std::string &name) : m_name(name)
{
}

ScriptApiCheatsCategory::~ScriptApiCheatsCategory()
{
	for (int i = 0; i < m_cheats.size(); i++) 
		delete m_cheats[i];
}

void ScriptApiCheatsCategory::read_cheats(lua_State *L)
{
	std::vector<std::pair<std::string, ScriptApiCheatsCheat*>> cheat_pairs;

	lua_pushnil(L);
	while (lua_next(L, -2)) {
		ScriptApiCheatsCheat *cheat = nullptr;
		std::string name = lua_tostring(L, -2);
		if (lua_isstring(L, -1))
			cheat = new ScriptApiCheatsCheat(name, lua_tostring(L, -1));
		else if (lua_isfunction(L, -1)) {
			cheat = new ScriptApiCheatsCheat(
					name, luaL_ref(L, LUA_REGISTRYINDEX));
			lua_pushnil(L);
		}
		if (cheat)
			cheat_pairs.emplace_back(name, cheat);
		lua_pop(L, 1);
	}

	// Sorting cheats
	std::sort(cheat_pairs.begin(), cheat_pairs.end());

	// Assigning sorted cheats to m_cheats
	for (const auto &pair : cheat_pairs) {
		m_cheats.push_back(pair.second);
	}
}

ScriptApiCheatsCheat *ScriptApiCheatsCategory::get_cheat_by_id(const std::string &setting_id)
{
	for (auto& cheat : m_cheats) {
        if (cheat->m_setting == setting_id) 
            return cheat;
    }
    return nullptr;
}

ScriptApiCheats::~ScriptApiCheats()
{
	for (auto i = m_cheat_categories.begin(); i != m_cheat_categories.end(); i++)
		delete *i;
}


void ScriptApiCheats::update_infotexts()
{
	SCRIPTAPI_PRECHECKHEADER

    lua_getglobal(L, "core");

	// Fetch 'core.infotexts'
	lua_getfield(L, -1, "infotexts");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 2); // Pop 'core.infotexts' and 'core'
		return;
	}

	// Update info texts for existing cheats
	for (ScriptApiCheatsCategory *category : m_cheat_categories) {
		lua_getfield(L, -1, category->m_name.c_str()); // Fetch category table
		if (lua_istable(L, -1)) {
			for (auto &cheat : category->m_cheats) {
				lua_getfield(L, -1, cheat->m_name.c_str()); // Fetch cheat info text
				if (lua_isstring(L, -1)) {
					cheat->set_info_text(lua_tostring(L, -1)); // Update info text
				}

				lua_pop(L, 1); // Pop cheat info text
			}
		}
		lua_pop(L, 1); // Pop category table
	}

	lua_pop(L, 2); // Pop 'core.infotexts' and 'core'
}

void ScriptApiCheats::get_description()
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");

	lua_getfield(L, -1, "descriptions");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 2);
		return;
	}

	for (ScriptApiCheatsCategory *category : m_cheat_categories) {
		lua_getfield(L, -1, category->m_name.c_str());
		if (lua_istable(L, -1)) {
			for (auto &cheat : category->m_cheats) {
				lua_getfield(L, -1, cheat->m_name.c_str());
				if (lua_isstring(L, -1)) {
					cheat->m_description = lua_tostring(L, -1);
				}

				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 2);
}

void ScriptApiCheats::init_cheat_settings()
{
    SCRIPTAPI_PRECHECKHEADER
    
    lua_getglobal(L, "core");
    lua_getfield(L, -1, "cheat_settings");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 2);
        warningstream << "No cheat_settings found in core" << std::endl;
        return;
    }

    // Iterate over the first level (categories)
    lua_pushnil(L); // Start iteration
    while (lua_next(L, -2) != 0) {
        if (lua_isstring(L, -2)) {
            const char *category_name = lua_tostring(L, -2);
			ScriptApiCheatsCategory* category = get_category(category_name);
			if (category == nullptr) {
				warningstream << "CheatMenuSettings: Invalid category: " << category_name << std::endl;
				lua_pop(L, 1);
				continue;
			}
            if (lua_istable(L, -1)) {
                // Iterate over the second level (settings)
                lua_pushnil(L);
                while (lua_next(L, -2) != 0) {
                    if (lua_isstring(L, -2)) {
                        const char *parent_name = lua_tostring(L, -2);

                        if (lua_istable(L, -1)) {

							lua_pushnil(L);
							while (lua_next(L, -2) != 0) {
                    			if (lua_isstring(L, -2)) {
									const char *setting_id = lua_tostring(L, -2);
									const char *setting_name = nullptr;
									std::vector<std::string *> setting_options;
									
									lua_getfield(L, -1, "name"); 
									if (lua_isstring(L, -1)) {
										setting_name = lua_tostring(L, -1);
									}
									lua_pop(L, 1); // Pop 'name'

									ScriptApiCheatsCheatSetting* cheat_setting = new ScriptApiCheatsCheatSetting(setting_name, setting_id);

									lua_getfield(L, -1, "type");
									if (lua_isstring(L, -1)) {
										cheat_setting->m_type = lua_tostring(L, -1);
									}
									lua_pop(L, 1); // Pop 'type'

									lua_getfield(L, -1, "min");
									if (lua_isnumber(L, -1)) {
										cheat_setting->m_min = lua_tonumber(L, -1);
									}
									lua_pop(L, 1); // Pop 'min'

									lua_getfield(L, -1, "max");  // get 'max'
									if (lua_isnumber(L, -1)) {
										cheat_setting->m_max = lua_tonumber(L, -1);
									}
									lua_pop(L, 1); // Pop 'max'

									lua_getfield(L, -1, "steps");  // get 'steps'
									if (lua_isnumber(L, -1)) {
										cheat_setting->m_steps = lua_tonumber(L, -1);
									}
									lua_pop(L, 1); // Pop 'steps'

									lua_getfield(L, -1, "size");  // get 'size'
									if (lua_isnumber(L, -1)) {
										cheat_setting->m_size = lua_tonumber(L, -1);
									}
									lua_pop(L, 1); // Pop 'size'

									lua_getfield(L, -1, "options");  // get 'options'
									if (lua_istable(L, -1)) {
										lua_pushnil(L);
										while (lua_next(L, -2) != 0) {
											if (lua_isstring(L, -1)) {
												cheat_setting->m_options.push_back(new std::string(lua_tostring(L, -1)));
											}
											lua_pop(L, 1); // Pop value (string)
										}
									}
                        			lua_pop(L, 2); // Pop 'options' and setting table
									cheat_setting->m_parent = parent_name;
									category->get_cheat_by_id(parent_name)->m_cheat_settings.push_back(cheat_setting);
								}
							}
                            
                        } else {
							warningstream << "CheatSettings: Unknown data type " << std::endl;
						}
                        lua_pop(L, 1); // Pop cheat table
                    }
                }
            }
            lua_pop(L, 1); // Pop category table
        }
    }

    lua_pop(L, 2); // Pop 'core.cheat_settings' and 'core'
}


void ScriptApiCheats::init_cheats()
{
    SCRIPTAPI_PRECHECKHEADER

    lua_getglobal(L, "core");
    lua_getfield(L, -1, "cheats");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 2);
        return;
    }
	
    // Collecting keys in a vector
    std::vector<std::string> keys;
    lua_pushnil(L);  // First key
    while (lua_next(L, -2) != 0) {
        if (lua_isstring(L, -2)) {
            keys.push_back(lua_tostring(L, -2));
        }
        lua_pop(L, 1);
    }

    // Sorting keys
    std::sort(keys.begin(), keys.end());

    // Iterate over sorted keys
    for (const auto &key : keys) {
        lua_pushstring(L, key.c_str());
        lua_gettable(L, -2);
        if (lua_istable(L, -1)) {
            ScriptApiCheatsCategory *category = new ScriptApiCheatsCategory(key.c_str());
            category->read_cheats(L);
            m_cheat_categories.push_back(category);
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 2);
    m_cheats_loaded = true;
}

void ScriptApiCheats::toggle_cheat(ScriptApiCheatsCheat *cheat)
{
	SCRIPTAPI_PRECHECKHEADER

	PUSH_ERROR_HANDLER(L);
	int error_handler = lua_gettop(L) - 1;
	lua_insert(L, error_handler);

	cheat->toggle(L, error_handler);
}

ScriptApiCheatsCategory* ScriptApiCheats::get_category(const std::string &name) {
    for (auto& category : m_cheat_categories) {
        if (category->m_name == name) 
            return category;
    }
    return nullptr;
}

ScriptApiCheatsCheatSetting::ScriptApiCheatsCheatSetting(const std::string &name, const std::string &setting) : 
	m_name(name),
	m_setting(setting)
{
}

ScriptApiCheatsCheatSetting::~ScriptApiCheatsCheatSetting()
{
	for (auto i = m_options.begin(); i != m_options.end(); i++)
		delete *i;
}

void ScriptApiCheatsCheatSetting::toggle()
{
	if (g_settings->exists(m_setting)) {
		set_value(!g_settings->getBool(m_setting));
	} else {
		set_value(true);
	}
}
void ScriptApiCheatsCheatSetting::set_value(const bool &value)
{
	g_settings->setBool(m_setting, value);
}

void ScriptApiCheatsCheatSetting::set_value(const double &value)
{
	g_settings->setFloat(m_setting, value);
}

void ScriptApiCheatsCheatSetting::set_value(const std::string &value)
{
	g_settings->set(m_setting, value);
}

void ScriptApiCheats::print_all_cheat_settings()
{
	warningstream << "Printing all cheat settings:" << std::endl;

	for (ScriptApiCheatsCategory *category : m_cheat_categories) {
		if (!category) continue;

		warningstream << "Category: " << category->m_name << std::endl;

		for (ScriptApiCheatsCheat *cheat : category->m_cheats) {
			warningstream << "  Cheat: " << cheat->m_name << std::endl;
			for (ScriptApiCheatsCheatSetting *setting : cheat->m_cheat_settings) {
				if (!setting) continue;

				warningstream << "    Cheat Setting: " << setting->m_name << std::endl;
				warningstream << "      Type: " << setting->m_type << std::endl;
				warningstream << "      Parent: " << setting->m_parent << std::endl;

				if (setting->m_type == "slider_int" || setting->m_type == "slider_float") {
					warningstream << "      Min: " << setting->m_min << std::endl;
					warningstream << "      Max: " << setting->m_max << std::endl;
					warningstream << "      Steps: " << setting->m_steps << std::endl;
				} 
				if (setting->m_type == "text") {
					warningstream << "      Size: " << setting->m_size << std::endl;
				}

				// Print options if available
				if (!setting->m_options.empty() && setting->m_type == "selectionbox") {
					warningstream << "      Options: ";
					for (std::string *option : setting->m_options) {
						if (option) {
							warningstream << *option << " ";
						}
					}
					warningstream << std::endl;
				}
			}
		}
	}

	warningstream << "Finished printing cheat settings." << std::endl;
}