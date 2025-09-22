#include <ninxout.prntscrn/include/api.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <random>
#include "Manager.hpp"

#define getModBool Mod::get()->getSettingValue<bool>
#define getModString Mod::get()->getSettingValue<std::string>
#define getModFloat Mod::get()->getSettingValue<double>
#define getModInt Mod::get()->getSettingValue<int64_t>
#define manager Manager::getSharedInstance()

#define STOP_MANAGER_CHANNEL\
	bool isPlayingBoolean;\
	manager->channel->isPlaying(&isPlayingBoolean);\
	if (isPlayingBoolean) manager->channel->stop();

class $modify(MyPlayLayer, PlayLayer) {
	static std::string grabRandomQuote(std::vector<std::string> vector = manager->quotes) {
		if (vector.empty()) return "";
		static std::mt19937_64 engine(std::random_device{}());
		std::uniform_int_distribution<size_t> dist(0, vector.size() - 1);
		return vector.at(dist(engine));
	}
	bool didPlayerDieAtNewBest() {
		return this->getCurrentPercentInt() > this->m_level->m_normalPercent.value();
	}
	bool didPlayerDieAtNewBestFloat() {
		return this->getCurrentPercent() > this->m_level->m_normalPercent.value();
	}
	void resetLevel() {
		PlayLayer::resetLevel();
		manager->addedNextKeyWhenLabel = false;
		if (!manager->channel) return;
		if (!getModBool("sisyphusStopSFXOnRespawn")) return;
		STOP_MANAGER_CHANNEL
	}
	void onQuit() {
		PlayLayer::onQuit();
		manager->lastDeathPercent = -10.f;
		if (!manager->channel) return;
		STOP_MANAGER_CHANNEL
	}
	void levelComplete() {
		PlayLayer::levelComplete();
		manager->lastDeathPercent = -10.f;
		if (!manager->channel) return;
		STOP_MANAGER_CHANNEL
	}
	void togglePracticeMode(bool practiceMode) {
		PlayLayer::togglePracticeMode(practiceMode);
		manager->lastDeathPercent = -10.f;
		if (!manager->channel) return;
		STOP_MANAGER_CHANNEL
	}
	void resetLevelFromStart() {
		PlayLayer::resetLevelFromStart();
		manager->lastDeathPercent = -10.f;
		if (!manager->channel) return;
		if (!getModBool("enabled") || !getModBool("sisyphusStopSFXOnRespawn")) return;
		STOP_MANAGER_CHANNEL
	}
	void updateInfoLabel() {
		PlayLayer::updateInfoLabel();
		if (!getModBool("enabled") || !m_level || m_level->isPlatformer() || !m_player1->m_isDead || m_isPlatformer) return;

		CCNode* newBestNodeProbably = nullptr;
		bool hasOrbsLabel = false;
		bool hasKeyLabel = false;
		const bool isNewBest = MyPlayLayer::didPlayerDieAtNewBest();
		for (int i = static_cast<int>(getChildrenCount() - 1); i >= 0; i--) {
			// NEW [good]: int i = getChildrenCount() - 1; i >= 0; i--
			// ORIG [bad]: int i = getChildrenCount(); i-- > 0;
			auto theLastCCNode = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(i));
			if (const auto crl = typeinfo_cast<CurrencyRewardLayer*>(theLastCCNode)) {
				// hide CurrencyRewardLayer
				if (getModBool("currencyLayer")) {
					theLastCCNode->setVisible(false);
					if (crl->m_orbsLabel) hasOrbsLabel = true;
					if (crl->m_keysLabel) hasKeyLabel = true;
				}
				continue;
			}
			if (!theLastCCNode || theLastCCNode == this->m_uiLayer) continue; // skip UILayer
			if (theLastCCNode->getZOrder() != 100) continue;
			if (theLastCCNode->getChildrenCount() < 2) continue;
			if (getModBool("noVisibleNewBest")) return theLastCCNode->setVisible(false);
			newBestNodeProbably = theLastCCNode;
			break;
		}

