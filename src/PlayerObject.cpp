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
		if (!pl || (this != pl->m_player1 && this != pl->m_player2)) return;
		const auto theLevel = pl->m_level;
		if (!theLevel || theLevel->isPlatformer()) return;
		if (this == pl->m_player2 && theLevel->m_twoPlayerMode) return;

		const bool isNewBest = MyPlayerObject::isNewBest(pl);
		const bool shouldPlayNewBestSFX = getBool("newBestSFX") && isNewBest;
		bool qualifiedToShowNewBest = false;
		// splitting it into three if statements for readability
		if (!pl->m_isTestMode && !pl->m_isPracticeMode && getBool("alwaysNewBest") && !isNewBest) qualifiedToShowNewBest = true;
		if (getBool("alwaysNewBestPlaytest") && pl->m_isTestMode) qualifiedToShowNewBest = true;
		if (getBool("alwaysNewBestPractice") && pl->m_isPracticeMode) qualifiedToShowNewBest = true;
		if (getBool("logging")) {
			log::info("pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value(): {}", pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value());
			log::info("pl->getCurrentPercentInt(): {}", pl->getCurrentPercentInt());
			log::info("pl->m_level->m_normalPercent.value(): {}", pl->m_level->m_normalPercent.value());
		}
		if (qualifiedToShowNewBest) pl->showNewBest(true, 0, 0, false, false, false);
		if (pl->m_isTestMode || pl->m_isPracticeMode) return; // shouldnt play new best sfx in practice/testmode
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return;

		Manager* manager = Manager::getSharedInstance();
		const float plCurrentPercent = pl->getCurrentPercent();
		const float originalDeathPercent = manager->lastDeathPercent;
		const bool shouldActivateSisyphusMode = getBool("sisyphus") && std::abs(originalDeathPercent - plCurrentPercent) <= static_cast<float>(std::clamp(getFloat("sisyphusThreshold"), .01, 10.));
		manager->lastDeathPercent = plCurrentPercent;

		const auto system = fmod->m_system;
		FMOD::Channel* channel;
		FMOD::Sound* sound;

		if (const std::filesystem::path& newBestSFXFile = Mod::get()->getConfigDir() / fmt::format("newBest.{}", getString("extension")); shouldPlayNewBestSFX && std::filesystem::exists(newBestSFXFile)) {
			if (getBool("logging")) {
				log::info("newBestSFXFile: {}", newBestSFXFile.string());
				log::info("getString(\"extension\"): {}", getString("extension"));
			}
			system->createSound(newBestSFXFile.string().c_str(), FMOD_DEFAULT, nullptr, &sound);
			system->playSound(sound, nullptr, false, &channel);
			channel->setVolume(getInt("newBestVolume") / 100.0f);
		} else if (theLevel->m_stars.value() == 0 && shouldPlayNewBestSFX) {
			fmod->playEffect("magicExplosion.ogg");
		}

		/*
		sisyphusPlaySFX
		sisyphusSFX
		sisyphusAddImage
		sisyphusImage
		*/

		if (!isNewBest && shouldActivateSisyphusMode) {
			const std::filesystem::path& sisyphusSFXFile = getFile("sisyphusSFX");
			const std::filesystem::path& sisyphusImageFile = getFile("sisyphusImage");
			const auto extension = sisyphusSFXFile.extension();
			const bool audioIsValidExtension = extension == ".mp3" || extension == ".flac" || extension == ".wav" || extension == ".oga" || extension == ".ogg";
			if (getBool("sisyphusPlaySFX") && std::filesystem::exists(sisyphusSFXFile) && audioIsValidExtension) {
				system->createSound(sisyphusSFXFile.string().c_str(), FMOD_DEFAULT, nullptr, &sound);
				system->playSound(sound, nullptr, false, &channel);
				channel->setVolume(getInt("sisyphusVolume") / 100.0f);
				log::info("playing {} at volume {}", sisyphusSFXFile, getInt("sisyphusVolume") / 100.0f);
			}
			if (getBool("sisyphusAddImage") && std::filesystem::exists(sisyphusImageFile) && sisyphusImageFile.extension() == ".png" && pl->getParent()) {
				CCNode* playLayerParent = pl->getParent();
				if (CCNode* sisyphusByID = playLayerParent->getChildByID("sisyphus"_spr)) sisyphusByID->removeMeAndCleanup();
				else if (CCNode* sisyphusByTag = playLayerParent->getChildByTag(8042025)) sisyphusByTag->removeMeAndCleanup();

				CCSprite* newSisyphus = CCSprite::create(sisyphusImageFile.string().c_str());

				const CCSize originalSize = playLayerParent->getContentSize();
				const CCSize replacementSize = newSisyphus->getContentSize();
				const float yRatio = originalSize.height / replacementSize.height;
				const float xRatio = originalSize.width / replacementSize.width;
				const std::string& fillMode = getString("sisyphusImageFillMode");

				if (fillMode == "Fit to Screen") newSisyphus->setScale(std::min(xRatio, yRatio));
				else if (fillMode == "Fill Screen") newSisyphus->setScale(std::max(xRatio, yRatio));
				else if (fillMode == "Stretch to Fill Screen") {
					newSisyphus->setScaleX(xRatio);
					newSisyphus->setScaleY(yRatio);
				} else newSisyphus->setScale(static_cast<float>(std::clamp(getFloat("sisyphusImageScale"), .01, 5.)));
				newSisyphus->setID("sisyphus"_spr);
				newSisyphus->setTag(8042025);
				newSisyphus->setPosition(playLayerParent->getContentSize() / 2.f);

				playLayerParent->addChild(newSisyphus);
				newSisyphus->runAction(CCFadeOut::create(.5f));
			}
		}
	}
};