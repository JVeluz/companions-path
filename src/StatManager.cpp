#include "StatManager.h"

#include <algorithm>

#include "ActorEngine.h"
#include "FollowerManager.h"
#include "Rules.h"
#include "Storage.h"
#include "profile.h"

namespace {

    int GetSpentPoints(RE::Actor* actor, std::span<const RE::ActorValue> actorValues) {
        int spent = 0;
        for (const auto& actorValue : actorValues) {
            spent += Storage::Stats::GetPoints(actor, actorValue);
        }
        return spent;
    }

    void SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points) {
        if (points < 0) return;

        float value = Rules::Stats::CalculateStatValue(actor, actorValue, points);
        int maxValue = Rules::Stats::GetMaxValue(actor, actorValue);

        if (maxValue == -1 || value <= maxValue) {
            ActorEngine::SetBaseStat(actor, actorValue, value);
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

    int GetAttributePoints(RE::Actor *actor) {
        return Rules::Stats::GetAttributePoints(actor);
    }
    
    int GetSkillPoints(RE::Actor *actor) {
        return Rules::Stats::GetSkillPoints(actor);
    }

    int GetRemainingAttributePoints(RE::Actor* actor) {
        int total = Rules::Stats::GetAttributePoints(actor);
        int spent = GetSpentPoints(actor, ProfileParser::GetProfile(actor).attributes);
        return total - spent;
    }

    int GetRemainingSkillPoints(RE::Actor* actor) {
        int total = Rules::Stats::GetSkillPoints(actor);
        int spent = GetSpentPoints(actor, ProfileParser::GetProfile(actor).skills);
        return total - spent;
    }

    void ResetAttributes(RE::Actor* actor) { Reset(actor, ProfileParser::GetProfile(actor).attributes); }

    void ResetSkills(RE::Actor* actor) { Reset(actor, ProfileParser::GetProfile(actor).skills); }

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
        for (auto actorPtr : FollowerManager::GetActorPtrs()) {
            auto actor = actorPtr.get();
            auto profile = ProfileParser::GetProfile(actor);

            for (const auto& actorValue : profile.all) {
                SetStat(actor, actorValue, Storage::Stats::GetPoints(actor, actorValue));
            }

            for (const auto& [actorValue, baseValue] : profile.baseValues) {
                if (std::find(profile.all.begin(), profile.all.end(), actorValue) == profile.all.end()) {
                    ActorEngine::SetBaseStat(actor, actorValue, baseValue);
                }
            }
        }
    }
}