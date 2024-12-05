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

	static Manager* getSharedInstance() {
		if (!instance) {
			instance = new Manager();
		}
		return instance;
	}

};