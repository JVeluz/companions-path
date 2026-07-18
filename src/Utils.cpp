#include "Utils.h"

namespace {
    std::vector<RE::ActorHandle> activeFollowers;

    bool IsValidIndex(int index) {
        return index >= 0 && index < activeFollowers.size();
    }
}

namespace Utils {

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
}