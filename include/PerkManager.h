#pragma once

#include "structs.h"

namespace PerkManager {
    void Initialize();

    const Perks::PerkTree* GetPerkTree(RE::ActorValue skill);
    const Perks::PerkNode* GetPerkNode(RE::BGSPerk* perk);
    
    int GetPoints(RE::Actor* actor);
    int GetRemainingPoints(RE::Actor *actor);
    int GetCurrentRank(RE::Actor* actor, const Perks::PerkNode* node);
    int GetNextRequirement(RE::Actor* actor, const Perks::PerkNode* node);
    
    bool HasPrerequisites(RE::Actor* actor, const Perks::PerkNode* node);
    bool IsMaxedOut(RE::Actor* actor, const Perks::PerkNode* node);
    bool CanPurchase(RE::Actor* actor, const Perks::PerkNode* node);
    bool CanRefund(RE::Actor* actor, const Perks::PerkNode* node);
    
    void Purchase(RE::Actor* actor, const Perks::PerkNode* node);
    void Refund(RE::Actor* actor, const Perks::PerkNode* node);
    void RefundTree(RE::Actor *actor, const Perks::PerkNode* node);
}