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
        RE::BGSPerk* Form = nullptr;                // Pointeur vers le Perk original dans le jeu
        std::string Name;                           // Nom récupéré pour un accès facile
        RE::ActorValue AssociatedSkill;             // Le skill auquel il appartient
        int RequiredSkillLevel = 0;                 // Niveau requis dans le skill
        int MaxRanks = 1;                           // Nombre de fois qu'on peut l'acheter

        std::vector<PerkNode*> Parents;             // Les perks qu'il faut posséder AVANT
        std::vector<PerkNode*> Children;            // Les perks débloqués PAR celui-ci
    };

    struct PerkTree {
        RE::ActorValue Skill = RE::ActorValue::kNone;
        
        // On possède les noeuds ici pour gérer la mémoire facilement
        std::vector<std::unique_ptr<PerkNode>> Nodes; 
        
        // Pointers rapides vers les points d'entrée de l'arbre (ceux sans parents)
        std::vector<PerkNode*> RootNodes; 
    };
}