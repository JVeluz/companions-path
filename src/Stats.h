#pragma once

#include <array>
#include <string_view>

class Stats
{
public:
    static std::array<RE::ActorValue, 21> All;
    static std::array<RE::ActorValue, 3> Attributes;
    static std::array<RE::ActorValue, 18> Skills;
    static std::array<RE::ActorValue, 6> CombatSkills;
    static std::array<RE::ActorValue, 5> MagicSkills;
    static std::array<RE::ActorValue, 7> MiscSkills;
    static bool IsAttribute(RE::ActorValue actorValue);
    static int GetMaxPoints(RE::ActorValue actorValue);
    static float GetStepValue(RE::ActorValue actorValue);
    static float GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue);
};