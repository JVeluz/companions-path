#include "ConfigManager.h"
#include "EventManager.h"
#include "FollowerManager.h"
#include "Logger.h"
#include "PerkManager.h"
#include "StatManager.h"
#include "Storage.h"
#include "UI.h"
#include "Language.h"
#include "profile.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
       
            auto setting = RE::INISettingCollection::GetSingleton()->GetSetting("sLanguage:General");
            std::string gameLanguage = setting ? setting->GetString() : "english";
            
            Language::Load(std::format("Data/SKSE/Plugins/CompanionsPath/languages/{}.json", gameLanguage));

            Language::Scan();
            
            ConfigManager::RegisterChangedCallback([](const Config& newConfig) {
                logger::info("Config has changed !");
                
                ProfileParser::ClearCache();
                
                if (ConfigManager::GetConfig().currentLanguage != newConfig.currentLanguage) {
                    Language::Load(std::format("Data/SKSE/Plugins/CompanionsPath/languages/{}.json", newConfig.currentLanguage));
                }
                if (newConfig.syncLevel) {
                    FollowerManager::SyncLevels();
                }
                if (newConfig.harmonize) {
                    StatManager::Harmonize();
                }
                PerkManager::Harmonize(newConfig.harmonize);
            });
            
            ConfigManager::Load("Data/SKSE/Plugins/CompanionsPath/config.json");
            ProfileRepository::Load("Data/SKSE/Plugins/CompanionsPath/profiles/default.json");

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