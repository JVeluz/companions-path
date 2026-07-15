#include "UI.h"
#include "ConfigManager.h"
#include "language.h"

#include <vector>
#include <string>
#include <algorithm>

namespace UI {
    namespace Settings {
        void __stdcall Render() {
            
            bool harmonize = ConfigManager::GetHarmonize();
            if (ImGuiMCP::Checkbox(TranslationService::GetString("UI_SETTING_HARMONIZE"), &harmonize)) {
                ConfigManager::SetHarmonize(harmonize);
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            static int currentLangIndex = -1;

            if (ImGuiMCP::Button(TranslationService::GetString("UI_REFRESH_LANGUAGES"))) {
                LanguageRepository::ScanAvailableLanguages();
                currentLangIndex = -1; 
            }

            ImGuiMCP::Spacing();

            const auto& availableLanguages = LanguageRepository::GetAvailableLanguages();
            
            if (!availableLanguages.empty()) {
                if (currentLangIndex == -1) {
                    std::string currentLang = ConfigManager::GetLanguage();
                    auto it = std::find(availableLanguages.begin(), availableLanguages.end(), currentLang);
                    if (it != availableLanguages.end()) {
                        currentLangIndex = static_cast<int>(std::distance(availableLanguages.begin(), it));
                    } else {
                        currentLangIndex = 0;
                    }
                }

                std::vector<const char*> langItems;
                for (const auto& lang : availableLanguages) {
                    langItems.push_back(lang.c_str());
                }

                if (ImGuiMCP::Combo(TranslationService::GetString("UI_SETTING_LANGUAGE"), &currentLangIndex, langItems.data(), static_cast<int>(langItems.size()))) {
                    ConfigManager::SetLanguage(availableLanguages[currentLangIndex]);
                }
            } else {
                ImGuiMCP::Text("No language files found.");
            }
        }
    }
}