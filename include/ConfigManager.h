#pragma once

#include <string>

namespace ConfigManager {
    void LoadConfig(const std::string& configPath);
    bool GetHarmonize();
}