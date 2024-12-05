#include <filesystem>
#include <fstream>
#include "Settings.hpp"
#include "Manager.hpp"

#define manager Manager::getSharedInstance()

using namespace geode::prelude;

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	auto pathDefault = (Mod::get()->getResourcesDir() / "default.txt");
	std::ifstream file(pathDefault);
	std::string placeHolder;
	while (std::getline(file, placeHolder)) {
		manager->quotes.push_back(placeHolder);
	}

	if (Mod::get()->getSettingValue<bool>("brandonRogers")) {
		auto pathRogers = (Mod::get()->getResourcesDir() / "brandonrogers.txt");
		std::ifstream bRogersFile(pathRogers);
		std::string bRogers;
		while (std::getline(bRogersFile, bRogers)) {
			std::string hBananaStud = fmt::format("\"{}", bRogers);
			manager->quotes.push_back(hBananaStud);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}

	auto oldDNBMessages = (dirs::getModConfigDir() / "raydeeux.disturbingnewbests" / "custom.txt");
	if (std::filesystem::exists(oldDNBMessages) && !Mod::get()->getSavedValue<bool>("migrationFromDNBSuccess")) {
		log::info("std::filesystem::exists(oldDNBMessages): {}", std::filesystem::exists(oldDNBMessages));
		log::info("Storing oldDNBMessages now.");
		std::ifstream dNBFile(oldDNBMessages);
		std::string dNBStr;
		while (std::getline(dNBFile, dNBStr)) {
			manager->dNBMigration.push_back(dNBStr);
			log::info("{}", dNBStr);
		}
		log::info("Finished storing oldDNBMessages.");
	}

	// code adapted with permission from dialouge handler original author thesillydoggo: https://discord.com/channels/911701438269386882/911702535373475870/1212633554345918514 --erymanthus | raydeeux

	auto pathCustom = (Mod::get()->getConfigDir() / "custom.txt");
	if (!std::filesystem::exists(pathCustom)) {
		if (manager->dNBMigration.empty()) {
			log::info("dNBMigration was empty. Confirm \"std::filesystem::exists(oldDNBMessages)\" didn't appear earlier in the logs.");
			std::string content = R"(lorem ipsum
abc def
sample text
each line is a new "new best" message
[this text file was brought to you by deathscreentweaks. if you're seeing this in the config directory for wholesomeendtexts, something has gone terribly TERRIBLY wrong-simply ping @erymanthus in the geode sdk discord server and do not panic!])";
			(void) utils::file::writeString(pathCustom, content);
		} else if (std::filesystem::exists(oldDNBMessages)) {
			if (!manager->dNBMigration.empty()) {
				log::info("Migrating custom messages from DisturbingNewBests. Buckle up!");
				/*
				for (std::string dNBCustomMessage : dNBMigration) {
					// std::string stringToMigrate = dNBCustomMessage;
					utils::file::writeString(pathCustom,  fmt::format("{}\n", dNBCustomMessage));
				}
				*/
				std::filesystem::copy(oldDNBMessages, pathCustom);
				log::info("Finished migrating messages from DisturbingNewBests. Confirm nothing went terribly wrong.");
				Mod::get()->setSavedValue("migrationFromDNBSuccess", true);
			} else {
				log::error("Migration failed! What happened?");
				std::string content = R"(migration failed, womp womp
migration failed, womp womp
migration failed, womp womp
migration failed, womp womp)";
				(void) utils::file::writeString(pathCustom, content);
			}
		}
	}

	if (Mod::get()->getSettingValue<bool>("custom")) {
		auto pathCustomConfig = (Mod::get()->getConfigDir() / "custom.txt");
		std::ifstream fileConfig(pathCustomConfig);
		std::string str;
		while (std::getline(fileConfig, str)) {
			if (str.starts_with("\"") && str.ends_with("\"")) {
				str = str.replace(0, 1, "\'\'");
			} else if (str.starts_with("\'") && str.ends_with("\'")) {
				str = str.replace(0, 2, "\"");
			}
			if (!Mod::get()->getSavedValue<bool>("noHyphens")) {
				str = fmt::format("- {} -", str);
			}
			manager->quotes.push_back(str);
			manager->customQuotes.push_back(str);
		} // technically i can write two one-time use boolean variables to allow people to toggle these things on and off as they please without the quotes adding themselves multiple times into the vector, but i'd rather add the "restart required" barrier just to be extra safe
	}
}