#include "Logger.h"
#include "UI.h"
#include "EventManager.h"
#include "ConfigManager.h"
#include "profile.h"
#include "language.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {

        auto setting = RE::INISettingCollection::GetSingleton()->GetSetting("sLanguage:General");
        std::string gameLanguage = setting ? setting->GetString() : "english";
        LanguageRepository::LoadLanguage(gameLanguage);

        EventManager::Register();
        
        ConfigManager::LoadConfig("Data/SKSE/Plugins/CompanionsPath/config.json");
        
        UI::Register(); 
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}