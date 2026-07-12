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
    for (const auto& actorValue : actorValues) {
        SetStat(actor, actorValue, 0);
    }
}

int StatEditor::GetTotalAttributePoints(RE::Actor* actor) { return std::max(0, actor->GetLevel() - 1); }
int StatEditor::GetTotalSkillPoints(RE::Actor* actor) { return std::max(0, actor->GetLevel() * 5); }

int StatEditor::GetRemainingAttributePoints(RE::Actor* actor) { return GetTotalAttributePoints(actor) - GetSpentPoints(actor, Stats::Attributes); }
int StatEditor::GetRemainingSkillPoints(RE::Actor* actor) { return GetTotalSkillPoints(actor) - GetSpentPoints(actor, Stats::Skills); }

void StatEditor::ResetAttributes(RE::Actor* actor) { Reset(actor, Stats::Attributes); }
void StatEditor::ResetSkills(RE::Actor* actor) { Reset(actor, Stats::Skills); }

bool StatEditor::HasPointsLeft(RE::Actor* actor, RE::ActorValue actorValue)
{
    if (Stats::IsAttribute(actorValue)) {
        return StatEditor::GetRemainingAttributePoints(actor) > 0;
    } else {
        return StatEditor::GetRemainingSkillPoints(actor) > 0;
    }
}

void StatEditor::AddPoint(RE::Actor* actor, RE::ActorValue actorValue) {
    if (StatEditor::HasPointsLeft(actor, actorValue)) {
        int currentPoints = Data::GetStat(actor, actorValue);
        int maxPoints = Stats::GetMaxPoints(actorValue);
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
                for (const auto& actorValue : Stats::All) {
                    SetStat(actor, actorValue, Data::GetStat(actor, actorValue));
                }
            }
        }
    }
}