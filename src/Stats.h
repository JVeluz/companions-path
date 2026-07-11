#include <array>
#include <span>

class Stats
{
public:
    static std::array<RE::ActorValue, 3> Attributes;
    static std::array<RE::ActorValue, 6> CombatSkills;
    static std::array<RE::ActorValue, 5> MagicSkills;
    static std::array<RE::ActorValue, 7> MiscSkills;

    float GetBaseValue(RE::ActorValue actorValue);
    float GetStepValue(RE::ActorValue actorValue);
};