#pragma once

#include <string>
#include <string_view>

namespace LanguageRepository {
    bool LoadLanguage(const std::string& languageName);
    const char* GetString(std::string_view key);
}