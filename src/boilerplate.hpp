#pragma once

#include "Manager.hpp"

#define getModBool Mod::get()->getSettingValue<bool>
#define managerMacro Manager::getSharedInstance()
#define configDir Mod::get()->getConfigDir()

inline void addQuotes(const std::string& settingName) {
	Manager* manager = managerMacro;
	if (settingName == "custom" && getModBool(settingName)) {
		log::info("adding custom.txt quotes");
		auto pathCustomConfigDir = (configDir / "custom.txt");
		std::ifstream fileConfigDir(pathCustomConfigDir);
		std::string str;
		while (std::getline(fileConfigDir, str)) {
			if (str.starts_with("\"") && str.ends_with("\""))
				str = str.replace(0, 1, "\'\'");
			else if (str.starts_with("\'") && str.ends_with("\'"))
				str = str.replace(0, 2, "\"");
			if (!Mod::get()->getSavedValue<bool>("noHyphens")) str = fmt::format("- {} -", str);
			manager->quotes.push_back(str);
			manager->customQuotes.push_back(str);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	} else if (getModBool(settingName)) {
		log::info("adding quotes from {}", settingName);
		auto settingAsFileName = fmt::format("{}.txt", settingName);
		auto filePath = (Mod::get()->getResourcesDir() / settingAsFileName).string();
		std::ifstream fileStream(filePath);
		std::string lineOfText;
		while (std::getline(fileStream, lineOfText)) {
			std::string textToAdd = settingName != "default" ? fmt::format("\"{}", lineOfText) : lineOfText;
			manager->quotes.push_back(textToAdd);
		}
	}
}

inline void addResourceQuotes() {
	addQuotes("default");
	addQuotes("brandonRogers");
	addQuotes("snl50");
}

inline void managerReset() {
	Manager* manager = managerMacro;
	manager->quotes.clear();
	manager->customQuotes.clear();
	addResourceQuotes();
	addQuotes("custom");
}