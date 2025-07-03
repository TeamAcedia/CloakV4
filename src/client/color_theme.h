#pragma once

#include <vector>
#include <string>
#include "irrlichttypes_bloated.h"

class ColorTheme {
public:
	std::string name;

	video::SColor background_top;
	video::SColor background;
	video::SColor background_bottom;

	video::SColor border;

	video::SColor text;
	video::SColor text_muted;

	video::SColor primary;
	video::SColor primary_muted;

	video::SColor secondary;
	video::SColor secondary_muted;

	ColorTheme() = default;
	explicit ColorTheme(const std::string &data);
};

class ThemeManager {
public:
	// Load all .theme files in a folder
	void LoadThemes(const std::string &folderpath);

	// Return available theme names
	std::vector<std::string> GetThemes() const;

	// Get a theme by name (returns a placeholder theme if not found)
	ColorTheme GetThemeByName(const std::string &name) const;

private:
	std::vector<ColorTheme> themes;
};
