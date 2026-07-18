#include "Logger.h"
#include "UI.h"
#include "Utils.h"
#include "EventManager.h"
#include "StatManager.h"
#include "PerkManager.h"
#include "ConfigManager.h"
#include "profile.h"
#include "language.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
            auto setting = RE::INISettingCollection::GetSingleton()->GetSetting("sLanguage:General");
            std::string gameLanguage = setting ? setting->GetString() : "english";
            LanguageRepository::LoadLanguage(gameLanguage);
            EventManager::Register();
            PerkManager::Initialize();
            ConfigManager::LoadConfig("Data/SKSE/Plugins/CompanionsPath/config.json");
            UI::Register(); 
            break;
        }
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame: {
            Utils::RefreshFollowers(); 
            StatManager::Harmonize();
            PerkManager::Harmonize(ConfigManager::GetHarmonize());
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}