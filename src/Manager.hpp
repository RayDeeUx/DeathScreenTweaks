#pragma once

#include <regex>

// Manager.hpp structure by acaruso
// reused with explicit permission and strong encouragement

using namespace geode::prelude;

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