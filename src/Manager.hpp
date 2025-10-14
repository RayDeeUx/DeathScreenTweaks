#pragma once

#include <AdvancedLabel.hpp>
#include <regex>

// Manager.hpp structure by acaruso
// reused with explicit permission and strong encouragement

using namespace geode::prelude;

struct XPositionComparator {
	bool operator()(const GameObject* coinOne, const GameObject* coinTwo) const noexcept {
		return coinOne->m_positionX < coinTwo->m_positionX;
	}
};

static const Label::EmojiMap g_emojis = {
	{U"🥇", "secretCoin_01_001.png"},
	{U"💔", "secretCoin_b_01_001.png"},
	{U"🪙", "secretCoin_2_01_001.png"},
	{U"🥈", "secretCoin_2_b_01_001.png"}
};

class Manager {

protected:
	static Manager* instance;
public:

	bool calledAlready = false;

	std::regex percentRegex = std::regex(R"(^(\d+)%$)");

	std::vector<std::string> quotes;
	std::vector<std::string> customQuotes;
	std::vector<std::string> dNBMigration;

	bool completedJDDNCheck = false;

	bool hasPRNTSCRN = false;
	bool hasNextKeyWhenLoaded = false;
	bool deathAnimationsFromZilko = false;

	bool addedNextKeyWhenLabel = false;
	int selectedDeathAnimation = -1;

	float lastDeathPercent = -10.f;

	float currentDeathPercentForQueueInMainLoader = -1.f;

	std::map<GameObject*, bool, XPositionComparator> coins;

	FMOD::Sound* sound;
	FMOD::Channel* channel;
	FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

	static Manager* getSharedInstance() {
		if (!instance) {
			instance = new Manager();
		}
		return instance;
	}

};