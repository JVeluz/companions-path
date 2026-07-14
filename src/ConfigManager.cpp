#include "ConfigManager.h"
#include "ProfileRepository.h"
#include "LanguageRepository.h"
#include "logger.h"
#include "json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace ConfigManager {
    void LoadConfig(const std::string& configPath) {
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
            std::string langOverride = config["Language"].get<std::string>();
            logger::info("Language override found in config: {}", langOverride);
            LanguageRepository::LoadLanguage(langOverride);
        }

        ProfileRepository::InitializeFromJson(config);
    }
}