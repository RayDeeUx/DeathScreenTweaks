#include <Geode/modify/FLAlertLayer.hpp>

#define PREFERRED_HOOK_PRIO (-2123456789)

using namespace geode::prelude;class $modify(MyFLAlertLayer, FLAlertLayer) {
	static void onModify(auto & self)
	{
		(void) self.setHookPriority("FLAlertLayer::init", PREFERRED_HOOK_PRIO);
	}
	bool init(FLAlertLayerProtocol* delegate, char const* title, gd::string desc, char const* btn1, char const* btn2, float width, bool scroll, float height, float textScale) {
		const std::string& titleAsString = title;
		const std::string& descAsString = desc;
		if (titleAsString != "\"New Best\" Text Font #" && !utils::string::contains(descAsString, "To use font numbers greater than 0: refer to the level editor for the font each number corresponds to, subtract that number by 1, and then put it here.") && !utils::string::contains(descAsString, "suggested by Saritahh") && !utils::string::contains(descAsString, "If using Oxygene One")) {
			return FLAlertLayer::init(delegate, title, desc, btn1, btn2, width, scroll, height, textScale);
		}
		return FLAlertLayer::init(delegate, title, desc, btn1, btn2, 420.f, true, 320.f, 1.0f);
	}
};