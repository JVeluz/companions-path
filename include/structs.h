#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Config {
    std::string language = "english";
    std::string profile = "default";
};

struct Profile {
    bool harmonize = false;
    bool syncLevel = false;
    float levelMultiplier = 1.0f;
    float attributeMultiplier = 1.0f;
    float skillMultiplier = 1.0f;
    float perkMultiplier = 1.0f;
};

struct ActorProfile {
    std::vector<RE::ActorValue> attributes;
    std::vector<RE::ActorValue> skills;
    std::vector<RE::ActorValue> all;
    std::unordered_map<RE::ActorValue, float> baseValues;
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

namespace Perks {

    struct PerkNode {
        std::string name;
        std::string description;
        RE::ActorValue associatedSkill;
        int maxRanks = 1;

        std::vector<RE::BGSPerk*> ranks;
        std::vector<int> rankRequirements;

        std::vector<PerkNode*> parents;
        std::vector<PerkNode*> children;

        float horizontalPosition;
        float verticalPosition;
    };

    struct PerkTree {
        RE::ActorValue skill = RE::ActorValue::kNone;
        std::vector<std::unique_ptr<PerkNode>> nodes;
        std::vector<PerkNode*> rootNodes;
    };

}