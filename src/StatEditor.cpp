#include "StatEditor.h"
#include <algorithm>
#include <array>

constexpr float ATTRIBUTE_BASE = 100.f;
constexpr float ATTRIBUTE_STEP = 10.f;
constexpr float SKILL_BASE = 15.f;
constexpr float SKILL_STEP = 1.f;

constexpr std::array<RE::ActorValue, 3> ATTRIBUTES = {
    RE::ActorValue::kHealth,
    RE::ActorValue::kMagicka,
    RE::ActorValue::kStamina
};

constexpr std::array<RE::ActorValue, 12> SKILLS = {
    RE::ActorValue::kOneHanded, RE::ActorValue::kBlock, RE::ActorValue::kHeavyArmor,
    RE::ActorValue::kArchery, RE::ActorValue::kLightArmor, RE::ActorValue::kTwoHanded,
    RE::ActorValue::kSneak, RE::ActorValue::kDestruction, RE::ActorValue::kRestoration,
    RE::ActorValue::kConjuration, RE::ActorValue::kIllusion, RE::ActorValue::kAlteration
};

bool isAttribute(RE::ActorValue actorValue) 
{
    return std::find(ATTRIBUTES.begin(), ATTRIBUTES.end(), actorValue) != ATTRIBUTES.end();
}

int CalculateSpentAttributePoints(RE::Actor *actor)
{
    auto owner = actor->AsActorValueOwner();
    float spentPoints = 0.0f;
    for (const auto& attr : ATTRIBUTES) {
        spentPoints += owner->GetBaseActorValue(attr);
    }
    return static_cast<int>((spentPoints - (ATTRIBUTES.size() * ATTRIBUTE_BASE)) / ATTRIBUTE_STEP);
}

int CalculateSpentSkillPoints(RE::Actor *actor)
{
    auto owner = actor->AsActorValueOwner();
    float spentPoints = 0.0f;
    for (const auto& skill : SKILLS) {
        spentPoints += owner->GetBaseActorValue(skill);
    }
    return static_cast<int>(spentPoints - (SKILLS.size() * SKILL_BASE));
}

int StatEditor::GetMaxAttributePoints(RE::Actor *actor)
{
    return std::max(0, actor->GetLevel() - 1);
}

int StatEditor::GetMaxSkillPoints(RE::Actor *actor)
{
    return std::max(0, actor->GetLevel() * 5);
}

int StatEditor::GetRemainingAttributePoints(RE::Actor *actor)
{
    return GetMaxAttributePoints(actor) - CalculateSpentAttributePoints(actor);
}

int StatEditor::GetRemainingSkillPoints(RE::Actor *actor)
{
    return GetMaxSkillPoints(actor) - CalculateSpentSkillPoints(actor);
}

bool StatEditor::HasAttributePointsLeft(RE::Actor *actor)
{
    return StatEditor::GetRemainingAttributePoints(actor) > 0;
}

bool StatEditor::HasSkillPointsLeft(RE::Actor *actor)
{
    return StatEditor::GetRemainingSkillPoints(actor) > 0;
}

void StatEditor::AddPoint(RE::Actor *actor, RE::ActorValue actorValue)
{
    auto owner = actor->AsActorValueOwner();
    float current = owner->GetBaseActorValue(actorValue);

    if (isAttribute(actorValue)) {
        if (HasAttributePointsLeft(actor)) {
            owner->SetBaseActorValue(actorValue, current + ATTRIBUTE_STEP);
        }
    } else {
        if (HasSkillPointsLeft(actor)) {
            owner->SetBaseActorValue(actorValue, current + SKILL_STEP);
        }
    }
}

void StatEditor::RemovePoint(RE::Actor *actor, RE::ActorValue actorValue)
{
    auto owner = actor->AsActorValueOwner();
    float current = owner->GetBaseActorValue(actorValue);
    
    // BUG CORRIGÉ : On limite la baisse à la BASE, pas à zéro
    if (isAttribute(actorValue))
        owner->SetBaseActorValue(actorValue, std::max(ATTRIBUTE_BASE, current - ATTRIBUTE_STEP));
    else
        owner->SetBaseActorValue(actorValue, std::max(SKILL_BASE, current - SKILL_STEP));
}

void StatEditor::ResetAttributes(RE::Actor *actor)
{
    auto owner = actor->AsActorValueOwner();
    for (const auto& attr : ATTRIBUTES) {
        owner->SetBaseActorValue(attr, ATTRIBUTE_BASE);
    }
}

void StatEditor::ResetSkills(RE::Actor *actor)
{
    auto owner = actor->AsActorValueOwner();
    for (const auto& skill : SKILLS) {
        owner->SetBaseActorValue(skill, SKILL_BASE);
    }
}