#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace Language {
    const char* GetString(std::string_view key);
    const std::vector<std::string>& GetLanguages();
    void Scan(const std::string& path);
    bool Load(const std::string& path);
}