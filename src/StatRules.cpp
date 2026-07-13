#include "StatRules.h"

namespace StatRules {

    bool IsCalculatedStat(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kDamageResist:
                return true;
            default:
                return false;
        }
    }

    bool IsAttribute(RE::Actor* actor, RE::ActorValue actorValue) {
        Profile profile = ProfileRepository::GetProfileForActor(actor);
        return std::find(profile.Attributes.begin(), profile.Attributes.end(), actorValue) != profile.Attributes.end();
    }

    int GetMaxPoints(RE::Actor* actor, RE::ActorValue actorValue) {
        if (IsAttribute(actor, actorValue)) {
            return -1;
        }
        return 100;
    }

    float GetStepValue(RE::Actor* actor, RE::ActorValue actorValue) {
        if (IsAttribute(actor, actorValue)) {
            return 10.f;
        }
        return 1.f;
    }

    float GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue) {
        Profile profile = ProfileRepository::GetProfileForActor(actor);
        
        auto it = profile.BaseValues.find(actorValue);
        if (it != profile.BaseValues.end()) {
            return it->second;
        }

        return IsAttribute(actor, actorValue) ? 100.f : 15.f;
    }

    float CalculateStatValue(RE::Actor* actor, RE::ActorValue actorValue, int points) {
        float baseValue = GetBaseValue(actor, actorValue);
        float step = GetStepValue(actor, actorValue);
        return baseValue + (points * step);
    }

    int GetTotalAttributePoints(RE::Actor* actor) { 
        return std::max(0, actor->GetLevel() - 1); 
    }

    int GetTotalSkillPoints(RE::Actor* actor) { 
        return std::max(0, actor->GetLevel() * 5); 
    }
}