		if (!newBestNodeProbably || newBestNodeProbably->getUserObject("modified-already"_spr)) return;
		newBestNodeProbably->setUserObject("modified-already"_spr, CCBool::create(true));

		if (getModBool("xPosPercentEnable")) {
			const float cWidth = CCScene::get()->getContentWidth();
			if (m_level->m_stars.value() == 0) newBestNodeProbably->setPositionX(cWidth * (std::clamp<float>(getModFloat("xPosPercent"), 0.f, 100.f) / 100.f));
			else newBestNodeProbably->setPositionX(cWidth * (std::clamp<float>(getModFloat("xPosPercentRatedLevels"), 0.f, 100.f) / 100.f));
		}
		if (getModBool("yPosPercentEnable")) {
			const float cHeight = CCScene::get()->getContentHeight();
			if (m_level->m_stars.value() == 0) newBestNodeProbably->setPositionY(cHeight * (std::clamp<float>(getModFloat("yPosPercent"), 0.f, 100.f) / 100.f));
			else newBestNodeProbably->setPositionY(cHeight * (std::clamp<float>(getModFloat("yPosPercentRatedLevels"), 0.f, 100.f) / 100.f));
		}

		if (manager->hasNextKeyWhenLoaded && getModBool("currencyLayer") && getModBool("currencyLayerNextKeyWhenCompat") && !manager->addedNextKeyWhenLabel && m_level->m_stars.value() > 1) {
			if (hasOrbsLabel) {
				CCLabelBMFont* nextKeyWhen = CCLabelBMFont::create(fmt::format("Key: {}/500", GameStatsManager::sharedState()->getTotalCollectedCurrency() % 500).c_str(), "bigFont.fnt");
				nextKeyWhen->setID("next-key-when-orbs-compat-label"_spr);
				nextKeyWhen->setTag(8042025);
				nextKeyWhen->setScale(.5f);
				nextKeyWhen->setColor({45, 255, 255});
				nextKeyWhen->setPosition(newBestNodeProbably->getContentSize() / 2.f);
				nextKeyWhen->setPositionY(nextKeyWhen->getPositionY() - 90.f);
				newBestNodeProbably->addChild(nextKeyWhen);
			}
			if (hasKeyLabel) {
				CCLabelBMFont* newKeyLabel = CCLabelBMFont::create("(+1 Key!)", "bigFont.fnt");
				newKeyLabel->setID("next-key-when-key-compat-label"_spr);
				newKeyLabel->setTag(8052025);
				newKeyLabel->setScale(.5f);
				newKeyLabel->setColor({235, 235, 235});
				newKeyLabel->setPosition(newBestNodeProbably->getContentSize() / 2.f);
				newKeyLabel->setPositionY(newKeyLabel->getPositionY() - 90.f);
				if (hasOrbsLabel) newKeyLabel->setPositionY(newKeyLabel->getPositionY() - 15.f);
				newBestNodeProbably->addChild(newKeyLabel);
			}
			manager->addedNextKeyWhenLabel = true;
		}

