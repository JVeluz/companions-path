#include "UI.h"

#include <string>
#include <vector>

#include "ConfigManager.h"
#include "PerkManager.h"
#include "Rules.h"
#include "StatManager.h"
#include "Utils.h"
#include "language.h"
#include "profile.h"
#include "structs.h"

namespace {
    std::vector<RE::ActorHandle> currentFollowers;

    const char* GetActorValueName(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kHealth:
                return TranslationService::GetString("STAT_HEALTH");
            case RE::ActorValue::kMagicka:
                return TranslationService::GetString("STAT_MAGICKA");
            case RE::ActorValue::kStamina:
                return TranslationService::GetString("STAT_STAMINA");

            case RE::ActorValue::kUnarmedDamage:
                return TranslationService::GetString("STAT_UNARMED_DAMAGE");
            case RE::ActorValue::kDamageResist:
                return TranslationService::GetString("STAT_DAMAGE_RESIST");

            case RE::ActorValue::kOneHanded:
                return TranslationService::GetString("STAT_ONE_HANDED");
            case RE::ActorValue::kTwoHanded:
                return TranslationService::GetString("STAT_TWO_HANDED");
            case RE::ActorValue::kBlock:
                return TranslationService::GetString("STAT_BLOCK");
            case RE::ActorValue::kHeavyArmor:
                return TranslationService::GetString("STAT_HEAVY_ARMOR");
            case RE::ActorValue::kLightArmor:
                return TranslationService::GetString("STAT_LIGHT_ARMOR");
            case RE::ActorValue::kArchery:
                return TranslationService::GetString("STAT_ARCHERY");

            case RE::ActorValue::kDestruction:
                return TranslationService::GetString("STAT_DESTRUCTION");
            case RE::ActorValue::kRestoration:
                return TranslationService::GetString("STAT_RESTORATION");
            case RE::ActorValue::kAlteration:
                return TranslationService::GetString("STAT_ALTERATION");
            case RE::ActorValue::kConjuration:
                return TranslationService::GetString("STAT_CONJURATION");
            case RE::ActorValue::kIllusion:
                return TranslationService::GetString("STAT_ILLUSION");

            case RE::ActorValue::kSneak:
                return TranslationService::GetString("STAT_SNEAK");
            case RE::ActorValue::kLockpicking:
                return TranslationService::GetString("STAT_LOCKPICKING");
            case RE::ActorValue::kPickpocket:
                return TranslationService::GetString("STAT_PICKPOCKET");
            case RE::ActorValue::kSpeech:
                return TranslationService::GetString("STAT_SPEECH");
            case RE::ActorValue::kAlchemy:
                return TranslationService::GetString("STAT_ALCHEMY");
            case RE::ActorValue::kSmithing:
                return TranslationService::GetString("STAT_SMITHING");
            case RE::ActorValue::kEnchanting:
                return TranslationService::GetString("STAT_ENCHANTING");

            default:
                return TranslationService::GetString("STAT_UNKNOWN");
        }
    }
}

namespace UI {
    int selectedCompanionIndex = 0;

    void Register() {
        if (SKSEMenuFramework::IsInstalled()) {
            SKSEMenuFramework::SetSection("Companions' Path");
            SKSEMenuFramework::AddSectionItem(TranslationService::GetString("UI_TAB_STATS"), UI::Stats::Render);
            SKSEMenuFramework::AddSectionItem(TranslationService::GetString("UI_TAB_PERKS"), UI::PerksUI::Render);
            SKSEMenuFramework::AddSectionItem(TranslationService::GetString("UI_TAB_SETTINGS"), UI::Settings::Render);
        }
    }

    namespace Stats {

