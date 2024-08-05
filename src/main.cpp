#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

using namespace geode::prelude;

std::string formerCustomDeathString = "";

std::vector<std::string> quotes;
std::vector<std::string> dNBMigration;

bool completedJDDNCheck = false;

$on_mod(Loaded) {
	auto pathDefault = (Mod::get()->getResourcesDir() / "default.txt");
	std::ifstream file(pathDefault);
	std::string placeHolder;
	while (std::getline(file, placeHolder)) { quotes.push_back(placeHolder); }

	if (Mod::get()->getSettingValue<bool>("brandonRogers")) {
		auto pathRogers = (Mod::get()->getResourcesDir() / "brandonrogers.txt");
		std::ifstream file(pathRogers);
		std::string bRogers;
		while (std::getline(file, bRogers)) {
			std::string hBananaStud = fmt::format("\"{}", bRogers);
			quotes.push_back(hBananaStud);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}

	auto oldDNBMessages = (dirs::getModConfigDir() / "raydeeux.disturbingnewbests" / "custom.txt");
	if (std::filesystem::exists(oldDNBMessages) && !Mod::get()->getSavedValue<bool>("migrationFromDNBSuccess")) {
		log::info("std::filesystem::exists(oldDNBMessages): {}", std::filesystem::exists(oldDNBMessages));
		log::info("Storing oldDNBMessages now.");
		std::ifstream dNBFile(oldDNBMessages);
		std::string dNBStr;
		while (std::getline(dNBFile, dNBStr)) {
			dNBMigration.push_back(dNBStr);
			log::info("{}", dNBStr);
		}
		log::info("Finished storing oldDNBMessages.");
	}

	// code adapted with permission from dialouge handler original author thesillydoggo: https://discord.com/channels/911701438269386882/911702535373475870/1212633554345918514 --erymanthus | raydeeux

	auto pathCustom = (Mod::get()->getConfigDir() / "custom.txt");
	if (!std::filesystem::exists(pathCustom)) {
		if (dNBMigration.empty()) {
			log::info("dNBMigration was empty. Confirm \"std::filesystem::exists(oldDNBMessages)\" didn't appear earlier in the logs.");
			std::string content = R"(lorem ipsum
abc def
sample text
each line is a new "new best" message
[this text file was brought to you by deathscreentweaks. if you're seeing this in the config directory for wholesomeendtexts, something has gone terribly TERRIBLY wrong-simply ping @erymanthus in the geode sdk discord server and do not panic!])";
			utils::file::writeString(pathCustom, content);
		} else if (std::filesystem::exists(oldDNBMessages)) {
			if (!dNBMigration.empty()) {
				log::info("Migrating custom messages from DisturbingNewBests. Buckle up!");
				/*
				for (std::string dNBCustomMessage : dNBMigration) {
					// std::string stringToMigrate = dNBCustomMessage;
					utils::file::writeString(pathCustom,  fmt::format("{}\n", dNBCustomMessage));
				}
				*/
				std::filesystem::copy(oldDNBMessages, pathCustom);
				log::info("Finished migrating messages from DisturbingNewBests. Confirm nothing went terribly wrong.");
				Mod::get()->setSavedValue("migrationFromDNBSuccess", true);
			} else {
				log::error("Migration failed! What happened?");
				std::string content = R"(migration failed, womp womp
migration failed, womp womp
migration failed, womp womp
migration failed, womp womp)";
				utils::file::writeString(pathCustom, content);
			}
		}
	}
	
	if (Mod::get()->getSettingValue<bool>("custom")) {
		auto pathCustom = (Mod::get()->getConfigDir() / "custom.txt");
		std::ifstream file(pathCustom);
		std::string str;
		while (std::getline(file, str)) {
			if (str.starts_with("\"") && str.ends_with("\"")) {
				str = str.replace(0, 1, "\'\'");
			} else if (str.starts_with("\'") && str.ends_with("\'")) {
				str = str.replace(0, 2, "\"");
			}
			if (!Mod::get()->getSavedValue<bool>("noHyphens")) {
				str = fmt::format("- {} -", str);
			}
			quotes.push_back(str);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}
}

std::string grabRandomQuote() {
	std::mt19937 randomSeed(std::random_device{}());
	std::shuffle(quotes.begin(), quotes.end(), randomSeed);
	return quotes.front();
}

void forceEnableJustDont() {
	auto gm = GameManager::sharedState();
	if (!gm->getGameVariable("0095")) {
		gm->setGameVariable("0095", true);
		Mod::get()->setSettingValue("checkJustDont", false);
		log::info("\"Just Dont\"/\"Do Not\" check complete.");
	}
}

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		bool result = MenuLayer::init();
		if (!completedJDDNCheck && Mod::get()->getSettingValue<bool>("enabled") && Mod::get()->getSettingValue<bool>("checkJustDont")) {
			forceEnableJustDont();
		}
		completedJDDNCheck = true;
		return result;
	}
};

