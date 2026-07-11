#include "Stats.h"

std::array<RE::ActorValue, 3> Stats::Attributes = {
    RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina
};

std::array<RE::ActorValue, 6> Stats::CombatSkills = {
    RE::ActorValue::kOneHanded, RE::ActorValue::kTwoHanded, RE::ActorValue::kBlock,
    RE::ActorValue::kHeavyArmor, RE::ActorValue::kLightArmor, RE::ActorValue::kArchery
};

std::array<RE::ActorValue, 5> Stats::MagicSkills = {
    RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration,
    RE::ActorValue::kConjuration, RE::ActorValue::kIllusion
};

std::array<RE::ActorValue, 7> Stats::MiscSkills = {
    RE::ActorValue::kSneak, RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,
    RE::ActorValue::kSpeech, RE::ActorValue::kAlchemy, RE::ActorValue::kSmithing,
    RE::ActorValue::kEnchanting
};

bool find(std::span<const RE::ActorValue> array, RE::ActorValue actorValue) {
    return std::find(array.begin(), array.end(), actorValue) != array.end();
}

float GetBaseValue(RE::ActorValue actorValue) {
    if (find(Stats::Attributes, actorValue))
        return 100.f;
    return 15.f;
}

float GetStepValue(RE::ActorValue actorValue) {
    if (find(Stats::Attributes, actorValue))
        return 10.f;
    return 1.f;
}