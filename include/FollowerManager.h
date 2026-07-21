#pragma once

#include <vector>

namespace FollowerManager {
    std::vector<RE::ActorPtr> GetActorPtrs();
    RE::Actor* GetActor(int index);
    void Refresh();
    void SyncLevels();
};