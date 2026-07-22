#include "ConfigManager.h"
#include "EventManager.h"
#include "FollowerManager.h"
#include "Language.h"
#include "Logger.h"
#include "PerkManager.h"
#include "ProfileManager.h"
#include "ProfileRepository.h"
#include "StatManager.h"
#include "Storage.h"
#include "UI.h"

#define LANGUAGES_FOLDER "Data/SKSE/Plugins/CompanionsPath/languages"
#define PROFILES_FOLDER "Data/SKSE/Plugins/CompanionsPath/profiles"
#define CONFIG_FILE "Data/SKSE/Plugins/CompanionsPath/config.json"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
            auto setting = RE::INISettingCollection::GetSingleton()->GetSetting("sLanguage:General");
            std::string gameLanguage = setting ? setting->GetString() : "english";
            Language::Load(LANGUAGES_FOLDER + std::format("/{}.json", gameLanguage));

            ConfigManager::RegisterChangedCallback([](const Config& newConfig) {
                Language::Load(LANGUAGES_FOLDER + std::format("/{}.json", newConfig.language));
                Language::Scan(LANGUAGES_FOLDER);

                ProfileManager::Load(PROFILES_FOLDER + std::format("/{}.json", newConfig.profile));
                ProfileRepository::Scan(PROFILES_FOLDER);
                UI::Settings::RefreshProfiles();

                const auto& currentProfile = ProfileManager::GetProfile();

                PerkManager::Harmonize(currentProfile.harmonize);

                if (currentProfile.harmonize) {
                    StatManager::Harmonize();
                }
            });

            ConfigManager::Load(CONFIG_FILE);

            PerkManager::Initialize();

            Storage::Register();
            EventManager::Register();
            UI::Register();

            break;
        }
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame: {
            FollowerManager::Refresh();
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}