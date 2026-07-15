#include "PerkManager.h"
#include "RE/A/ActorValueList.h"
#include "RE/A/ActorValueInfo.h"
#include "RE/B/BGSSkillPerkTreeNode.h"
#include "RE/B/BGSPerk.h"

#include <unordered_map>

namespace {
    std::unordered_map<RE::ActorValue, Perks::PerkTree> perkTrees;
    std::unordered_map<RE::FormID, Perks::PerkNode*> nodeIndex;

    int GetMaxRanks(RE::BGSPerk* perk) {
        int ranks = 1;
        RE::BGSPerk* current = perk;
        while (current->nextPerk) {
            ranks++;
            current = current->nextPerk;
        }
        return ranks;
    }

    int GetSkillLevelRequirement(RE::BGSPerk* perk) {
        if (!perk || !perk->perkConditions.head) return 0;

        int reqLevel = 0;
        auto currentCondition = perk->perkConditions.head;
        
        while (currentCondition) {
            uint16_t funcID = static_cast<uint16_t>(currentCondition->data.functionData.function.get());
            float compVal = currentCondition->data.comparisonValue.f;
            
            // 71 = GetActorValue | 73 = GetBaseActorValue | 277 = La fonction specifique utilisee pour les Perks
            if (funcID == 71 || funcID == 73 || funcID == 277) {
                int val = static_cast<int>(compVal);
                if (val > reqLevel) {
                    reqLevel = val;
                }
            }
            currentCondition = currentCondition->next;
        }
        
        return reqLevel;
    }

    // Fonction récursive pour parcourir l'arbre natif de Skyrim
    void TraverseNode(
        RE::BGSSkillPerkTreeNode* nativeNode, 
        RE::ActorValue av, 
        Perks::PerkTree& tree, 
        std::unordered_map<RE::BGSSkillPerkTreeNode*, Perks::PerkNode*>& nodeMap, 
        int depth = 0, 
        Perks::PerkNode* parentNode = nullptr
    ) {
        if (!nativeNode) {
            logger::info("[Profondeur {}] TraverseNode appele avec un nativeNode null.", depth);
            return;
        }

        // Si on a déjà traité ce noeud, on s'arrête (évite les boucles infinies)
        if (nodeMap.find(nativeNode) != nodeMap.end()) {
            logger::info("[Profondeur {}] Noeud deja visite, on arrete cette branche.", depth);
            return;
        }

        Perks::PerkNode* currentCustom = nullptr;

        if (nativeNode->perk) {
            auto customNode = std::make_unique<Perks::PerkNode>();
            customNode->Form = nativeNode->perk;
            customNode->Name = nativeNode->perk->GetFullName();
            customNode->AssociatedSkill = av;
            customNode->RequiredSkillLevel = GetSkillLevelRequirement(nativeNode->perk);
            customNode->MaxRanks = GetMaxRanks(nativeNode->perk);

            currentCustom = customNode.get();

            logger::info("[Profondeur {}] Perk trouve : '{}' (SkillLevel: {}, MaxRanks: {}, ID: {:08X})", 
                depth, currentCustom->Name, currentCustom->RequiredSkillLevel, customNode->MaxRanks, currentCustom->Form->GetFormID());

            if (parentNode) {
                currentCustom->Parents.push_back(parentNode);
                parentNode->Children.push_back(currentCustom);
                logger::info("[Profondeur {}] -> Parent assigne : '{}'", depth, parentNode->Name);
            }

            nodeMap[nativeNode] = currentCustom;
            nodeIndex[nativeNode->perk->GetFormID()] = currentCustom;
            tree.Nodes.push_back(std::move(customNode));
        } else {
            logger::info("[Profondeur {}] Noeud factice (Dummy) detecte (aucun Perk associe).", depth);
            nodeMap[nativeNode] = nullptr;
        }

        Perks::PerkNode* nextParent = currentCustom ? currentCustom : parentNode;

        // 2. Parcours des enfants
        logger::info("[Profondeur {}] Exploration de {} enfant(s)...", depth, nativeNode->children.size());
        
        for (auto* childNative : nativeNode->children) {
            if (childNative) {
                TraverseNode(childNative, av, tree, nodeMap, depth + 1, nextParent);
            } else {
                logger::info("[Profondeur {}] -> Pointeur d'enfant null detecte dans le tableau.", depth);
            }
        }
    }
}

namespace PerkManager {

    void InitializeTrees() {
        logger::info("=== DEBUT INITIALISATION DES ARBRES DE PERKS ===");
        perkTrees.clear();
        nodeIndex.clear();

        auto* actorValueList = RE::ActorValueList::GetSingleton();
        if (!actorValueList) {
            logger::info("ERREUR CRITIQUE : ActorValueList::GetSingleton() a retourne null !");
            return;
        }

        uint32_t totalAv = static_cast<uint32_t>(RE::ActorValue::kTotal);
        logger::info("Scan de {} ActorValues...", totalAv);

        for (uint32_t i = 0; i < totalAv; ++i) {
            auto av = static_cast<RE::ActorValue>(i);
            RE::ActorValueInfo* avInfo = actorValueList->GetActorValueInfo(av);

            if (avInfo && avInfo->perkTree) {
                logger::info("--- Arbre trouve pour ActorValue ID : {} ({}) ---", i, avInfo->GetFullName());
                
                Perks::PerkTree& tree = perkTrees[av];
                tree.Skill = av;

                std::unordered_map<RE::BGSSkillPerkTreeNode*, Perks::PerkNode*> nodeMap;

                // Lancement du scan récursif
                TraverseNode(avInfo->perkTree, av, tree, nodeMap, 0, nullptr);

                for (const auto& node : tree.Nodes) {
                    if (node->Parents.empty()) {
                        tree.RootNodes.push_back(node.get());
                    }
                }
                
                logger::info("--- Fin de l'arbre ActorValue {}. Noeuds recuperes: {}, RootNodes: {} ---", 
                    i, tree.Nodes.size(), tree.RootNodes.size());
            }
        }
        logger::info("=== FIN INITIALISATION DES ARBRES DE PERKS. Total arbres en cache : {} ===", perkTrees.size());
    }

    const Perks::PerkTree* GetPerkTree(RE::ActorValue skill) {
        auto it = perkTrees.find(skill);
        return (it != perkTrees.end()) ? &it->second : nullptr;
    }

    const Perks::PerkNode* GetPerkNode(RE::BGSPerk* perk) {
        if (!perk) return nullptr;
        auto it = nodeIndex.find(perk->GetFormID());
        return (it != nodeIndex.end()) ? it->second : nullptr;
    }

    bool HasPrerequisites(RE::Actor* actor, const Perks::PerkNode* node) {
        if (!actor || !node) return false;

        if (node->Parents.empty()) return true;

        for (const auto* parentNode : node->Parents) {
            if (actor->HasPerk(parentNode->Form)) {
                return true;
            }
        }
        return false;
    }
}