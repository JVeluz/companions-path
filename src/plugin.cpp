#include "SKSEMenuFramework.h"
#include "CompanionMenu.h"

void OnMessage(SKSE::MessagingInterface::Message* message)
{
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
        if (SKSEMenuFramework::IsInstalled()) {
            CompanionMenu::Register();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    return true;
}