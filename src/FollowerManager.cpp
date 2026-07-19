#include "FollowerManager.h"
#include "ConfigManager.h"

namespace {
    std::vector<RE::ActorHandle> activeFollowers;

    bool IsValidIndex(int index) {
        return index >= 0 && index < activeFollowers.size();
    }
}

namespace FollowerManager {

    bool IsUniqueNPC(RE::Actor* actor) {
        return actor->GetActorBase()->IsUnique(); 
    }

    void RefreshFollowers() {
        auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
        auto processLists = RE::ProcessLists::GetSingleton();
        
        if (!followerFaction || !processLists) return;
    
        activeFollowers.clear();

        for (auto& actorHandle : processLists->highActorHandles) {
            auto actorPtr = actorHandle.get();
            if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
                activeFollowers.push_back(actorHandle);
            }
        }
    }

    RE::ActorPtr GetActorPtr(int index) {
        if (!IsValidIndex(index)) 
            return nullptr;
            
        return activeFollowers[index].get();
    }

    RE::Actor* GetActor(int index) {
        if (!IsValidIndex(index))
            return nullptr;
            
        auto ptr = activeFollowers[index].get(); 
        return ptr.get();
    }

    std::vector<RE::ActorHandle> GetActiveFollowers() {
        return activeFollowers;
    }

    void SyncFollowerLevels() {
        if (!ConfigManager::GetSyncLevel()) return;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        for (auto& handle : activeFollowers) {
            if (auto actorPtr = handle.get()) {
                if (auto actor = actorPtr.get()) {
                    if (auto base = actor->GetActorBase()) {
                        base->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
                        base->actorData.calcLevelMax = 0; 
                    }
                }
            }
        }
    }

}