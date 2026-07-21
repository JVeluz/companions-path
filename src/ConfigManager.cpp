#include "ConfigManager.h"

#include <fstream>

#include "Language.h"
#include "json.hpp"
#include "logger.h"
#include "profile.h"

using json = nlohmann::json;

namespace {
    Config config;
    std::string currentPath = "";
    std::vector<ChangedCallback> changedCallbacks;

    void NotifyChanged() {
        for (const auto& callback : changedCallbacks) {
            callback(config);
        }
    }
}

namespace ConfigManager {

    const Config& GetConfig() { return config; }

    void RegisterChangedCallback(ChangedCallback callback) {
        changedCallbacks.push_back(callback);
    }

    void SetHarmonize(bool value) {
        if (config.harmonize != value) {
            config.harmonize = value;
            NotifyChanged();
            Save();
        }
    }
    
    void SetLanguage(const std::string& lang) {
        if (config.currentLanguage != lang) {
            config.currentLanguage = lang;
            Language::Load(config.currentLanguage);
            NotifyChanged();
            Save();
        }
    }

    void SetSyncLevel(bool value) {
        if (config.syncLevel != value) {
            config.syncLevel = value;
            NotifyChanged();
            Save();
        }
    }

    void Refresh() {
        NotifyChanged();
    }

    void Load(const std::string& path) {
        currentPath = path;

        std::ifstream file(path);
        if (!file.is_open()) {
            logger::error("Could not open config file: {}", path);
            return;
        }

        json configFile;
        try {
            file >> configFile;
        } catch (const json::parse_error& e) {
            logger::error("JSON parsing error in config file: {}", e.what());
            return;
        }

        if (configFile.contains("Language")) {
            config.currentLanguage = configFile["Language"].get<std::string>();
        }

        if (configFile.contains("Harmonize")) {
            if (configFile["Harmonize"].is_boolean()) {
                config.harmonize = configFile["Harmonize"].get<bool>();
            } else {
                logger::error("Harmonize option must be a boolean (true or false).");
            }
        }

        if (configFile.contains("SyncLevel")) {
            if (configFile["SyncLevel"].is_boolean()) {
                config.syncLevel = configFile["SyncLevel"].get<bool>();
            } else {
                logger::error("SyncLevel option must be a boolean.");
            }
        }

        NotifyChanged();
    }

    void Save() {
        if (currentPath.empty()) {
            return;
        }

        json configFile;

        std::ifstream inFile(currentPath);
        if (inFile.is_open()) {
            try {
                inFile >> configFile;
            } catch (const json::parse_error& e) {
                logger::error("Failed to parse existing config before saving. Aborting save to prevent data loss. Error: {}", e.what());
                inFile.close();
                return;
            }
            inFile.close();
        }

        configFile["Harmonize"] = config.harmonize;
        configFile["SyncLevel"] = config.syncLevel;
        configFile["Language"] = config.currentLanguage;

        std::ofstream outFile(currentPath);
        if (outFile.is_open()) {
            outFile << configFile.dump(4);
            outFile.close();
        } else {
            logger::error("Could not open config file for writing: {}", currentPath);
        }
    }
}