        namespace {
            void RenderStatRow(RE::Actor* actor, RE::ActorValue actorValue) {
                const char* name = GetActorValueName(actorValue);

                ImGuiMCP::PushID(name);

                float value = StatManager::GetStatValue(actor, actorValue);

                ImGuiMCP::Text("%3.0f %s", value, name);

                if (ImGuiMCP::SmallButton(" << ")) {
                    for (int i = 0; i < 5; ++i) StatManager::RemovePoint(actor, actorValue);
                }
                ImGuiMCP::SameLine();

                if (ImGuiMCP::SmallButton(" - ")) {
                    StatManager::RemovePoint(actor, actorValue);
                }

                ImGuiMCP::SameLine();

                if (ImGuiMCP::SmallButton(" + ")) {
                    StatManager::AddPoint(actor, actorValue);
                }

                ImGuiMCP::SameLine();

                if (ImGuiMCP::SmallButton(" >> ")) {
                    for (int i = 0; i < 5; ++i) StatManager::AddPoint(actor, actorValue);
                }

                ImGuiMCP::PopID();
            }
        }

        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);
            if (currentFollowers.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_FOLLOWER"));
            } else {
                std::vector<const char*> names;
                static std::string unloadedStr = TranslationService::GetString("UI_UNKNOWN_UNLOADED");

                for (auto& handle : currentFollowers) {
                    auto actorPtr = handle.get();
                    if (actorPtr) {
                        if (auto actorBase = actorPtr->GetActorBase()) {
                            names.push_back(actorBase->GetName());
                        } else {
                            names.push_back(actorPtr->GetName());
                        }
                    } else {
                        names.push_back(unloadedStr.c_str());
                    }
                }
                ImGuiMCP::Combo("##Target", &selectedCompanionIndex, names.data(), static_cast<int>(names.size()));
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(TranslationService::GetString("UI_REFRESH_FOLLOWERS"))) {
                currentFollowers = Utils::GetActiveFollowers();
                selectedCompanionIndex = 0;
                StatManager::Harmonize();
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(TranslationService::GetString("UI_RELOAD_CONFIG"))) {
                ConfigManager::LoadConfig("Data/SKSE/Plugins/CompanionsPath/config.json");
                StatManager::Harmonize();
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            if (currentFollowers.empty()) return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= currentFollowers.size()) return;

