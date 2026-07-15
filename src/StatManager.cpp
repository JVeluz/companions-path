#include "StatManager.h"
#include "Storage.h"
#include "Rules.h"
#include "Utils.h"
#include "profile.h"

#include <algorithm>

namespace {

    int GetSpentPoints(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
        int spent = 0;
        for (const auto& actorValue : actorValues) {
            spent += Storage::Stats::GetPoints(actor, actorValue);
        }
        return spent;
    }

    void ApplyStatValue(RE::Actor* actor, RE::ActorValue actorValue, float targetValue) {
        if (Rules::Stats::IsCalculatedStat(actorValue)) {
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
        
        float value = Rules::Stats::CalculateStatValue(actor, actorValue, points);
        int maxValue = Rules::Stats::GetMaxValue(actor, actorValue);
        
        if (maxValue == -1 || value <= maxValue) {
            ApplyStatValue(actor, actorValue, value);
            Storage::Stats::SetPoints(actor, actorValue, points);
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
        int points = Storage::Stats::GetPoints(actor, actorValue);
        return Rules::Stats::CalculateStatValue(actor, actorValue, points);
    }

    int GetRemainingAttributePoints(RE::Actor* actor) { 
        int total = Rules::Stats::GetAttributePoints(actor);
        int spent = GetSpentPoints(actor, ProfileParser::GetProfile(actor).Attributes);
        return total - spent; 
    }

    int GetRemainingSkillPoints(RE::Actor* actor) {
        int total = Rules::Stats::GetSkillPoints(actor);
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
        if (Rules::Stats::IsAttribute(actor, actorValue)) {
            return GetRemainingAttributePoints(actor) > 0;
        } else {
            return GetRemainingSkillPoints(actor) > 0;
        }
    }

    void AddPoint(RE::Actor* actor, RE::ActorValue actorValue) {
        if (HasPointsLeft(actor, actorValue)) {
            int currentPoints = Storage::Stats::GetPoints(actor, actorValue);
            SetStat(actor, actorValue, currentPoints + 1);
        }
    }

    void RemovePoint(RE::Actor* actor, RE::ActorValue actorValue) {
        int currentPoints = Storage::Stats::GetPoints(actor, actorValue);
        SetStat(actor, actorValue, currentPoints - 1);
    }

    void Harmonize() {
        for (auto& handle : Utils::GetActiveFollowers()) {
            if (auto actorPtr = handle.get()) {
                if (auto actor = actorPtr.get()) {
                    auto profile = ProfileParser::GetProfile(actor);
                    
                    for (const auto& actorValue : profile.All) {
                        SetStat(actor, actorValue, Storage::Stats::GetPoints(actor, actorValue));
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