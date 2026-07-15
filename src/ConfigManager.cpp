#include "ConfigManager.h"
#include "logger.h"
#include "json.hpp"
#include "profile.h"
#include "language.h"

#include <fstream>

using json = nlohmann::json;

namespace {
    bool harmonize = true; 
    std::string currentLanguage = "english";
    std::string currentConfigPath = "";
}

namespace ConfigManager {
    void LoadConfig(const std::string& configPath) {
        LanguageRepository::ScanAvailableLanguages();

        currentConfigPath = configPath;

        std::ifstream file(configPath);
        if (!file.is_open()) {
            logger::error("Could not open config file: {}", configPath);
            return;
        }

        json config;
        try {
            file >> config;
        } catch (const json::parse_error& e) {
            logger::error("JSON parsing error in config file: {}", e.what());
            return;
        }

        if (config.contains("Language")) {
            currentLanguage = config["Language"].get<std::string>();
            logger::info("Language override found in config: {}", currentLanguage);
            LanguageRepository::LoadLanguage(currentLanguage);
        }

        if (config.contains("Harmonize")) {
            if (config["Harmonize"].is_boolean()) {
                harmonize = config["Harmonize"].get<bool>();
                logger::info("Harmonize option set to: {}", harmonize ? "true" : "false");
            } else {
                logger::error("Harmonize option must be a boolean (true or false).");
            }
        }

        ProfileRepository::InitializeFromJson(config);
    }

    void SaveConfig() {
        if (currentConfigPath.empty()) {
            logger::error("Cannot save config: Path is empty.");
            return;
        }

        json config;
        
        std::ifstream inFile(currentConfigPath);
        if (inFile.is_open()) {
            try {
                inFile >> config;
            } catch (const json::parse_error& e) {
                logger::warn("Failed to parse existing config before saving, creating a new one. Error: {}", e.what());
            }
            inFile.close();
        }

        config["Harmonize"] = harmonize;
        config["Language"] = currentLanguage;

        std::ofstream outFile(currentConfigPath);
        if (outFile.is_open()) {
            outFile << config.dump(4);
            outFile.close();
            logger::info("Configuration saved successfully.");
        } else {
            logger::error("Could not open config file for writing: {}", currentConfigPath);
        }
    }

    bool GetHarmonize() {
        return harmonize;
    }
    
    void SetHarmonize(bool value) {
        if (harmonize != value) {
            harmonize = value;
            SaveConfig();
            logger::info("Harmonize changed to {}", harmonize);
        }
    }

    std::string GetLanguage() {
        return currentLanguage;
    }

    void SetLanguage(const std::string& lang) {
        if (currentLanguage != lang) {
            currentLanguage = lang;
            LanguageRepository::LoadLanguage(currentLanguage);
            SaveConfig();
            logger::info("Language changed via UI to: {}", currentLanguage);
        }
    }
}