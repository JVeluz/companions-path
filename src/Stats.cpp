#include "Stats.h"

std::array<RE::ActorValue, 21> Stats::All = {
    RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina,
    RE::ActorValue::kOneHanded, RE::ActorValue::kTwoHanded, RE::ActorValue::kBlock,
    RE::ActorValue::kHeavyArmor, RE::ActorValue::kLightArmor, RE::ActorValue::kArchery,
    RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration,
    RE::ActorValue::kConjuration, RE::ActorValue::kIllusion,
    RE::ActorValue::kSneak, RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,
    RE::ActorValue::kSpeech, RE::ActorValue::kAlchemy, RE::ActorValue::kSmithing,
    RE::ActorValue::kEnchanting
};

std::array<RE::ActorValue, 3> Stats::Attributes = {
    RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina
};

std::array<RE::ActorValue, 18> Stats::Skills = {
    RE::ActorValue::kOneHanded, RE::ActorValue::kTwoHanded, RE::ActorValue::kBlock,
    RE::ActorValue::kHeavyArmor, RE::ActorValue::kLightArmor, RE::ActorValue::kArchery,
    RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration,
    RE::ActorValue::kConjuration, RE::ActorValue::kIllusion,
    RE::ActorValue::kSneak, RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,
    RE::ActorValue::kSpeech, RE::ActorValue::kAlchemy, RE::ActorValue::kSmithing,
    RE::ActorValue::kEnchanting
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

static bool Find(std::span<const RE::ActorValue> array, RE::ActorValue actorValue) {
    return std::find(array.begin(), array.end(), actorValue) != array.end();
}

bool Stats::IsAttribute(RE::ActorValue actorValue) {
    return Find(Stats::Attributes, actorValue);
}

int Stats::GetMaxPoints(RE::ActorValue actorValue) {
    if (IsAttribute(actorValue))
        return -1;
    return 100;
}

float Stats::GetStepValue(RE::ActorValue actorValue) {
    if (IsAttribute(actorValue))
        return 10.f;
    return 1.f;
}

float Stats::GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue) {
    float defaultBase = IsAttribute(actorValue) ? 100.f : 15.f;

    if (!actor || !actor->GetRace()) {
        return defaultBase;
    }

    std::string_view raceName = actor->GetRace()->GetFormEditorID();

    if (IsAttribute(actorValue)) {
        if (actorValue == RE::ActorValue::kMagicka && raceName.find("HighElf") != std::string_view::npos) {
            return 150.f;
        }
        return 100.f;
    }

    if (raceName.find("Argonian") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kLockpicking) return 25.f;
        if (actorValue == RE::ActorValue::kAlteration || actorValue == RE::ActorValue::kLightArmor || 
            actorValue == RE::ActorValue::kPickpocket || actorValue == RE::ActorValue::kRestoration || 
            actorValue == RE::ActorValue::kSneak) return 20.f;
    } 
    else if (raceName.find("Breton") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kConjuration) return 25.f;
        if (actorValue == RE::ActorValue::kAlchemy || actorValue == RE::ActorValue::kAlteration || 
            actorValue == RE::ActorValue::kIllusion || actorValue == RE::ActorValue::kRestoration || 
            actorValue == RE::ActorValue::kSpeech) return 20.f;
    } 
    else if (raceName.find("DarkElf") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kDestruction) return 25.f;
        if (actorValue == RE::ActorValue::kAlchemy || actorValue == RE::ActorValue::kAlteration || 
            actorValue == RE::ActorValue::kIllusion || actorValue == RE::ActorValue::kLightArmor || 
            actorValue == RE::ActorValue::kSneak) return 20.f;
    } 
    else if (raceName.find("HighElf") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kIllusion) return 25.f;
        if (actorValue == RE::ActorValue::kAlteration || actorValue == RE::ActorValue::kConjuration || 
            actorValue == RE::ActorValue::kDestruction || actorValue == RE::ActorValue::kEnchanting || 
            actorValue == RE::ActorValue::kRestoration) return 20.f;
    } 
    else if (raceName.find("Imperial") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kRestoration) return 25.f;
        if (actorValue == RE::ActorValue::kBlock || actorValue == RE::ActorValue::kDestruction || 
            actorValue == RE::ActorValue::kEnchanting || actorValue == RE::ActorValue::kHeavyArmor || 
            actorValue == RE::ActorValue::kOneHanded) return 20.f;
    } 
    else if (raceName.find("Khajiit") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kSneak) return 25.f;
        if (actorValue == RE::ActorValue::kAlchemy || actorValue == RE::ActorValue::kArchery || 
            actorValue == RE::ActorValue::kLockpicking || actorValue == RE::ActorValue::kPickpocket || 
            actorValue == RE::ActorValue::kOneHanded) return 20.f;
    } 
    else if (raceName.find("Nord") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kTwoHanded) return 25.f;
        if (actorValue == RE::ActorValue::kBlock || actorValue == RE::ActorValue::kLightArmor || 
            actorValue == RE::ActorValue::kOneHanded || actorValue == RE::ActorValue::kSmithing || 
            actorValue == RE::ActorValue::kSpeech) return 20.f;
    } 
    else if (raceName.find("Orc") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kHeavyArmor) return 25.f;
        if (actorValue == RE::ActorValue::kBlock || actorValue == RE::ActorValue::kEnchanting || 
            actorValue == RE::ActorValue::kOneHanded || actorValue == RE::ActorValue::kSmithing || 
            actorValue == RE::ActorValue::kTwoHanded) return 20.f;
    } 
    else if (raceName.find("Redguard") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kOneHanded) return 25.f;
        if (actorValue == RE::ActorValue::kAlteration || actorValue == RE::ActorValue::kArchery || 
            actorValue == RE::ActorValue::kBlock || actorValue == RE::ActorValue::kDestruction || 
            actorValue == RE::ActorValue::kSmithing) return 20.f;
    } 
    else if (raceName.find("WoodElf") != std::string_view::npos) {
        if (actorValue == RE::ActorValue::kArchery) return 25.f;
        if (actorValue == RE::ActorValue::kAlchemy || actorValue == RE::ActorValue::kLightArmor || 
            actorValue == RE::ActorValue::kLockpicking || actorValue == RE::ActorValue::kPickpocket || 
            actorValue == RE::ActorValue::kSneak) return 20.f;
    }

    return defaultBase;
}