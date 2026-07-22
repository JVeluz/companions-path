#pragma once

namespace StatManager {
    float GetStatValue(RE::Actor *actor, RE::ActorValue actorValue);

    int GetRemainingAttributePoints(RE::Actor *actor);
    int GetRemainingSkillPoints(RE::Actor *actor);
    
    bool HasPointsLeft(RE::Actor *actor, RE::ActorValue actorValue);

    void AddPoint(RE::Actor *actor, RE::ActorValue actorValue);
    void RemovePoint(RE::Actor *actor, RE::ActorValue actorValue);
    
    void ResetAttributes(RE::Actor *actor);
    void ResetSkills(RE::Actor *actor);

    void Harmonize();
}