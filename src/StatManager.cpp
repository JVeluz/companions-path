#include "StatManager.h"
#include "StatRules.h"
#include "StatStorage.h"
#include "Utils.h"
#include "profile.h"

#include <algorithm>

namespace {

    int GetSpentPoints(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
        int spent = 0;
        for (const auto& actorValue : actorValues) {
            spent += StatStorage::GetStatPoints(actor, actorValue);
        }
        return spent;
    }

    void ApplyStatValue(RE::Actor* actor, RE::ActorValue actorValue, float targetValue) {
        if (StatRules::IsCalculatedStat(actorValue)) {
            float currentMod = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, actorValue);
            float diff = targetValue - currentMod;
            if (diff != 0.0f) {
                actor->AsActorValueOwner()->RestoreActorValue(actorValue, diff);
            }
        } else {
            actor->AsActorValueOwner()->SetBaseActorValue(actorValue, targetValue);
        }
    }

    void SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points) {
        if (points < 0) return;
        
        float value = StatRules::CalculateStatValue(actor, actorValue, points);
        int maxValue = StatRules::GetMaxValue(actor, actorValue);
        
        if (maxValue == -1 || value <= maxValue) {
            ApplyStatValue(actor, actorValue, value);
            StatStorage::SetStatPoints(actor, actorValue, points);
        }
    }

    void Reset(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
        for (const auto& actorValue : actorValues) {
            SetStat(actor, actorValue, 0);
        }
    }
}

namespace StatManager {

    float GetStatValue(RE::Actor* actor, RE::ActorValue actorValue) {
        int points = StatStorage::GetStatPoints(actor, actorValue);
        return StatRules::CalculateStatValue(actor, actorValue, points);
    }

    int GetRemainingAttributePoints(RE::Actor* actor) { 
        int total = StatRules::GetTotalAttributePoints(actor);
        int spent = GetSpentPoints(actor, ProfileParser::GetProfile(actor).Attributes);
        return total - spent; 
    }

    int GetRemainingSkillPoints(RE::Actor* actor) {
        int total = StatRules::GetTotalSkillPoints(actor);
        int spent = GetSpentPoints(actor, ProfileParser::GetProfile(actor).Skills);
        return total - spent; 
    }

    void ResetAttributes(RE::Actor* actor) { 
        Reset(actor, ProfileParser::GetProfile(actor).Attributes); 
    }

    void ResetSkills(RE::Actor* actor) { 
        Reset(actor, ProfileParser::GetProfile(actor).Skills); 
    }

    bool HasPointsLeft(RE::Actor* actor, RE::ActorValue actorValue) {
        if (StatRules::IsAttribute(actor, actorValue)) {
            return GetRemainingAttributePoints(actor) > 0;
        } else {
            return GetRemainingSkillPoints(actor) > 0;
        }
    }

    void AddPoint(RE::Actor* actor, RE::ActorValue actorValue) {
        if (HasPointsLeft(actor, actorValue)) {
            int currentPoints = StatStorage::GetStatPoints(actor, actorValue);
            SetStat(actor, actorValue, currentPoints + 1);
        }
    }

    void RemovePoint(RE::Actor* actor, RE::ActorValue actorValue) {
        int currentPoints = StatStorage::GetStatPoints(actor, actorValue);
        SetStat(actor, actorValue, currentPoints - 1);
    }

    void Harmonize() {
        for (auto& handle : Utils::GetActiveFollowers()) {
            if (auto actorPtr = handle.get()) {
                if (auto actor = actorPtr.get()) {
                    auto profile = ProfileParser::GetProfile(actor);
                    
                    for (const auto& actorValue : profile.All) {
                        SetStat(actor, actorValue, StatStorage::GetStatPoints(actor, actorValue));
                    }

                    for (const auto& [actorValue, baseValue] : profile.BaseValues) {
                        if (std::find(profile.All.begin(), profile.All.end(), actorValue) == profile.All.end()) {
                            ApplyStatValue(actor, actorValue, baseValue);
                        }
                    }
                }
            }
        }
    }
}