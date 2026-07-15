#pragma once

#include "structs.h"

namespace PerkManager {
    void InitializeTrees();
    const Perks::PerkTree* GetPerkTree(RE::ActorValue skill);
    const Perks::PerkNode* GetPerkNode(RE::BGSPerk* perk);
    bool HasPrerequisites(RE::Actor* actor, const Perks::PerkNode* node);
}