#include <Geode/modify/PlayerObject.hpp>
#include <filesystem>

#define getBool Mod::get()->getSettingValue<bool>
#define getString Mod::get()->getSettingValue<std::string>
#define getInt Mod::get()->getSettingValue<int64_t>

using namespace geode::prelude;

class $modify(MyPlayerObject, PlayerObject) {
	static bool isNewBest(PlayLayer* pl) {
		return pl->getCurrentPercentInt() > pl->m_level->m_normalPercent.value();
	}
	void playerDestroyed(bool p0) {
		PlayerObject::playerDestroyed(p0);
		if (!getBool("enabled")) return;
		const auto pl = PlayLayer::get();
		if (!pl || this != pl->m_player1) return;
		const auto theLevel = pl->m_level;
		if (!theLevel || theLevel->isPlatformer()) return;
		if (this == pl->m_player2 && theLevel->m_twoPlayerMode) return;
		bool qualifiedForAlwaysNewBest = false;
		// splitting it into three if statements for readability
		if (!pl->m_isTestMode && !pl->m_isPracticeMode && getBool("alwaysNewBest") && !isNewBest(pl)) qualifiedForAlwaysNewBest = true;
		if (getBool("alwaysNewBestPlaytest") && pl->m_isTestMode) qualifiedForAlwaysNewBest = true;
		if (getBool("alwaysNewBestPractice") && pl->m_isPracticeMode) qualifiedForAlwaysNewBest = true;
		if (getBool("logging")) {
			log::info("pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value(): {}", pl->getCurrentPercentInt() <= pl->m_level->m_normalPercent.value());
			log::info("pl->getCurrentPercentInt(): {}", pl->getCurrentPercentInt());
			log::info("pl->m_level->m_normalPercent.value(): {}", pl->m_level->m_normalPercent.value());
		}
		if (qualifiedForAlwaysNewBest) pl->showNewBest(true, 0, 0, false, false, false);
		if (!getBool("newBestSFX") || MyPlayerObject::isNewBest(pl) || pl->m_isTestMode || pl->m_isPracticeMode) return; // shouldnt play new best sfx in practice/testmode
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