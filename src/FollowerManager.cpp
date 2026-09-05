#include "FollowerManager.h"

namespace {
    std::vector<RE::ActorPtr> activeFollowers;
    bool IsValidIndex(int index) { return index >= 0 && index < activeFollowers.size(); }

    bool IsFollowerFaction(RE::ActorPtr actorPtr) {
        auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
        return actorPtr && actorPtr->IsInFaction(followerFaction);
    }
    
    bool IsPlayerTeammate(RE::ActorPtr actorPtr) {
        return actorPtr && actorPtr->IsPlayerTeammate();
    }
}

namespace FollowerManager {

    std::vector<RE::ActorPtr> GetActorPtrs() { return activeFollowers; }

    RE::Actor* GetActor(int index) {
        if (IsValidIndex(index)) return activeFollowers[index].get();
        return nullptr;
    }

    void Refresh() {
        auto processLists = RE::ProcessLists::GetSingleton();

        if (!processLists) return;

        activeFollowers.clear();

        for (auto& actorHandle : processLists->highActorHandles) {
            auto actorPtr = actorHandle.get();
            if (IsPlayerTeammate(actorPtr) || IsFollowerFaction(actorPtr)) {
                activeFollowers.push_back(actorPtr);
            }
        }
    }

}