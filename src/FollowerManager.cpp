#include "FollowerManager.h"

namespace {
    std::vector<RE::ActorPtr> activeFollowers;
    bool IsValidIndex(int index) { return index >= 0 && index < activeFollowers.size(); }
}

namespace FollowerManager {

    std::vector<RE::ActorPtr> GetActorPtrs() { return activeFollowers; }

    RE::Actor* GetActor(int index) {
        if (IsValidIndex(index)) return activeFollowers[index].get();
        return nullptr;
    }

    void Refresh() {
        auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
        auto processLists = RE::ProcessLists::GetSingleton();

        if (!followerFaction || !processLists) return;

        activeFollowers.clear();

        for (auto& actorHandle : processLists->highActorHandles) {
            auto actorPtr = actorHandle.get();
            if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
                activeFollowers.push_back(actorPtr);
            }
        }
    }

}