#include <filesystem>
#include <fstream>
#include "Settings.hpp"
#include "Manager.hpp"
#include "boilerplate.hpp"

#define manager Manager::getSharedInstance()

using namespace geode::prelude;

$on_mod(Loaded) {
	Mod::get()->setLoggingEnabled(Mod::get()->getSettingValue<bool>("logging"));
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	(void) Mod::get()->registerCustomSettingType("refresh", &MyButtonSettingV3::parse);

	addResourceQuotes();

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
[make sure to edit this file!])";
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

	addQuotes("custom");

	listenForSettingChanges("default", [](bool unusedVar) {
		managerReset();
	});

	listenForSettingChanges("brandonRogers", [](bool unusedVar) {
		managerReset();
	});

	listenForSettingChanges("snl50", [](bool unusedVar) {
		managerReset();
	});

	listenForSettingChanges("custom", [](bool unusedVar) {
		managerReset();
	});
}