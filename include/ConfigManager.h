#pragma once

#include <string>
#include <functional>

struct Config {
    bool harmonize = true;
    bool syncLevel = false;
    std::string currentLanguage = "english";
};

using ChangedCallback = std::function<void(const Config&)>;

namespace ConfigManager {
    const Config& GetConfig();
    
    void RegisterChangedCallback(ChangedCallback callback);

    void Load(const std::string& path);
    void Save();
    void Refresh();

    void SetLanguage(const std::string& lang);
    void SetHarmonize(bool value);
    void SetSyncLevel(bool value);
}