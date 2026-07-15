#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace TranslationService {
    const char* GetString(std::string_view key);
}

namespace LanguageRepository {
    void ScanAvailableLanguages();
    const std::vector<std::string>& GetAvailableLanguages();
    bool LoadLanguage(const std::string& languageName);
}