#include "LanguageRepository.h"
#include "logger.h"

#include <unordered_map>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace {
    std::unordered_map<std::string, std::string> dictionary;
}

namespace LanguageRepository {

    bool LoadLanguage(const std::string& languageName) {
        std::string filePath = "Data/SKSE/Plugins/CompanionsPath/languages/" + languageName + ".json";
        
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