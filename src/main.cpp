#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <regex>

using namespace geode::prelude;

const static std::regex percentRegex = std::regex(R"(^(\d+)%$)");

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

bool isNewBest(PlayLayer* pl) {
	return pl->getCurrentPercentInt() > pl->m_level->m_normalPercent.value();
}

class $modify(MyMenuLayer, MenuLayer) {
	struct Fields {
		Mod* mod = Mod::get();
	};
	bool getBool(const std::string& key) {
		return m_fields->mod->getSettingValue<bool>(key);
	}
	bool init() {
		bool result = MenuLayer::init();
		if (!completedJDDNCheck && getBool("enabled") && getBool("checkJustDont")) {
			forceEnableJustDont();
		}
		completedJDDNCheck = true;
		return result;
	}
};

class $modify(MyPlayLayer, PlayLayer) {
	struct Fields {
		Mod* mod = Mod::get();
	};
	bool getBool(const std::string& key) {
		return m_fields->mod->getSettingValue<bool>(key);
	}
	int64_t getInt(const std::string& key) {
		return m_fields->mod->getSettingValue<int64_t>(key);
	}
	void updateProgressbar() {
		PlayLayer::updateProgressbar();
		if (!getBool("enabled")) { return; }
		if (m_level->isPlatformer() || !m_player1->m_isDead || m_isPlatformer) { return; }
		for (int i = getChildrenCount() - 1; i >= 0; i--) {
			// NEW [good]: int i = getChildrenCount() - 1; i >= 0; i--
			// ORIG [bad]: int i = getChildrenCount(); i-- > 0;
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode) != nullptr) {
				// hide CurrencyRewardLayer
				theLastCCNode->setVisible(!getBool("currencyLayer"));
				continue;
			}
			if (theLastCCNode == nullptr || typeinfo_cast<UILayer*>(theLastCCNode) != nullptr) { continue; } // skip UILayer
			if (theLastCCNode->getZOrder() != 100) { continue; }
			if (theLastCCNode->getChildrenCount() < 2) { continue; }
			if (getBool("noVisibleNewBest")) { return theLastCCNode->setVisible(false); }
			for (const auto child : CCArrayExt<CCNode*>(theLastCCNode->getChildren())) {
				const auto node = typeinfo_cast<CCLabelBMFont*>(child);
				if (!node) { continue; }
				std::string nodeString = node->getString();
				std::string fontFile = node->getFntFile();
				if (nodeString.ends_with("%") && fontFile == "bigFont.fnt") {
					if (isNewBest(this) && getBool("accuratePercent")) { return node->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getInt("accuracy")).c_str()); }
					// i have to do all of this because robtop's wonderful technology shows percent from previous death if i dont include all of this
					std::smatch match;
					if (!std::regex_match(nodeString, match, percentRegex)) { continue; }
					auto percent = std::regex_replace(nodeString, std::regex("%"), "");
					auto percentAsInt = utils::numFromString<int>(percent);
					if (percentAsInt.isErr()) { continue; }
					auto currPercent = this->getCurrentPercentInt();
					if (getBool("logging")) {
						log::info("percentAsInt == currentPercentInt: {}", percentAsInt.unwrap() == currPercent);
						log::info("percentAsInt: {}", percentAsInt.unwrap());
						log::info("getCurrentPercentInt: {}", currPercent);
					}
					if (!getBool("accuratePercent")) { node->setString(fmt::format("{}%", currPercent).c_str()); }
					else { node->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getInt("accuracy")).c_str()); }
					continue;
				}
				const std::string randomString = grabRandomQuote();
				if (fontFile != "goldFont.fnt" || std::ranges::find(quotes, nodeString) != quotes.end() || randomString.empty()) { continue; } // avoid regenerating new quotes
				if (getBool("hideNewBestMessages")) {
					node->setVisible(false);
					continue;
				}
				if (!getBool("changeDeathText")) { continue; }
				node->setString(randomString.c_str(), true);
				if (getBool("lineWrapping")) {
					node->setAlignment(CCTextAlignment::kCCTextAlignmentCenter); // center text
					float scale = .25f * (155.f / randomString.length());
					if (scale > Mod::get()->getSettingValue<double>("maxScale")) { scale = Mod::get()->getSettingValue<double>("maxScale"); }
					node->setWidth(420.f); // width of end screen minus 20px, not marajuana referenec
					node->setScale(scale);
				} else { node->limitLabelWidth(420.f, 10.f, .25f); } // you never know how long these custom strings might get
				auto fontID = getInt("customFont");
				if (fontID == -2) { node->setFntFile("chatFont.fnt"); }
				else if (fontID == -1) { node->setFntFile("bigFont.fnt"); }
				else if (fontID != 0) { node->setFntFile(fmt::format("gjFont{:02d}.fnt", fontID).c_str()); }
				node->setAlignment(kCCTextAlignmentCenter);
				if (fontID != 0 && getBool("customFontGoldColor")) { node->setColor({254, 207, 6}); }
			}
		}
	}
};

