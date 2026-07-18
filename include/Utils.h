#pragma once

#include <vector>

namespace Utils {
    void RefreshFollowers();
    RE::ActorPtr GetActorPtr(int index);
    RE::Actor* GetActor(int index);
    std::vector<RE::ActorHandle> GetActiveFollowers();
};