class $modify(MyPlayLayer, PlayLayer) {
	void updateProgressbar() {
		PlayLayer::updateProgressbar();
		if (!Mod::get()->getSettingValue<bool>("enabled")) { return; }
		if (m_level->isPlatformer()) { return; }
		if (!m_player1->m_isDead) { return; }
		for (int i = getChildrenCount() - 1; i >= 0; i--) {
			// NEW [good]: int i = getChildrenCount() - 1; i >= 0; i--
			// ORIG [bad]: int i = getChildrenCount(); i-- > 0;
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode) != nullptr) {
				theLastCCNode->setVisible(!Mod::get()->getSettingValue<bool>("currencyLayer"));
				continue;
			} // skip UILayer
			if (theLastCCNode == nullptr || typeinfo_cast<UILayer*>(theLastCCNode) != nullptr) { continue; } // skip UILayer
			if (theLastCCNode->getZOrder() != 100) { continue; } // macos-specific narrowing down nodes
			if (theLastCCNode->getChildrenCount() < 2) { continue; }
			if (Mod::get()->getSettingValue<bool>("noVisibleNewBest")) {
				theLastCCNode->setVisible(false);
				return;
			}
			auto deathNode = typeinfo_cast<CCLabelBMFont*>(theLastCCNode->getChildren()->objectAtIndex(0));
			if (deathNode == nullptr) { continue; }
			if (strcmp(deathNode->getFntFile(), "goldFont.fnt") != 0) { continue; } // avoid non-gold font HUDs
			auto maybeOnDeathString = deathNode->getString();
			if ((strcmp(formerCustomDeathString.c_str(), maybeOnDeathString) == 0)) { continue; } // avoid regenerating new quotes
			auto randomString = grabRandomQuote();
			formerCustomDeathString = randomString; // cache previous string for the next time it gets checked ( i had a really weird race condition where the new best message kept changing and i only want it to change once )
			if (strcmp("", randomString.c_str()) != 0) {
				deathNode->setString(randomString.c_str(), true);
				if (Mod::get()->getSettingValue<bool>("lineWrapping")) {
					deathNode->setAlignment(CCTextAlignment::kCCTextAlignmentCenter); // center text
					float scale = .25f * (155.f / strlen(randomString.c_str()));
					if (scale > Mod::get()->getSettingValue<double>("maxScale")) scale = Mod::get()->getSettingValue<double>("maxScale");
					deathNode->setWidth(420.f); // width of end screen minus 20px
					deathNode->setScale(scale);
				} else {
					deathNode->limitLabelWidth(420.f, 10.f, .25f); // you never know how long these custom strings might get
				}
				int64_t fontID = Mod::get()->getSettingValue<int64_t>("customFont");
				if (fontID == -2) {
					deathNode->setFntFile("chatFont.fnt");
				} else if (fontID == -1) {
					deathNode->setFntFile("bigFont.fnt");
				} else if (fontID != 0) {
					deathNode->setFntFile(fmt::format("gjFont{:02d}.fnt", fontID).c_str());
				}
				deathNode->setAlignment(kCCTextAlignmentCenter);
				if (fontID != 0 && Mod::get()->getSettingValue<bool>("customFontGoldColor")) {
					deathNode->setColor({254, 207, 6});
				}
			} // fallback to default newbest message in case randomstring is empty
			break;
		}
	}
};
