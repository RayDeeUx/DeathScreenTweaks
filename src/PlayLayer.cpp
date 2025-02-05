#include <Geode/modify/PlayLayer.hpp>
#include <iostream>
#include <algorithm>
#include <random>
#include "Manager.hpp"

#define getModBool Mod::get()->getSettingValue<bool>
#define getModString Mod::get()->getSettingValue<std::string>
#define getModInt Mod::get()->getSettingValue<int64_t>
#define manager Manager::getSharedInstance()

class $modify(MyPlayLayer, PlayLayer) {
	static std::string grabRandomQuote(std::vector<std::string> vector = manager->quotes) {
		if (vector.empty()) return "";
		std::mt19937 randomSeed(std::random_device{}());
		std::shuffle(vector.begin(), vector.end(), randomSeed);
		return vector.front();
	}
	static bool isNewBest(PlayLayer* pl) {
		return pl->getCurrentPercentInt() > pl->m_level->m_normalPercent.value();
	}
	void updateProgressbar() {
		PlayLayer::updateProgressbar();
		if (!getModBool("enabled") || m_level->isPlatformer() || !m_player1->m_isDead || m_isPlatformer) return;
		CCNode* allTheGoodStuff = nullptr;
		for (int i = getChildrenCount() - 1; i >= 0; i--) {
			// NEW [good]: int i = getChildrenCount() - 1; i >= 0; i--
			// ORIG [bad]: int i = getChildrenCount(); i-- > 0;
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode)) {
				// hide CurrencyRewardLayer
				if (getModBool("currencyLayer")) theLastCCNode->setVisible(false);
				continue;
			}
			if (!theLastCCNode || theLastCCNode == this->m_uiLayer) continue; // skip UILayer
			if (theLastCCNode->getZOrder() != 100) continue;
			if (theLastCCNode->getChildrenCount() < 2) continue;
			if (getModBool("noVisibleNewBest")) return theLastCCNode->setVisible(false);
			allTheGoodStuff = theLastCCNode;
			break;
		}
		if (!allTheGoodStuff) return;
		for (const auto child : CCArrayExt<CCNode*>(allTheGoodStuff->getChildren())) {
			const auto hopefullyALabel = typeinfo_cast<CCLabelBMFont*>(child);
			if (!hopefullyALabel) continue;
			std::string nodeString = hopefullyALabel->getString();
			std::string fontFile = hopefullyALabel->getFntFile();
			if (nodeString.ends_with("%") && fontFile == "bigFont.fnt") {
				// this is the node displaying where you died as a new best
				if (MyPlayLayer::isNewBest(this) && getModBool("accuratePercent")) return hopefullyALabel->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getModInt("accuracy")).c_str());
				// i have to do all of this because robtop's wonderful technology shows percent from previous death if i dont include all of this
				std::smatch match;
				if (!std::regex_match(nodeString, match, manager->percentRegex)) { continue; }
				auto percent = std::regex_replace(nodeString, std::regex("%"), "");
				auto percentAsInt = utils::numFromString<int>(percent);
				if (percentAsInt.isErr()) continue;
				auto currPercent = this->getCurrentPercentInt();
				if (getModBool("logging")) {
					log::info("percentAsInt == currentPercentInt: {}", percentAsInt.unwrap() == currPercent);
					log::info("percentAsInt: {}", percentAsInt.unwrap());
					log::info("getCurrentPercentInt: {}", currPercent);
				}
				if (!getModBool("accuratePercent")) hopefullyALabel->setString(fmt::format("{}%", currPercent).c_str());
				else hopefullyALabel->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getModInt("accuracy")).c_str());
				continue;
			}
			std::string randomString = !manager->customQuotes.empty() && getModBool("customTextsOnly") ? grabRandomQuote(manager->customQuotes) : grabRandomQuote();
			if (fontFile != "goldFont.fnt" || std::ranges::find(manager->quotes, nodeString) != manager->quotes.end()) continue; // avoid regenerating new quotes
			if (getModBool("hideNewBestMessages")) {
				hopefullyALabel->setVisible(false);
				continue;
			}
			if (!getModBool("changeDeathText")) continue;
			auto fontID = getModInt("customFont");
			if (fontID == -3) {
				hopefullyALabel->setFntFile("newBestFont.fnt"_spr);
				hopefullyALabel->setExtraKerning(4);
				randomString = utils::string::toUpper(randomString); // oxygene one does not support lowercase chars
				randomString = utils::string::replace(randomString, "\"", "\'\'"); // oxygene one does not support `"` char
			}
			if (!randomString.empty()) hopefullyALabel->setString(randomString.c_str(), true);
			if (getModBool("lineWrapping")) {
				hopefullyALabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter); // center text
				float scale = .25f * (155.f / randomString.length());
				if (scale > Mod::get()->getSettingValue<double>("maxScale")) scale = Mod::get()->getSettingValue<double>("maxScale");
				hopefullyALabel->setWidth(420.f); // width of end screen minus 20px, not marajuana referenec
				hopefullyALabel->setScale(scale);
			} else hopefullyALabel->limitLabelWidth(420.f, 10.f, .25f); // you never know how long these custom strings might get
			if (fontID == -2) hopefullyALabel->setFntFile("chatFont.fnt");
			else if (fontID == -1) hopefullyALabel->setFntFile("bigFont.fnt");
			else if (fontID != 0 && fontID != -3) hopefullyALabel->setFntFile(fmt::format("gjFont{:02d}.fnt", fontID).c_str());
			hopefullyALabel->setAlignment(kCCTextAlignmentCenter);
			if (fontID != 0 && fontID != -3 && getModBool("customFontGoldColor")) hopefullyALabel->setColor({254, 207, 6});
		}
	}
};