            auto selectedActorNiPtr = currentFollowers[selectedCompanionIndex].get();
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_ACTOR_INVALID"));
                return;
            }

            auto selectedActor = selectedActorNiPtr.get();
            auto profile = ProfileParser::GetProfile(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = Rules::Stats::GetAttributePoints(selectedActor);
            int maxSkillPoints = Rules::Stats::GetSkillPoints(selectedActor);

            ImGuiMCP::Text(TranslationService::GetString("UI_LEVEL"), selectedActor->GetLevel());

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(TranslationService::GetString("UI_ATTRIBUTES"), remainingAttributePoints, maxAttributePoints);
            ImGuiMCP::Spacing();

            ImGuiMCP::Columns(3, "AttributesColumns", false);

            for (const auto& attr : profile.Attributes) {
                RenderStatRow(selectedActor, attr);
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1);

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(TranslationService::GetString("UI_SKILLS"), remainingSkillPoints, maxSkillPoints);
            ImGuiMCP::Spacing();

            ImGuiMCP::Columns(3, "StatsColumns", false);

            for (const auto& skill : profile.Skills) {
                RenderStatRow(selectedActor, skill);
                ImGuiMCP::Spacing();
                ImGuiMCP::Spacing();
                ImGuiMCP::Spacing();
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1);

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            if (ImGuiMCP::Button(TranslationService::GetString("UI_RESET_ATTRIBUTES"))) {
                StatManager::ResetAttributes(selectedActor);
            }

            ImGuiMCP::SameLine();

            if (ImGuiMCP::Button(TranslationService::GetString("UI_RESET_SKILLS"))) {
                StatManager::ResetSkills(selectedActor);
            }
        }
    }

    namespace PerksUI {

        float CalculateActorArmor(RE::Actor* actor) {
            if (!actor) return 0.0f;

            // Sécurité : On s'assure de bien récupérer l'interface ActorValueOwner
            auto avOwner = actor->AsActorValueOwner();
            if (!avOwner) return 0.0f;

            return avOwner->GetActorValue(RE::ActorValue::kDamageResist);
        }

        float CalculateActorDamage(RE::Actor* actor) {
            if (!actor) return 0.0f;

            float calculatedDamage = 0.0f;

            // FIX ANTI-CRASH : On vérifie manuellement que la liste d'inventaire
            // existe bien AVANT d'appeler GetEquippedObject.
            auto invChanges = actor->GetInventoryChanges();
            if (!invChanges || !invChanges->entryList) {
                return 0.0f;  // L'inventaire n'est pas encore prêt, on évite le crash.
            }

            auto rightHandObj = actor->GetEquippedObject(false);
            if (rightHandObj && rightHandObj->IsWeapon()) {
                auto weapon = rightHandObj->As<RE::TESObjectWEAP>();
                if (weapon) {  // Sécurité supplémentaire
                    float baseDamage = weapon->GetAttackDamage();

                    auto avOwner = actor->AsActorValueOwner();
                    float damageMult = avOwner ? avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult) : 1.0f;
                    if (damageMult == 0.0f) damageMult = 1.0f;

                    calculatedDamage = baseDamage * damageMult;
                }
            }
            return calculatedDamage;
        }

        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);

            if (currentFollowers.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_FOLLOWER"));
                return;
            }

            // --- 1. SELECTION DU COMPAGNON ---
            std::vector<const char*> names;
            static std::string unloadedStr = TranslationService::GetString("UI_UNKNOWN_UNLOADED");

            for (auto& handle : currentFollowers) {
                auto actorPtr = handle.get();
                if (actorPtr) {
                    if (auto actorBase = actorPtr->GetActorBase()) {
                        names.push_back(actorBase->GetName());
                    } else {
                        names.push_back(actorPtr->GetName());
                    }
                } else {
                    names.push_back(unloadedStr.c_str());
                }
            }
            ImGuiMCP::Combo("##TargetPerks", &selectedCompanionIndex, names.data(), static_cast<int>(names.size()));
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();

            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= currentFollowers.size()) return;

            auto selectedActorNiPtr = currentFollowers[selectedCompanionIndex].get();
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_ACTOR_INVALID"));
                return;
            }

            auto selectedActor = selectedActorNiPtr.get();
            auto profile = ProfileParser::GetProfile(selectedActor);

            if (profile.Skills.empty()) {
                ImGuiMCP::Text("Aucune competence pour ce profil.");
                return;
            }

            // --- 2. AFFICHAGE DES POINTS & COMPETENCES ---
            int remainingPoints = PerkManager::GetRemainingPoints(selectedActor);
            ImGuiMCP::Text("Points de Perk disponibles : %d", remainingPoints);
            ImGuiMCP::Spacing();

            static int selectedSkillIndex = 0;
            if (selectedSkillIndex >= profile.Skills.size()) {
                selectedSkillIndex = 0;
            }

            std::vector<const char*> skillNames;
            for (auto skill : profile.Skills) {
                skillNames.push_back(GetActorValueName(skill));
            }

            ImGuiMCP::Combo("##SkillTree", &selectedSkillIndex, skillNames.data(), static_cast<int>(skillNames.size()));

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            // --- 3. RECUPERATION ET TRI DES PERKS ---
            RE::ActorValue currentSkill = profile.Skills[selectedSkillIndex];
            const Perks::PerkTree* tree = PerkManager::GetPerkTree(currentSkill);

            if (!tree || tree->Nodes.empty()) {
                ImGuiMCP::Text("Aucun perk trouve pour cette competence.");
                return;
            }

            std::vector<Perks::PerkNode*> sortedNodes;
            for (const auto& node : tree->Nodes) {
                sortedNodes.push_back(node.get());
            }

            std::sort(sortedNodes.begin(), sortedNodes.end(), [](const Perks::PerkNode* a, const Perks::PerkNode* b) {
                int reqA = a->RankRequirements.empty() ? 0 : a->RankRequirements[0];
                int reqB = b->RankRequirements.empty() ? 0 : b->RankRequirements[0];
                return reqA < reqB;
            });

            // --- 4. AFFICHAGE DES BOUTONS ---
            for (auto* node : sortedNodes) {
                if (node->Ranks.empty()) continue;

                int currentRank = PerkManager::GetCurrentRank(selectedActor, node);
                bool isMaxedOut = PerkManager::IsMaxedOut(selectedActor, node);
                int nextReqLevel = PerkManager::GetNextRequirement(selectedActor, node);

                bool canBuy = PerkManager::CanPurchase(selectedActor, node);
                bool canRefund = PerkManager::CanRefund(selectedActor, node);

                if (!canBuy && !isMaxedOut) {
                    ImGuiMCP::BeginDisabled();
                }

                std::string btnText = node->Name + " (" + std::to_string(currentRank) + "/" + std::to_string(node->MaxRanks) + ") ";
                btnText += isMaxedOut ? "[Max]" : "(Niv. " + std::to_string(nextReqLevel) + ")";

                if (ImGuiMCP::Button(btnText.c_str())) {
                    if (canBuy) {
                        PerkManager::Purchase(selectedActor, node);
                    }
                }

                if (!canBuy && !isMaxedOut) {
                    ImGuiMCP::EndDisabled();
                }

                if (canRefund) {
                    ImGuiMCP::SameLine();
                    ImGuiMCP::PushID(node->Ranks[0]->GetFormID());

                    if (ImGuiMCP::Button(" - ")) {
                        PerkManager::Refund(selectedActor, node);
                    }

                    ImGuiMCP::PopID();
                }
            }

            // --- 5. STATISTIQUES DE COMBAT ---
            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text("Statistiques de combat (Equipement actuel) :");

            float currentArmor = CalculateActorArmor(selectedActor);
            float currentDamage = CalculateActorDamage(selectedActor);

            ImGuiMCP::Text("Armure totale : %.0f", currentArmor);

            if (currentDamage > 0.0f) {
                ImGuiMCP::Text("Degats (Main droite) : %.1f", currentDamage);
            } else {
                ImGuiMCP::Text("Degats (Main droite) : Mains nues / Magie");
            }
        }
    }
}