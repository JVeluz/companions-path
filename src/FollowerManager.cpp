#include "FollowerManager.h"

namespace {
    std::vector<RE::ActorPtr> activeFollowers;
    bool IsValidIndex(int index) { return index >= 0 && index < activeFollowers.size(); }
}

namespace FollowerManager {

    std::vector<RE::ActorPtr> GetActorPtrs() { return activeFollowers; }

    RE::Actor* GetActor(int index) {
        if (IsValidIndex(index)) 
            return activeFollowers[index].get();
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

    void SyncLevels() {
        // auto player = RE::PlayerCharacter::GetSingleton();

        for (auto& ptr : activeFollowers) {
            if (auto actor = ptr.get()) {
                auto base = actor->GetActorBase();
                base->actorData.level = 1000;
                // base->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
                // base->actorData.calcLevelMax = 0;
            }
        }
    }

}