class $modify(MyPlayerObject, PlayerObject) {
	struct Fields {
		Mod* mod = Mod::get();
	};
	bool getBool(const std::string& key) {
		return m_fields->mod->getSettingValue<bool>(key);
	}
	std::string getString(const std::string& key) {
		return m_fields->mod->getSettingValue<std::string>(key);
	}
	int64_t getInt(const std::string& key) {
		return m_fields->mod->getSettingValue<int64_t>(key);
	}
	void playerDestroyed(bool p0) {
		PlayerObject::playerDestroyed(p0);
		if (!getBool("enabled")) { return; }
		const auto pl = PlayLayer::get();
		if (!pl) { return; }
		if (this != pl->m_player1) { return; }
		const auto theLevel = pl->m_level;
		if (!theLevel) { return; }
		if (theLevel->isPlatformer()) { return; }
		if (this == pl->m_player2 && theLevel->m_twoPlayerMode) { return; }
		bool qualifiedForAlwaysNewBest = false;
		// splitting it into three if statements for readability
		if (!pl->m_isTestMode && !pl->m_isPracticeMode && getBool("alwaysNewBest") && !isNewBest(pl)) { qualifiedForAlwaysNewBest = true; }
		if (getBool("alwaysNewBestPlaytest") && pl->m_isTestMode) { qualifiedForAlwaysNewBest = true; }
		if (getBool("alwaysNewBestPractice") && pl->m_isPracticeMode) { qualifiedForAlwaysNewBest = true; }
		if (getBool("logging")) {
			log::info("pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value(): {}", pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value());
			log::info("pl->getCurrentPercentInt(): {}", pl->getCurrentPercentInt());
			log::info("pl->m_level->m_normalPercent.value(): {}", pl->m_level->m_normalPercent.value());
		}
		if (qualifiedForAlwaysNewBest) {
			pl->showNewBest(true, 0, 0, false, false, false);
		}
		if (!getBool("newBestSFX") || !isNewBest(pl) || pl->m_isTestMode || pl->m_isPracticeMode) { return; } // shouldnt play new best sfx in practice/testmode
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) { return; }
		if (const auto newBestSFXFile = Mod::get()->getConfigDir() / fmt::format("newBest.{}", getString("extension")); std::filesystem::exists(newBestSFXFile)) {
			if (getBool("logging")) {
				log::info("newBestSFXFile: {}", newBestSFXFile.string());
				log::info("getString(\"extension\"): {}", getString("extension"));
			}
			auto system = fmod->m_system;
			FMOD::Channel* channel;
			FMOD::Sound* sound;
			system->createSound(newBestSFXFile.string().c_str(), FMOD_DEFAULT, nullptr, &sound);
			system->playSound(sound, nullptr, false, &channel);
			channel->setVolume(getInt("newBestVolume") / 100.0f);
		} else if (theLevel->m_stars.value() == 0) {
			fmod->playEffect("magicExplosion.ogg");
		}
	}
};