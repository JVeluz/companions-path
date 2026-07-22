#include "ConfigManager.h"
#include <fstream>
#include "json.hpp"
#include "logger.h"

using json = nlohmann::json;

namespace {
    Config config;
    std::string currentPath = "";
    std::vector<ConfigManager::ChangedCallback> changedCallbacks;

    void NotifyChanged() {
        for (const auto& callback : changedCallbacks) {
            callback(config);
        }
    }
}

namespace ConfigManager {

    const Config& GetConfig() { return config; }

    void RegisterChangedCallback(ChangedCallback callback) { changedCallbacks.push_back(callback); }

    void SetLanguage(const std::string& language) {
        if (config.language != language) {
            config.language = language;
            NotifyChanged();
            Save();
        }
    }

    void SetProfile(const std::string& profile) {
        if (config.profile != profile) {
            config.profile = profile;
            NotifyChanged();
            Save();
        }
    }

    void Load(const std::string& path) {
        currentPath = path;
        std::ifstream file(path);
        if (!file.is_open()) return;

        json configFile;
        try { file >> configFile; } 
        catch (const json::parse_error&) { return; }

        if (configFile.contains("Language")) config.language = configFile["Language"].get<std::string>();
        if (configFile.contains("Profile")) config.profile = configFile["Profile"].get<std::string>();

        NotifyChanged();
    }

    void Save() {
        if (currentPath.empty()) return;

        json configFile;
        std::ifstream inFile(currentPath);
        if (inFile.is_open()) {
            try { inFile >> configFile; } catch (...) {}
            inFile.close();
        }

        configFile["Language"] = config.language;
        configFile["Profile"] = config.profile;

        std::ofstream outFile(currentPath);
        if (outFile.is_open()) outFile << configFile.dump(4);
    }
}