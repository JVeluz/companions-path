#pragma once
#include "structs.h"
#include <string>
#include <functional>
#include <vector>

namespace ConfigManager {
    using ChangedCallback = std::function<void(const Config&)>;

    const Config& GetConfig();
    void RegisterChangedCallback(ChangedCallback callback);

    void SetLanguage(const std::string& language);
    void SetProfile(const std::string& profile);

    void Load(const std::string& path);
    void Save();
}