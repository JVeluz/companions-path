#pragma once

#include <string>

namespace ConfigManager {
    void LoadConfig(const std::string& configPath);
    void SaveConfig();

    std::string GetLanguage();
    void SetLanguage(const std::string& lang);
    
    bool GetHarmonize();
    void SetHarmonize(bool value);
}