#include "SKSEMenuFramework.h"
#include "StatEditor.h"
#include "Menu.h"
#include "Data.h"
#include "LevelUpEventSink.h"

void OnMessage(SKSE::MessagingInterface::Message* message)
{
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
        if (SKSEMenuFramework::IsInstalled()) {
            Menu::Register();
        }
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        LevelUpEventSink::GetSingleton()->Register();
        Stats::Initialize("Data/SKSE/Plugins/CompanionsPath/config.json");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    Data::Register();

    return true;
}