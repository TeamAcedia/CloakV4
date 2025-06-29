#include "gui/targetHUD.h"

TargetHUD::TargetHUD(const core::rect<s32>& rect) : CheatUIElement(rect) {}


double TargetHUD::getInterpolatedHealth(const GenericCAO *obj, float dtime) {
	ActiveObject::object_t id = obj->getId();

	auto it = m_interpolated_entity_health.find(id);

	double currentHp = static_cast<double>(obj->getHp());

	if (it != m_interpolated_entity_health.end()) {
		double interpolatedHealth = it->second;

		double healthDifference = currentHp - interpolatedHealth;

		double change = healthDifference * (dtime * 8);

		interpolatedHealth += change;

		m_interpolated_entity_health[id] = interpolatedHealth;

		return interpolatedHealth;
	} else {
		m_interpolated_entity_health[id] = currentHp;
		return currentHp;
	}
}

void drawTargetHud(video::IVideoDriver* driver, gui::IGUIFont* font, double interpolated_health, u16 hp_max, core::rect<s32> bounds, std::wstring title) {
	const video::SColor outline_color = video::SColor(255, 0, 0, 0);
	const video::SColor background_color = video::SColor(255, 25, 25, 25);
	const video::SColor text_color = video::SColor(255, 255, 255, 255);

	s32 vertical_padding = s32(bounds.getHeight()/8);
	s32 horizontal_padding = s32(bounds.getWidth()/16);

	driver->draw2DRectangle(background_color, bounds);
	driver->draw2DRectangleOutline(bounds, outline_color, 3);

	double health_percentage = hp_max > 0 ? interpolated_health / hp_max : 0.0;
	health_percentage = std::max(0.0, std::min(1.0, health_percentage));

	u8 red = static_cast<u8>(255 * (1.0f - health_percentage));
	u8 green = static_cast<u8>(255 * health_percentage);
	video::SColor health_filled_color(255, red, green, 0);

	core::rect<s32> health_bar_rect(
		bounds.UpperLeftCorner.X + horizontal_padding,
		bounds.UpperLeftCorner.Y + vertical_padding * 5,
		bounds.LowerRightCorner.X - horizontal_padding,
		bounds.LowerRightCorner.Y - vertical_padding
	);

	s32 filled_width = static_cast<s32>(health_bar_rect.getWidth() * health_percentage);

	core::rect<s32> health_bar_filled_rect(
		health_bar_rect.UpperLeftCorner.X,
		health_bar_rect.UpperLeftCorner.Y,
		health_bar_rect.UpperLeftCorner.X + filled_width,
		health_bar_rect.LowerRightCorner.Y
	);

	core::rect<s32> text_rect(
		bounds.UpperLeftCorner.X + horizontal_padding,
		bounds.UpperLeftCorner.Y + vertical_padding,
		bounds.LowerRightCorner.X - horizontal_padding,
		health_bar_rect.UpperLeftCorner.Y - vertical_padding
	);

	driver->draw2DRectangle(health_filled_color, health_bar_filled_rect);
	driver->draw2DRectangleOutline(health_bar_filled_rect, outline_color, 2);
	driver->draw2DRectangleOutline(health_bar_rect, outline_color, 2);

    core::dimension2d<u32> textSizeU32 = font->getDimension(title.c_str());
    core::dimension2d<s32> textSize(textSizeU32.Width, textSizeU32.Height);

    s32 textX = text_rect.UpperLeftCorner.X + (text_rect.getWidth() - textSize.Width) / 2;
    s32 textY = text_rect.UpperLeftCorner.Y + (text_rect.getHeight() - textSize.Height) / 2;

    font->draw(title.c_str(), core::rect<s32>(textX, textY, textX + textSize.Width, textY + textSize.Height), text_color);
}

void TargetHUD::draw(video::IVideoDriver* driver, gui::IGUIFont* font, float dtime, ClientEnvironment &env, bool editing) {
	if (g_settings->getBool("enable_combat_target_hud")) {
		if (editing) {
			drawTargetHud(driver, font, 10, 20, bounds, L"Target");
		} else {
			std::unordered_map<u16, ClientActiveObject*> allObjects;
	
			env.getAllActiveObjectsLegacy(allObjects);
	
			for (auto &ao_it : allObjects) {
				ClientActiveObject *cao = ao_it.second;
				GenericCAO *obj = dynamic_cast<GenericCAO *>(cao);
				
				double interpolated_health = TargetHUD::getInterpolatedHealth(obj, dtime); // unoptimized workaround for interpolation not being stepped if it isn't a valid target.
	
				if (!obj || obj->getId() != RenderingCore::combat_target || RenderingCore::combat_target == NULL)
					continue;

				drawTargetHud(driver, font, interpolated_health, obj->getProperties().hp_max, bounds, std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(obj->getName()));
			}
		}
	}
}
