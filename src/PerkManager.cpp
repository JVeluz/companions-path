#include "PerkManager.h"

#include <unordered_map>

#include "Rules.h"
#include "StatManager.h"
#include "Storage.h"

namespace {
    std::unordered_map<RE::ActorValue, Perks::PerkTree> perkTrees;
    std::unordered_map<RE::FormID, Perks::PerkNode*> nodeIndex;

    int GetSkillLevelRequirement(RE::BGSPerk* perk) {
        if (!perk || !perk->perkConditions.head) return 0;

        int reqLevel = 0;
        auto currentCondition = perk->perkConditions.head;

        while (currentCondition) {
            uint16_t funcID = static_cast<uint16_t>(currentCondition->data.functionData.function.get());
            float compVal = currentCondition->data.comparisonValue.f;

            // 71 = GetActorValue | 73 = GetBaseActorValue | 277 = Fonction specifique des Perks
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

    void TraverseNode(RE::BGSSkillPerkTreeNode* nativeNode, RE::ActorValue av, Perks::PerkTree& tree, std::unordered_map<RE::BGSSkillPerkTreeNode*, Perks::PerkNode*>& nodeMap, int depth = 0, Perks::PerkNode* parentNode = nullptr) {
        if (!nativeNode) return;

        if (nodeMap.find(nativeNode) != nodeMap.end()) return;

        Perks::PerkNode* currentCustom = nullptr;

        if (nativeNode->perk) {
            auto customNode = std::make_unique<Perks::PerkNode>();
            customNode->Name = nativeNode->perk->GetFullName();
            customNode->AssociatedSkill = av;

            RE::BGSPerk* currentRankPerk = nativeNode->perk;
            while (currentRankPerk) {
                customNode->Ranks.push_back(currentRankPerk);
                customNode->RankRequirements.push_back(GetSkillLevelRequirement(currentRankPerk));
                currentRankPerk = currentRankPerk->nextPerk;
            }
            customNode->MaxRanks = static_cast<int>(customNode->Ranks.size());

            currentCustom = customNode.get();

            // logger::info("[Profondeur {}] Perk trouve : '{}' (MaxRanks: {}, ID Rang 1: {:08X})", depth, currentCustom->Name, currentCustom->MaxRanks, currentCustom->Ranks[0]->GetFormID());

            if (parentNode) {
                currentCustom->Parents.push_back(parentNode);
                parentNode->Children.push_back(currentCustom);
                // logger::info("[Profondeur {}] -> Parent assigne : '{}'", depth, parentNode->Name);
            }

            for (auto* rankPerk : currentCustom->Ranks) {
                nodeIndex[rankPerk->GetFormID()] = currentCustom;
            }

            nodeMap[nativeNode] = currentCustom;
            tree.Nodes.push_back(std::move(customNode));

        } else {
            // logger::info("[Profondeur {}] Noeud factice (Dummy) detecte.", depth);
            nodeMap[nativeNode] = nullptr;
        }

        Perks::PerkNode* nextParent = currentCustom ? currentCustom : parentNode;

        for (auto* childNative : nativeNode->children) {
            if (childNative) {
                TraverseNode(childNative, av, tree, nodeMap, depth + 1, nextParent);
            }
        }
    }

    // TrumanGIT : Follower-Leveling-System-Redone/src/serialization.cpp
    void AddPerkToActor(RE::Actor* actor, RE::BGSPerk* perk) {
        if (!actor || !perk) return;

        if (actor->HasPerk(perk)) {
            logger::info("L'acteur possède déjà ce perk.");
            return; 
        }

        if (auto base = actor->GetActorBase(); base) {
            if (base->AddPerk(perk, 1)) {
                for (const auto& perkEntry : perk->perkEntries) {
                    if (perkEntry) {
                        logger::info("Applying perk entry");
                        perkEntry->ApplyPerkEntry(actor);
                        logger::info("Applied perk entry");
                    }
                }
            }
        }
    }

    void RemovePerkToActor(RE::Actor* actor, RE::BGSPerk* perk) {
        if (!actor || !perk) return;

        if (auto base = actor->GetActorBase(); base) {
            for (const auto& perkEntry : perk->perkEntries) {
                if (perkEntry) {
                    logger::info("Removing perk entry");
                    perkEntry->RemovePerkEntry(actor);
                    logger::info("Removed perk entry");
                }
            }
            
            base->RemovePerk(perk);
        }
    }
}

namespace PerkManager {

    void Initialize() {
        // logger::info("=== DEBUT INITIALISATION DES ARBRES DE PERKS ===");
        perkTrees.clear();
        nodeIndex.clear();

        auto* actorValueList = RE::ActorValueList::GetSingleton();
        if (!actorValueList) {
            // logger::info("ERREUR : ActorValueList::GetSingleton() a retourne null !");
            return;
        }

        uint32_t totalAv = static_cast<uint32_t>(RE::ActorValue::kTotal);

        for (uint32_t i = 0; i < totalAv; ++i) {
            auto av = static_cast<RE::ActorValue>(i);
            RE::ActorValueInfo* avInfo = actorValueList->GetActorValueInfo(av);

            if (avInfo && avInfo->perkTree) {
                // logger::info("--- Arbre trouve pour ActorValue ID : {} ({}) ---", i, avInfo->GetFullName());

                Perks::PerkTree& tree = perkTrees[av];
                tree.Skill = av;

                std::unordered_map<RE::BGSSkillPerkTreeNode*, Perks::PerkNode*> nodeMap;

                TraverseNode(avInfo->perkTree, av, tree, nodeMap, 0, nullptr);

                for (const auto& node : tree.Nodes) {
                    if (node->Parents.empty()) {
                        tree.RootNodes.push_back(node.get());
                    }
                }
                // logger::info("--- Fin de l'arbre ActorValue {}. Noeuds: {}, RootNodes: {} ---", i, tree.Nodes.size(), tree.RootNodes.size());
            }
        }
        // logger::info("=== FIN INITIALISATION DES ARBRES DE PERKS ===");
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

    int GetPoints(RE::Actor* actor) { return Rules::Perks::GetPoints(actor); }

    int GetRemainingPoints(RE::Actor* actor) {
        int total = PerkManager::GetPoints(actor);
        int spent = static_cast<int>(Storage::Perks::GetPurchased(actor).size());
        return total - spent;
    }

    int GetCurrentRank(RE::Actor* actor, const Perks::PerkNode* node) {
        int rank = 0;
        while (rank < node->MaxRanks) {
            if (!Storage::Perks::HasPurchased(actor, node->Ranks[rank])) break;
            rank++;
        }
        return rank;
    }

    int GetNextRequirement(RE::Actor* actor, const Perks::PerkNode* node) {
        int rank = GetCurrentRank(actor, node);
        if (rank >= node->MaxRanks) return node->RankRequirements.back();
        return node->RankRequirements[rank];
    }

    bool HasPrerequisites(RE::Actor* actor, const Perks::PerkNode* node) {
        if (!actor || !node) return false;
        if (node->Parents.empty()) return true;

        for (const auto* parentNode : node->Parents) {
            if (!parentNode->Ranks.empty() && Storage::Perks::HasPurchased(actor, parentNode->Ranks[0])) {
                return true;
            }
        }
        return false;
    }

    bool IsMaxedOut(RE::Actor* actor, const Perks::PerkNode* node) { return GetCurrentRank(actor, node) >= node->MaxRanks; }

    bool CanPurchase(RE::Actor* actor, const Perks::PerkNode* node) {
        if (IsMaxedOut(actor, node)) return false;
        if (GetRemainingPoints(actor) <= 0) return false;
        if (!HasPrerequisites(actor, node)) return false;

        int requirement = GetNextRequirement(actor, node);
        float currentSkillLevel = StatManager::GetStatValue(actor, node->AssociatedSkill);

        return currentSkillLevel >= requirement;
    }

    bool CanRefund(RE::Actor* actor, const Perks::PerkNode* node) { return GetCurrentRank(actor, node) > 0; }

    void Purchase(RE::Actor* actor, const Perks::PerkNode* node) {
        if (CanPurchase(actor, node)) {
            int rank = GetCurrentRank(actor, node);
            auto* perk = node->Ranks[rank];
            AddPerkToActor(actor, perk);
            Storage::Perks::RecordPurchase(actor, perk);
        }
    }

    void Refund(RE::Actor* actor, const Perks::PerkNode* node) {
        if (CanRefund(actor, node)) {
            int rank = GetCurrentRank(actor, node);
            auto* perk = node->Ranks[rank - 1];
            RemovePerkToActor(actor, perk);
            Storage::Perks::RecordRefund(actor, perk);
        }
    }

    void RefundTree(RE::Actor* actor, const Perks::PerkNode* node) {
        for (int rank = 0; rank < node->MaxRanks; rank++) {
            auto* perk = node->Ranks[rank];
            RemovePerkToActor(actor, perk);
            if (Storage::Perks::HasPurchased(actor, perk)) {
                actor->RemovePerk(perk);
                Storage::Perks::RecordRefund(actor, perk);
            }
        }
    }
}