#include <Geode/modify/MenuLayer.hpp>
#include "Manager.hpp"

#define getBool Mod::get()->getSettingValue<bool>
#define manager Manager::getSharedInstance()

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
	void forceEnableJustDont() {
		auto gm = GameManager::sharedState();
		if (!gm->getGameVariable("0095")) {
			gm->setGameVariable("0095", true);
			Mod::get()->setSettingValue("checkJustDont", false);
			log::info("\"Just Dont\"/\"Do Not\" check complete.");
		}
	}
	bool init() {
		if (!MenuLayer::init()) return false;
		if (!manager->completedJDDNCheck && getBool("enabled") && getBool("checkJustDont")) {
			MyMenuLayer::forceEnableJustDont();
		}
		manager->completedJDDNCheck = true;
		manager->hasNextKeyWhenLoaded = Loader::get()->isModLoaded("alphalaneous.next_key_when");
		manager->hasPRNTSCRN = Loader::get()->isModLoaded("ninxout.prntscrn");
		return true;
	}
};