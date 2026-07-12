#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <span>

#include "Utils.h"
#include "Stats.h"
#include "Data.h"

class StatEditor
{
public:
    static int GetTotalAttributePoints(RE::Actor *actor);
    static int GetTotalSkillPoints(RE::Actor *actor);
    
    static int GetRemainingAttributePoints(RE::Actor *actor);
    static int GetRemainingSkillPoints(RE::Actor *actor);
    
    static bool HasPointsLeft(RE::Actor *actor, RE::ActorValue actorValue);

    static float GetStat(RE::Actor *actor, RE::ActorValue actorValue);

    static void AddPoint(RE::Actor *actor, RE::ActorValue actorValue);
    static void RemovePoint(RE::Actor *actor, RE::ActorValue actorValue);
    
    static void ResetAttributes(RE::Actor *actor);
    static void ResetSkills(RE::Actor *actor);

    static void Harmonize();
};