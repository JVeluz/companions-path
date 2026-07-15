#include "language.h"
#include "logger.h"

#include <string>
#include <string_view>

namespace {
    std::unordered_map<std::string, std::string> dictionary;
}

namespace TranslationService {
    const char* GetString(std::string_view key) {
        std::string strKey(key);
        auto it = dictionary.find(strKey);
        if (it != dictionary.end()) {
            return it->second.c_str(); 
        }
        dictionary[strKey] = strKey;
        return dictionary[strKey].c_str();
    }
}

#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
    std::vector<std::string> availableLanguages;
    const std::string languageDirectory = "Data/SKSE/Plugins/CompanionsPath/languages/";
}

namespace LanguageRepository {

    void ScanAvailableLanguages() {
        availableLanguages.clear();
        try {
            if (fs::exists(languageDirectory) && fs::is_directory(languageDirectory)) {
                for (const auto& entry : fs::directory_iterator(languageDirectory)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".json") {
                        availableLanguages.push_back(entry.path().stem().string());
                    }
                }
                logger::info("Scanned language directory, found {} languages.", availableLanguages.size());
            } else {
                logger::error("Language directory does not exist: {}", languageDirectory);
            }
        } catch (const fs::filesystem_error& e) {
            logger::error("Filesystem error while scanning languages: {}", e.what());
        }
    }

    const std::vector<std::string>& GetAvailableLanguages() {
        return availableLanguages;
    }

    bool LoadLanguage(const std::string& languageName) {
        std::string filePath = languageDirectory + languageName + ".json";
        
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logger::error("No language file found in : {}", filePath);
            return false;
        }

        try {
            json j;
            file >> j;
            
            dictionary.clear();

            for (auto& [key, value] : j.items()) {
                if (value.is_string()) {
                    dictionary[key] = value.get<std::string>();
                }
            }
            
            logger::info("Language loaded : {}", languageName);
            return true;
            
        } catch (const json::parse_error& e) {
            logger::error("Parsing error in : {}.json : {}", languageName, e.what());
            return false;
        }
    }

}
