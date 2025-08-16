#include "color_theme.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>

#if defined(_WIN32) && __has_include(<filesystem>)
    #include <filesystem>
    namespace fs = std::filesystem;
    #define USE_STD_FILESYSTEM
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

static std::string trim(const std::string &s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	return (start == std::string::npos || end == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

static std::string toLower(const std::string &s) {
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

static video::SColor hslToSColor(int h, int s, int l, float a = 1.0f) {
	float hf = h / 360.0f;
	float sf = s / 100.0f;
	float lf = l / 100.0f;

	auto hueToRGB = [](float p, float q, float t) {
		if (t < 0) t += 1;
		if (t > 1) t -= 1;
		if (t < 1.0f / 6) return p + (q - p) * 6 * t;
		if (t < 1.0f / 2) return q;
		if (t < 2.0f / 3) return p + (q - p) * (2.0f / 3 - t) * 6;
		return p;
	};

	float r, g, b;
	if (sf == 0) {
		r = g = b = lf;
	} else {
		float q = lf < 0.5f ? lf * (1 + sf) : lf + sf - lf * sf;
		float p = 2 * lf - q;
		r = hueToRGB(p, q, hf + 1.0f / 3);
		g = hueToRGB(p, q, hf);
		b = hueToRGB(p, q, hf - 1.0f / 3);
	}

	return video::SColor(
		static_cast<u32>(a * 255.0f),
		static_cast<u32>(r * 255.0f),
		static_cast<u32>(g * 255.0f),
		static_cast<u32>(b * 255.0f)
	);
}

static video::SColor parseHSL(const std::string &value) {
	int h = 0, s = 0, l = 0;
	float a = 1.0f;

	if (value.find("hsla") == 0)
		sscanf(value.c_str(), "hsla(%d, %d%%, %d%%, %f)", &h, &s, &l, &a);
	else
		sscanf(value.c_str(), "hsl(%d, %d%%, %d%%)", &h, &s, &l);

	return hslToSColor(h, s, l, a);
}

ColorTheme::ColorTheme(const std::string &data) {
	std::istringstream stream(data);
	std::string line;

	std::map<std::string, video::SColor*> colorMap = {
		{"background-top",    &background_top},
		{"background",        &background},
		{"background-bottom", &background_bottom},
		{"border",            &border},
		{"text",              &text},
		{"text-muted",        &text_muted},
		{"primary",           &primary},
		{"primary-muted",     &primary_muted},
		{"secondary",         &secondary},
		{"secondary-muted",   &secondary_muted},
	};

	while (std::getline(stream, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#' || line[0] == '[')
			continue;

		size_t eq = line.find('=');
		if (eq == std::string::npos)
			continue;

		std::string key = toLower(trim(line.substr(0, eq)));
		std::string value = trim(line.substr(eq + 1));

		if (key == "name") {
			name = value;
		} else if (colorMap.count(key)) {
			*colorMap[key] = parseHSL(value);
		}
	}
}

void ThemeManager::LoadThemes(const std::string &folderpath) {
    themes.clear();

#ifdef USE_STD_FILESYSTEM
    for (const auto &entry : fs::directory_iterator(folderpath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".theme")
            continue;

        std::ifstream file(entry.path());
        if (!file) continue;

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        ColorTheme theme(content);
        if (!theme.name.empty())
            themes.push_back(theme);
    }
#else
    // POSIX fallback for Linux/macOS (works on old macOS versions)
    DIR *dir = opendir(folderpath.c_str());
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..") continue;
        if (filename.size() < 6 || filename.substr(filename.size() - 6) != ".theme") continue;

        std::string fullpath = folderpath + "/" + filename;

        struct stat st;
        if (stat(fullpath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        std::ifstream file(fullpath);
        if (!file) continue;

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        ColorTheme theme(content);
        if (!theme.name.empty())
            themes.push_back(theme);
    }

    closedir(dir);
#endif
}


std::vector<std::string> ThemeManager::GetThemes() const {
	std::vector<std::string> names;
	for (const auto &theme : themes)
		names.push_back(theme.name);
	return names;
}

ColorTheme ThemeManager::GetThemeByName(const std::string &name) const {
	std::string target = toLower(name);
	for (const auto &theme : themes) {
		if (toLower(theme.name) == target)
			return theme;
	}

	// Return a default theme with all black colors and white text
	ColorTheme fallback;
	fallback.name = "Fallback Theme";
	fallback.background_top = fallback.background = fallback.background_bottom =
	fallback.border = fallback.primary = fallback.primary_muted = fallback.secondary = fallback.secondary_muted = video::SColor(255, 0, 0, 0);
	fallback.text = fallback.text_muted = video::SColor(255, 255, 255, 255);
	return fallback;
}
