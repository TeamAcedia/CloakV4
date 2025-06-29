#include "gui/coordsHUD.h"

coordsHUD::coordsHUD(const core::rect<s32>& rect) : CheatUIElement(rect) {}

void coordsHUD::draw(video::IVideoDriver* driver, gui::IGUIFont* font, float dtime, ClientEnvironment &env, bool editing) {
    if (g_settings->getBool("coords") || editing) {
        const video::SColor outline_color = video::SColor(255, 0, 0, 0);
        const video::SColor background_color = video::SColor(255, 25, 25, 25);
        const video::SColor text_color = video::SColor(255, 255, 255, 255);

        driver->draw2DRectangle(background_color, bounds);
        driver->draw2DRectangleOutline(bounds, outline_color, 3);

        v3f coords = env.getLocalPlayer()->getPosition();

        // Convert v3f coordinates to a wstring with 1 decimal place
        std::wstringstream wss;
        wss << std::fixed << std::setprecision(1);  // Ensures one decimal place
        wss << L"X: " << coords.X / 10 << L" Y: " << coords.Y / 10 << L" Z: " << coords.Z / 10;

        std::wstring coords_wstr = wss.str();

		core::dimension2d<u32> textSizeU32 = font->getDimension(coords_wstr.c_str());
		core::dimension2d<s32> textSize(textSizeU32.Width, textSizeU32.Height);
	
		s32 textX = bounds.UpperLeftCorner.X + (bounds.getWidth() - textSize.Width) / 2;
		s32 textY = bounds.UpperLeftCorner.Y + (bounds.getHeight() - textSize.Height) / 2;
	
		font->draw(coords_wstr.c_str(), core::rect<s32>(textX, textY, textX + textSize.Width, textY + textSize.Height), text_color);
    }
}