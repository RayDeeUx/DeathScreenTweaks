#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

using namespace geode::prelude;

std::string formerCustomDeathString = "";

std::vector<std::string> quotes;

$on_mod(Loaded) {
	auto path = (Mod::get()->getResourcesDir() / "default.txt").string();
	std::ifstream file(path);
	std::string str;
	while (std::getline(file, str)) { quotes.push_back(str); }

	if (Mod::get()->getSettingValue<bool>("brandonRogers")) {
		auto pathRogers = (Mod::get()->getResourcesDir() / "brandonrogers.txt").string();
		std::ifstream file(pathRogers);
		std::string bRogers;
		while (std::getline(file, bRogers)) {
			std::string hBananaStud = fmt::format("\"{}", bRogers);
			quotes.push_back(hBananaStud);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}


	// code adapted with permission from dialouge handler original author thesillydoggo: https://discord.com/channels/911701438269386882/911702535373475870/1212633554345918514 --erymanthus | raydeeux

	auto path3 = (Mod::get()->getConfigDir() / "custom.txt").string();
	if (!std::filesystem::exists(path3)) {
		std::string content = R"(lorem ipsum
abc def
sample text
each line is a new "new best" message)";
		utils::file::writeString(path3, content);
	}
	
	if (Mod::get()->getSettingValue<bool>("custom")) {
		auto pathCustom = (Mod::get()->getConfigDir() / "custom.txt").string();
		std::ifstream file(pathCustom);
		std::string str;
		while (std::getline(file, str)) {
			std::string customStr = "- " + str + " -";
			quotes.push_back(customStr);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}
}

std::string grabRandomQuote() {
	std::mt19937 randomSeed(std::random_device{}());
	std::shuffle(quotes.begin(), quotes.end(), randomSeed);
	return quotes.front();
}

class $modify(MyPlayLayer, PlayLayer) {
	void updateProgressbar() {
		PlayLayer::updateProgressbar();
		if (!Mod::get()->getSettingValue<bool>("enabled")) { return; }
		if (!m_player1->m_isDead) { return; }
		for (int i = this->getChildrenCount(); i-- > 0; ) {
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode) != nullptr) {
				theLastCCNode->setVisible(!Mod::get()->getSettingValue<bool>("currencyLayer"));
				continue;
			} // skip UILayer
			if (theLastCCNode == nullptr || typeinfo_cast<UILayer*>(theLastCCNode) != nullptr) { continue; } // skip UILayer
			if (theLastCCNode->getZOrder() != 100) { continue; } // macos-specific narrowing down nodes
			if (theLastCCNode->getChildrenCount() < 2) { continue; }
			auto deathNode = typeinfo_cast<CCLabelBMFont*>(theLastCCNode->getChildren()->objectAtIndex(0));
			if (deathNode == nullptr) { continue; }
			#ifndef __APPLE__
				if (strcmp(deathNode->getFntFile(), "goldFont.fnt") != 0) { continue; } // avoid non-gold font HUDs
			#endif
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
			} // fallback to default newbest message in case randomstring is empty
			break;
		}
	}
};