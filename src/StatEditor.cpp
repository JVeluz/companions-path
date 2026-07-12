#include "StatEditor.h"

static int GetSpentPoints(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
    int spent = 0;
    for (const auto& actorValue : actorValues) {
        spent += Data::GetStat(actor, actorValue);
    }
    return spent;
}

static void SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points) {
    auto owner = actor->AsActorValueOwner();
    float baseValue = Stats::GetBaseValue(actor, actorValue);
    float step = Stats::GetStepValue(actorValue);
    owner->SetBaseActorValue(actorValue, baseValue + (points * step));
    Data::SetStat(actor, actorValue, points);
}

static void Reset(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
    auto owner = actor->AsActorValueOwner();
    for (const auto& actorValue : actorValues) {
        SetStat(actor, actorValue, 0);
    }
}

int StatEditor::GetTotalAttributePoints(RE::Actor* actor) { return std::max(0, actor->GetLevel() - 1); }
int StatEditor::GetTotalSkillPoints(RE::Actor* actor) { return std::max(0, actor->GetLevel() * 5); }

int StatEditor::GetRemainingAttributePoints(RE::Actor* actor) { return GetTotalAttributePoints(actor) - GetSpentPoints(actor, Stats::Attributes); }
int StatEditor::GetRemainingSkillPoints(RE::Actor* actor) { return GetTotalSkillPoints(actor) - GetSpentPoints(actor, Stats::Skills); }

bool StatEditor::HasAttributePointsLeft(RE::Actor* actor) { return StatEditor::GetRemainingAttributePoints(actor) > 0; }
bool StatEditor::HasSkillPointsLeft(RE::Actor* actor) { return StatEditor::GetRemainingSkillPoints(actor) > 0; }

void StatEditor::ResetAttributes(RE::Actor* actor) { Reset(actor, Stats::Attributes); }
void StatEditor::ResetSkills(RE::Actor* actor) { Reset(actor, Stats::Skills); }

void StatEditor::AddPoint(RE::Actor* actor, RE::ActorValue actorValue) {
    if (Stats::IsAttribute(actorValue)) {
        if (!HasAttributePointsLeft(actor))
            return;
    } else {
        if (!HasSkillPointsLeft(actor))
            return;
    }
    int currentPoints = Data::GetStat(actor, actorValue);
    SetStat(actor, actorValue, currentPoints + 1);
}

void StatEditor::RemovePoint(RE::Actor* actor, RE::ActorValue actorValue) {
    int currentPoints = Data::GetStat(actor, actorValue);
    if (currentPoints > 0) {
        SetStat(actor, actorValue, currentPoints - 1);
    }
}

void StatEditor::Harmonize()
{
    auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
    auto processLists = RE::ProcessLists::GetSingleton();
    
    if (!followerFaction || !processLists) return;

    for (auto& actorHandle : processLists->highActorHandles) {
        auto actorPtr = actorHandle.get();
        if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
            auto actor = actorPtr.get();
            auto owner = actor->AsActorValueOwner();
            
            for (const auto& actorValue : Stats::All) {
                SetStat(actor, actorValue, Data::GetStat(actor, actorValue));
            }
        }
    }
}