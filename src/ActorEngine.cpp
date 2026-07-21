#include "ActorEngine.h"

namespace ActorEngine {

    bool IsUniqueNPC(RE::Actor* actor) {
        if (!actor || !actor->GetActorBase()) return false;
        return actor->GetActorBase()->IsUnique();
    }

    void SetActorMaxLevel(RE::Actor* actor, uint16_t level) {
        if (!actor) return;
        if (auto base = actor->GetActorBase()) {
            base->actorData.level = level;
        }
    }

    void SetBaseStat(RE::Actor* actor, RE::ActorValue actorValue, float value) {
        if (!actor || actorValue == RE::ActorValue::kNone) return;
        actor->AsActorValueOwner()->SetBaseActorValue(actorValue, value);
    }

    // TrumanGIT : Follower-Leveling-System-Redone/src/serialization.cpp
    void AddPerk(RE::Actor* actor, RE::BGSPerk* perk) {
        if (!actor || !perk) return;

        if (actor->HasPerk(perk)) return;

        if (auto base = actor->GetActorBase(); base) {
            if (base->AddPerk(perk, 1)) {
                for (const auto& perkEntry : perk->perkEntries) {
                    if (perkEntry) {
                        perkEntry->ApplyPerkEntry(actor);
                    }
                }
            }
        }
    }

    void RemovePerk(RE::Actor* actor, RE::BGSPerk* perk) {
        if (!actor || !perk) return;

        if (auto base = actor->GetActorBase(); base) {
            for (const auto& perkEntry : perk->perkEntries) {
                if (perkEntry) {
                    perkEntry->RemovePerkEntry(actor);
                }
            }

            base->RemovePerk(perk);
        }
    }
}