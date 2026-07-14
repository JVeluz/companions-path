#pragma once

#include <algorithm>

#include "ProfileRepository.h"

namespace StatRules {
    bool IsCalculatedStat(RE::ActorValue actorValue);
    bool IsAttribute(RE::Actor* actor, RE::ActorValue actorValue);
    
    int GetMaxValue(RE::Actor* actor, RE::ActorValue actorValue);
    float GetStepValue(RE::Actor* actor, RE::ActorValue actorValue);
    float GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue);
    
    float CalculateStatValue(RE::Actor* actor, RE::ActorValue actorValue, int points);

    int GetTotalAttributePoints(RE::Actor* actor);
    int GetTotalSkillPoints(RE::Actor* actor);
}