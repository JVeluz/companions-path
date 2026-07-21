#pragma once

namespace ActorEngine {
    bool IsUniqueNPC(RE::Actor* actor);
    void SetActorMaxLevel(RE::Actor* actor, uint16_t level);

    void SetBaseStat(RE::Actor* actor, RE::ActorValue actorValue, float value);

    void AddPerk(RE::Actor* actor, RE::BGSPerk* perk);
    void RemovePerk(RE::Actor* actor, RE::BGSPerk* perk);
}