		std::smatch match;
		for (CCNode* child : CCArrayExt<CCNode*>(newBestNodeProbably->getChildren())) {
			if (child->getID() == "next-key-when-compat-label"_spr) continue;

			const auto hopefullyALabel = typeinfo_cast<CCLabelBMFont*>(child);
			if (!hopefullyALabel || hopefullyALabel->getTag() == 8042025) continue;

			std::string nodeString = hopefullyALabel->getString();
			std::string fontFile = hopefullyALabel->getFntFile();

			if (nodeString.ends_with("%") && fontFile == "bigFont.fnt") {
				// this is the node displaying where you died as a new best
				if (isNewBest && getModBool("accuratePercent")) return hopefullyALabel->setString(fmt::format("{:.{}f}%", getCurrentPercent(), getModInt("accuracy")).c_str());
				// i have to do all of this because robtop's wonderful technology shows percent from previous death if i dont include all of this
				if (!std::regex_match(nodeString, match, manager->percentRegex)) continue;
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
				float scale = .25f * (155.f / static_cast<float>(randomString.length()));
				if (scale > Mod::get()->getSettingValue<double>("maxScale")) scale = static_cast<float>(Mod::get()->getSettingValue<double>("maxScale"));
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
	void showNewBest(bool newReward, int orbs, int diamonds, bool demonKey, bool noRetry, bool noTitle) {
		PlayLayer::showNewBest(newReward, orbs, diamonds, demonKey, noRetry, noTitle);
		if (!getModBool("enabled") || !m_player1->m_isDead || this->m_isPracticeMode || this->m_isTestMode || m_isPlatformer || (m_level && m_level->isPlatformer())) return;
		if (!manager->hasPRNTSCRN || !getModBool("screenshotOnDeath")) return;
		this->scheduleOnce(schedule_selector(MyPlayLayer::PRNTSCRNOnDeath), .24f);
	}
	void PRNTSCRNOnDeath(float) {
		if (!getModBool("enabled") || !m_player1->m_isDead || this->m_isPracticeMode || this->m_isTestMode || m_isPlatformer || (m_level && m_level->isPlatformer())) return;
		if (!manager->hasPRNTSCRN || !getModBool("screenshotOnDeath")) return;

		Mod* prntscrn = Loader::get()->getLoadedMod("ninxout.prntscrn");
		if (!prntscrn) return;

		const std::string& screenshotOnDeathWhen = utils::string::toLower(getModString("screenshotOnDeathWhen"));
		const bool hasStars = m_level && m_level->m_stars.value() > 0;
		bool shouldScreenshot = false;

		if (screenshotOnDeathWhen == "always") shouldScreenshot = true;
		else if (utils::string::startsWith(screenshotOnDeathWhen, "\"new best\" ") && MyPlayLayer::didPlayerDieAtNewBestFloat()) {
			if (utils::string::endsWith(screenshotOnDeathWhen, " (all)")) shouldScreenshot = true;
			else if (hasStars && utils::string::endsWith(screenshotOnDeathWhen, " (rated)")) shouldScreenshot = true;
			else if (!hasStars && utils::string::endsWith(screenshotOnDeathWhen, " (unrated)")) shouldScreenshot = true;
			else shouldScreenshot = false;
		} else if (utils::string::startsWith(screenshotOnDeathWhen, "all ")) {
			if (utils::string::endsWith(screenshotOnDeathWhen, " (all)")) shouldScreenshot = true;
			else if (hasStars && utils::string::endsWith(screenshotOnDeathWhen, " (rated)")) shouldScreenshot = true;
			else if (!hasStars && utils::string::endsWith(screenshotOnDeathWhen, " (unrated)")) shouldScreenshot = true;
			else shouldScreenshot = false;
		} else shouldScreenshot = false;
		if (!shouldScreenshot) return;

		const std::string& screenshotOnDeathPreference = utils::string::toLower(getModString("screenshotOnDeathPreference"));
		const bool originalHidePlayerValue = prntscrn->getSettingValue<bool>("hide-player");
		prntscrn->setSettingValue<bool>("hide-player", false);
		log::info("screenshotOnDeathPreference: {}", screenshotOnDeathPreference);

		if (screenshotOnDeathPreference == "keep ui") {
			(void) PRNTSCRN::screenshotNodeAdvanced(this, {FMODAudioEngine::get()}, {""_spr});
		} else if (screenshotOnDeathPreference == "hide (~99% of) ui") {
			const bool originalHideUIValue = prntscrn->getSettingValue<bool>("hide-ui");
			prntscrn->setSettingValue<bool>("hide-ui", true);
			(void) PRNTSCRN::screenshotNode(this);
			prntscrn->setSettingValue<bool>("hide-ui", originalHideUIValue);
		} else {
			(void) PRNTSCRN::screenshotNode(this);
		}
		(void) PRNTSCRN::screenshotNodeAdvanced(CCScene::get(), {FMODAudioEngine::get()}, {""_spr});

		prntscrn->setSettingValue<bool>("hide-player", originalHidePlayerValue);
	}
};