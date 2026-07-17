#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct Profile {
    std::vector<RE::ActorValue> attributes;
    std::vector<RE::ActorValue> skills;
    std::unordered_map<RE::ActorValue, float> baseValues;
    std::vector<RE::ActorValue> all;
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

namespace Perks {

    struct PerkNode {
        std::string name;
        RE::ActorValue associatedSkill;
        int maxRanks = 1;
        
        std::vector<RE::BGSPerk*> ranks;
        std::vector<int> rankRequirements;

        std::vector<PerkNode*> parents;
        std::vector<PerkNode*> children;
    };
    
    struct PerkTree {
        RE::ActorValue skill = RE::ActorValue::kNone;
        std::vector<std::unique_ptr<PerkNode>> nodes;
        std::vector<PerkNode*> rootNodes;
    };
}