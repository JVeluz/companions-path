#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct Profile {
    std::vector<RE::ActorValue> Attributes;
    std::vector<RE::ActorValue> Skills;
    std::unordered_map<RE::ActorValue, float> BaseValues;
    std::vector<RE::ActorValue> All;
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

namespace Perks {

    struct PerkNode {
        std::string Name;
        RE::ActorValue AssociatedSkill;
        int MaxRanks = 1;
        
        std::vector<RE::BGSPerk*> Ranks;
        std::vector<int> RankRequirements;

        std::vector<PerkNode*> Parents;
        std::vector<PerkNode*> Children;
    };
    
    struct PerkTree {
        RE::ActorValue Skill = RE::ActorValue::kNone;
        std::vector<std::unique_ptr<PerkNode>> Nodes;
        std::vector<PerkNode*> RootNodes;
    };
}