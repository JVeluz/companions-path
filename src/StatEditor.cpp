#include "StatEditor.h"

static bool IsCalculatedStat(RE::ActorValue actorValue) {
    switch (actorValue) {
        case RE::ActorValue::kDamageResist:
        // case RE::ActorValue::kUnarmedDamage:
        // case RE::ActorValue::kCarryWeight:
        // case RE::ActorValue::kMeleeDamage:
        // case RE::ActorValue::kSpeedMult:
        // case RE::ActorValue::kWeaponSpeedMult:
        // case RE::ActorValue::kResistMagic:
        // case RE::ActorValue::kResistFire:
        // case RE::ActorValue::kResistFrost:
        // case RE::ActorValue::kResistShock:
        // case RE::ActorValue::kResistDisease:
        // case RE::ActorValue::kCriticalChance:
        // case RE::ActorValue::kShoutRecoveryMult:
            return true;
        default:
            return false;
    }
}

static int GetSpentPoints(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
    int spent = 0;
    for (const auto& actorValue : actorValues) {
        spent += Data::GetStat(actor, actorValue);
    }
    return spent;
}

static void ApplyStatToEngine(RE::Actor* actor, RE::ActorValue actorValue, float targetValue) {
    if (IsCalculatedStat(actorValue)) {
        float currentMod = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, actorValue);
        float diff = targetValue - currentMod;
        if (diff != 0.0f) {
            actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, actorValue, diff);
        }
    } else {
        actor->AsActorValueOwner()->SetBaseActorValue(actorValue, targetValue);
    }
}

static void SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points) {
    float baseValue = Stats::GetBaseValue(actor, actorValue);
    float step = Stats::GetStepValue(actor, actorValue);
    ApplyStatToEngine(actor, actorValue, baseValue + (points * step));
    Data::SetStat(actor, actorValue, points);
}

static void Reset(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
    for (const auto& actorValue : actorValues) {
        SetStat(actor, actorValue, 0);
    }
}

int StatEditor::GetTotalAttributePoints(RE::Actor* actor) { 
    return std::max(0, actor->GetLevel() - 1); 
}

int StatEditor::GetTotalSkillPoints(RE::Actor* actor) { 
    return std::max(0, actor->GetLevel() * 5); 
}

int StatEditor::GetRemainingAttributePoints(RE::Actor* actor) { 
    int total = GetTotalAttributePoints(actor);
    int spent = GetSpentPoints(actor, Stats::GetProfileForActor(actor).Attributes);
    return total - spent; 
}

int StatEditor::GetRemainingSkillPoints(RE::Actor* actor) {
    int total = GetTotalSkillPoints(actor);
    int spent = GetSpentPoints(actor, Stats::GetProfileForActor(actor).Skills);
    return total - spent; 
}

void StatEditor::ResetAttributes(RE::Actor* actor) { 
    Reset(actor, Stats::GetProfileForActor(actor).Attributes); 
}

void StatEditor::ResetSkills(RE::Actor* actor) { 
    Reset(actor, Stats::GetProfileForActor(actor).Skills); 
}

bool StatEditor::HasPointsLeft(RE::Actor* actor, RE::ActorValue actorValue) {
    if (Stats::IsAttribute(actor, actorValue)) {
        return StatEditor::GetRemainingAttributePoints(actor) > 0;
    } else {
        return StatEditor::GetRemainingSkillPoints(actor) > 0;
    }
}

float StatEditor::GetStat(RE::Actor* actor, RE::ActorValue actorValue) {
    int points = Data::GetStat(actor, actorValue);
    float baseValue = Stats::GetBaseValue(actor, actorValue);
    float step = Stats::GetStepValue(actor, actorValue);
    return baseValue + (points * step);
}

void StatEditor::AddPoint(RE::Actor* actor, RE::ActorValue actorValue) {
    if (StatEditor::HasPointsLeft(actor, actorValue)) {
        int currentPoints = Data::GetStat(actor, actorValue);
        int maxPoints = Stats::GetMaxPoints(actor, actorValue);
        if (maxPoints == -1 || currentPoints < maxPoints) {
            SetStat(actor, actorValue, currentPoints + 1);
        }
    }
}

void StatEditor::RemovePoint(RE::Actor* actor, RE::ActorValue actorValue) {
    int currentPoints = Data::GetStat(actor, actorValue);
    if (currentPoints > 0) {
        SetStat(actor, actorValue, currentPoints - 1);
    }
}

void StatEditor::Harmonize() {
    for (auto& handle : Utils::GetActiveFollowers()) {
        if (auto actorPtr = handle.get()) {
            if (auto actor = actorPtr.get()) {
                auto profile = Stats::GetProfileForActor(actor);
                
                for (const auto& actorValue : profile.All) {
                    SetStat(actor, actorValue, Data::GetStat(actor, actorValue));
                }

                for (const auto& [actorValue, baseValue] : profile.BaseValues) {
                    if (std::find(profile.All.begin(), profile.All.end(), actorValue) == profile.All.end()) {
                        ApplyStatToEngine(actor, actorValue, baseValue);
                    }
                }
            }
        }
    }
}