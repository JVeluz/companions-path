#include "Language.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

#include "logger.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
    std::unordered_map<std::string, std::string> dictionary;
    std::vector<std::string> availableLanguages;
    const std::string languageDirectory = "Data/SKSE/Plugins/CompanionsPath/languages/";
}

namespace Language {

    const char* GetString(std::string_view key) {
        std::string strKey(key);
        auto it = dictionary.find(strKey);
        if (it != dictionary.end()) {
            return it->second.c_str();
        }
        dictionary[strKey] = strKey;
        return dictionary[strKey].c_str();
    }

    const std::vector<std::string>& GetLanguages() { return availableLanguages; }

    void Scan() {
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

    bool Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            logger::error("No language file found in : {}", path);
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
            return true;

        } catch (const json::parse_error& _) {
            return false;
        }
    }

}
