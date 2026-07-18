#pragma once

#include <vector>

namespace FollowerManager {
    bool IsUniqueNPC(RE::Actor* actor);
    void RefreshFollowers();
    RE::ActorPtr GetActorPtr(int index);
    RE::Actor* GetActor(int index);
    std::vector<RE::ActorHandle> GetActiveFollowers();
};