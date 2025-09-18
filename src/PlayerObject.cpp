#include <Geode/modify/PlayerObject.hpp>
#include <filesystem>

#include "Manager.hpp"

#define getBool Mod::get()->getSettingValue<bool>
#define getString Mod::get()->getSettingValue<std::string>
#define getInt Mod::get()->getSettingValue<int64_t>
#define getFloat Mod::get()->getSettingValue<double>
#define getFile Mod::get()->getSettingValue<std::filesystem::path>

using namespace geode::prelude;

class $modify(MyPlayerObject, PlayerObject) {
	static bool isNewBest(PlayLayer* pl) {
		return pl->getCurrentPercentInt() > pl->m_level->m_normalPercent.value();
	}
	void playerDestroyed(bool p0) {
		PlayerObject::playerDestroyed(p0);
		if (!m_gameLayer || m_gameLayer == LevelEditorLayer::get()) return;

		if (!getBool("enabled")) return;
		const auto pl = PlayLayer::get();
		if (!pl || this != pl->m_player1) return;
		const auto theLevel = pl->m_level;
		if (!theLevel || theLevel->isPlatformer()) return;
		if (this == pl->m_player2 && theLevel->m_twoPlayerMode) return;

		const bool isNewBest = MyPlayerObject::isNewBest(pl);
		const bool shouldPlayNewBestSFX = getBool("newBestSFX") && isNewBest;
		bool qualifiedToShowAFakeNewBest = false;
		// splitting it into three if statements for readability
		if (!pl->m_isTestMode && !pl->m_isPracticeMode && getBool("alwaysNewBest") && !isNewBest) qualifiedToShowAFakeNewBest = true;
		if (getBool("alwaysNewBestPlaytest") && pl->m_isTestMode) qualifiedToShowAFakeNewBest = true;
		if (getBool("alwaysNewBestPractice") && pl->m_isPracticeMode) qualifiedToShowAFakeNewBest = true;
		if (getBool("logging")) {
			log::info("pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value(): {}", pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value());
			log::info("pl->getCurrentPercentInt(): {}", pl->getCurrentPercentInt());
			log::info("pl->m_level->m_normalPercent.value(): {}", pl->m_level->m_normalPercent.value());
		}
		if (qualifiedToShowAFakeNewBest) pl->showNewBest(false, 0, 0, false, false, false);
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return;

		Manager* manager = Manager::getSharedInstance();
		const float plCurrentPercent = pl->getCurrentPercent();
		const float originalDeathPercent = manager->lastDeathPercent;
		const bool shouldActivateSisyphusMode = getBool("sisyphus") && std::abs(originalDeathPercent - plCurrentPercent) <= static_cast<float>(std::clamp(getFloat("sisyphusThreshold"), .01, 10.));
		if (!pl->m_isTestMode) {
			manager->lastDeathPercent = plCurrentPercent;
		}

		if (const std::filesystem::path& newBestSFXFile = Mod::get()->getConfigDir() / fmt::format("newBest.{}", getString("extension")); shouldPlayNewBestSFX && std::filesystem::exists(newBestSFXFile) && !pl->m_isTestMode && !pl->m_isPracticeMode) {
			if (getBool("logging")) {
				log::info("newBestSFXFile: {}", newBestSFXFile.string());
				log::info("getString(\"extension\"): {}", getString("extension"));
			}
			manager->system->createSound(geode::utils::string::pathToString(newBestSFXFile).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
			manager->system->playSound(manager->sound, nullptr, false, &manager->channel);
			manager->channel->setVolume(getInt("newBestVolume") / 100.0f);
		} else if (theLevel->m_stars.value() == 0 && shouldPlayNewBestSFX) {
			fmod->playEffect("magicExplosion.ogg");
		}

		if (CCNode* playLayerParent = pl->getParent(); !isNewBest && shouldActivateSisyphusMode && playLayerParent && !pl->m_isTestMode) {
			const std::filesystem::path& sisyphusSFXFile = getFile("sisyphusSFX");
			const std::filesystem::path& sisyphusImageFile = getFile("sisyphusImage");
			const auto extension = sisyphusSFXFile.extension();
			const bool audioIsValidExtension = extension == ".mp3" || extension == ".flac" || extension == ".wav" || extension == ".oga" || extension == ".ogg";
			if (getBool("sisyphusPlaySFX") && std::filesystem::exists(sisyphusSFXFile) && audioIsValidExtension) {
				manager->system->createSound(geode::utils::string::pathToString(sisyphusSFXFile).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
				manager->system->playSound(manager->sound, nullptr, false, &manager->channel);
				manager->channel->setVolume(getInt("sisyphusVolume") / 100.0f);
			}
			if (getBool("sisyphusAddImage") && std::filesystem::exists(sisyphusImageFile) && sisyphusImageFile.extension() == ".png" && pl->getParent()) {
				if (CCNode* sisyphusByID = playLayerParent->getChildByID("sisyphus"_spr)) sisyphusByID->removeMeAndCleanup();
				else if (CCNode* sisyphusByTag = playLayerParent->getChildByTag(8042025)) sisyphusByTag->removeMeAndCleanup();

				CCSprite* newSisyphus = CCSprite::create(sisyphusImageFile.string().c_str());

				const CCSize originalSize = playLayerParent->getContentSize();
				const CCSize replacementSize = newSisyphus->getContentSize();
				const float yRatio = originalSize.height / replacementSize.height;
				const float xRatio = originalSize.width / replacementSize.width;
				const std::string& fillMode = utils::string::toLower(getString("sisyphusImageFillMode"));

				if (fillMode == "fit to screen") newSisyphus->setScale(std::min(xRatio, yRatio));
				else if (fillMode == "fill screen") newSisyphus->setScale(std::max(xRatio, yRatio));
				else if (fillMode == "stretch to fill screen") {
					newSisyphus->setScaleX(xRatio);
					newSisyphus->setScaleY(yRatio);
				} else {
					newSisyphus->setScale(static_cast<float>(std::clamp(getFloat("sisyphusImageScale"), .01, 5.)));
				}

				newSisyphus->setID("sisyphus"_spr);
				newSisyphus->setTag(8042025);
				newSisyphus->setPosition(playLayerParent->getContentSize() / 2.f);

				playLayerParent->addChild(newSisyphus);
				newSisyphus->runAction(CCFadeOut::create(static_cast<float>(std::clamp(getFloat("sisyphusFadeOutTime"), 0., 10.))));
			}
		}
	}
};