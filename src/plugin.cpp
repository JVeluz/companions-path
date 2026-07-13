#include "Plugin.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        EventManager::Register();
        ProfileRepository::Initialize("Data/SKSE/Plugins/CompanionsPath/config.json");
    }
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
    }
}


SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    SetupLog();
    logger::info("Plugin loaded");
    UI::Register();
    return true;
}