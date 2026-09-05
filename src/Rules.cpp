#include "Rules.h"

#include <algorithm>

#include "ProfileManager.h"
#include "structs.h"

namespace Rules {

    int GetLevel(RE::Actor* actor) {
        if (ProfileManager::GetProfile().syncLevel) {
            auto player = RE::PlayerCharacter::GetSingleton();
            int baseLevel = player ? player->GetLevel() : actor->GetLevel();
            return static_cast<int>(std::round(baseLevel * ProfileManager::GetProfile().levelMultiplier));
        }
        return static_cast<int>(std::round(actor->GetLevel() * ProfileManager::GetProfile().levelMultiplier));
    }

    namespace Stats {

        bool IsCalculatedStat(RE::ActorValue actorValue) {
            switch (actorValue) {
                case RE::ActorValue::kDamageResist:
                    return true;
                default:
                    return false;
            }
        }

        bool IsAttribute(RE::Actor* actor, RE::ActorValue actorValue) {
            ActorProfile profile = ProfileManager::GetActorProfile(actor);
            return std::find(profile.attributes.begin(), profile.attributes.end(), actorValue) != profile.attributes.end();
        }

        int GetMaxValue(RE::Actor* actor, RE::ActorValue actorValue) {
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
            ActorProfile profile = ProfileManager::GetActorProfile(actor);

            auto it = profile.baseValues.find(actorValue);
            if (it != profile.baseValues.end()) {
                return it->second;
            }

            if (!ProfileManager::GetProfile().harmonizeStats) {
                if (auto baseNPC = actor->GetActorBase()) {
                    return baseNPC->GetActorValue(actorValue);
                }
            }

            return IsAttribute(actor, actorValue) ? 100.f : 15.f;
        }

        float CalculateStatValue(RE::Actor* actor, RE::ActorValue actorValue, int points) {
            float baseValue = GetBaseValue(actor, actorValue);
            float step = GetStepValue(actor, actorValue);
            return baseValue + (points * step);
        }

        int GetAttributePoints(RE::Actor* actor) { return static_cast<int>(std::round(std::max(0, GetLevel(actor) - 1) * ProfileManager::GetProfile().attributeMultiplier)); }

        int GetSkillPoints(RE::Actor* actor) { return static_cast<int>(std::round(std::max(0, GetLevel(actor) * 5) * ProfileManager::GetProfile().skillMultiplier)); }
    }

    namespace Perks {
        int GetPoints(RE::Actor* actor) { return static_cast<int>(std::round(std::max(0, GetLevel(actor) - 1) * ProfileManager::GetProfile().perkMultiplier)); }
    }
}
