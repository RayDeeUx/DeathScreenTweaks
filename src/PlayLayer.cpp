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
		for (int i = getChildrenCount() - 1; i >= 0; i--) {
			// NEW [good]: int i = getChildrenCount() - 1; i >= 0; i--
			// ORIG [bad]: int i = getChildrenCount(); i-- > 0;
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode) != nullptr) {
				// hide CurrencyRewardLayer
				theLastCCNode->setVisible(!getModBool("currencyLayer"));
				continue;
			}
			if (theLastCCNode == nullptr || typeinfo_cast<UILayer*>(theLastCCNode) != nullptr) continue; // skip UILayer
			if (theLastCCNode->getZOrder() != 100) continue;
			if (theLastCCNode->getChildrenCount() < 2) continue;
			if (getModBool("noVisibleNewBest")) return theLastCCNode->setVisible(false);
			for (const auto child : CCArrayExt<CCNode*>(theLastCCNode->getChildren())) {
				const auto node = typeinfo_cast<CCLabelBMFont*>(child);
				if (!node) continue;
				std::string nodeString = node->getString();
				std::string fontFile = node->getFntFile();
				if (nodeString.ends_with("%") && fontFile == "bigFont.fnt") {
					if (MyPlayLayer::isNewBest(this) && getModBool("accuratePercent")) { return node->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getModInt("accuracy")).c_str()); }
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
					if (!getModBool("accuratePercent")) node->setString(fmt::format("{}%", currPercent).c_str());
					else node->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getModInt("accuracy")).c_str());
					continue;
				}
				std::string randomString = grabRandomQuote();
				if (!manager->customQuotes.empty() && getModBool("customTextsOnly")) randomString = grabRandomQuote(manager->customQuotes);
				if (fontFile != "goldFont.fnt" || std::ranges::find(manager->quotes, nodeString) != manager->quotes.end() || randomString.empty()) continue; // avoid regenerating new quotes
				if (getModBool("hideNewBestMessages")) {
					node->setVisible(false);
					continue;
				}
				auto fontID = getModInt("customFont");
				if (fontID == -3) {
					node->setFntFile("newBestFont.fnt"_spr);
					node->setExtraKerning(4);
					randomString = utils::string::toUpper(randomString); // oxygene one does not support lowercase chars
					randomString = utils::string::replace(randomString, "\"", "\'\'"); // oxygene one does not support `"` char
				}
				if (getModBool("changeDeathText")) node->setString(randomString.c_str(), true);
				if (getModBool("lineWrapping")) {
					node->setAlignment(CCTextAlignment::kCCTextAlignmentCenter); // center text
					float scale = .25f * (155.f / randomString.length());
					if (scale > Mod::get()->getSettingValue<double>("maxScale")) scale = Mod::get()->getSettingValue<double>("maxScale");
					node->setWidth(420.f); // width of end screen minus 20px, not marajuana referenec
					node->setScale(scale);
				} else { node->limitLabelWidth(420.f, 10.f, .25f); } // you never know how long these custom strings might get
				if (fontID == -2) node->setFntFile("chatFont.fnt");
				else if (fontID == -1) node->setFntFile("bigFont.fnt");
				else if (fontID != 0 && fontID != -3) node->setFntFile(fmt::format("gjFont{:02d}.fnt", fontID).c_str());
				node->setAlignment(kCCTextAlignmentCenter);
				if (fontID != 0 && fontID != -3 && getModBool("customFontGoldColor")) node->setColor({254, 207, 6});
			}
		}
	}
};