#include "Utils.h"

std::vector<RE::ActorHandle> Utils::GetActiveFollowers()
{
    std::vector<RE::ActorHandle> followers;
    auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
    auto processLists = RE::ProcessLists::GetSingleton();
    
    if (!followerFaction || !processLists) return followers;

    for (auto& actorHandle : processLists->highActorHandles) {
        auto actorPtr = actorHandle.get();
        if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
            followers.push_back(actorHandle);
        }
    }
    return followers;
}