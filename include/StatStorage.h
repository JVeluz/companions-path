#pragma once

namespace StatStorage {
    void Register();

    void SetStatPoints(RE::Actor* actor, RE::ActorValue actorValue, int points);
    int GetStatPoints(RE::Actor* actor, RE::ActorValue actorValue, int defaultPoints = 0);
    
    void ClearActor(RE::Actor* actor);
}