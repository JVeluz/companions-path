#include "UI.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "ConfigManager.h"
#include "FollowerManager.h"
#include "PerkManager.h"
#include "Rules.h"
#include "StatManager.h"
#include "language.h"
#include "profile.h"
#include "structs.h"

namespace {
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

            auto followers = FollowerManager::GetActiveFollowers();

            if (followers.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_FOLLOWER"));
            } else {
                std::vector<const char*> names;
                static std::string unloadedStr = TranslationService::GetString("UI_UNKNOWN_UNLOADED");

                for (auto& handle : followers) {
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
                FollowerManager::RefreshFollowers();
                selectedCompanionIndex = 0;
                StatManager::Harmonize();
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (followers.empty()) return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= followers.size()) return;

            auto selectedActorNiPtr = FollowerManager::GetActorPtr(selectedCompanionIndex);
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
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(TranslationService::GetString("UI_ATTRIBUTES"), remainingAttributePoints, maxAttributePoints);
            ImGuiMCP::Spacing();

            ImGuiMCP::Columns(3, "AttributesColumns", false);

            for (const auto& attr : profile.attributes) {
                RenderStatRow(selectedActor, attr);
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1);

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(TranslationService::GetString("UI_SKILLS"), remainingSkillPoints, maxSkillPoints);
            ImGuiMCP::Spacing();

            ImGuiMCP::Columns(3, "StatsColumns", false);

            for (const auto& skill : profile.skills) {
                RenderStatRow(selectedActor, skill);
                ImGuiMCP::Spacing();
                ImGuiMCP::Spacing();
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1);

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
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

    namespace Settings {
        void __stdcall Render() {
            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(TranslationService::GetString("UI_RELOAD_CONFIG"))) {
                ConfigManager::LoadConfig("Data/SKSE/Plugins/CompanionsPath/config.json");
                StatManager::Harmonize();
            }

            bool harmonize = ConfigManager::GetHarmonize();
            if (ImGuiMCP::Checkbox(TranslationService::GetString("UI_SETTING_HARMONIZE"), &harmonize)) {
                ConfigManager::SetHarmonize(harmonize);
                StatManager::Harmonize();
                PerkManager::Harmonize(harmonize);
            }

            bool syncLevel = ConfigManager::GetSyncLevel();
            if (ImGuiMCP::Checkbox(TranslationService::GetString("UI_SETTING_SYNC_LEVEL"), &syncLevel)) {
                ConfigManager::SetSyncLevel(syncLevel);
                if (syncLevel) {
                    FollowerManager::SyncFollowerLevels();
                    StatManager::Harmonize();
                }
            }

            // ImGuiMCP::Spacing();
            // ImGuiMCP::Separator();
            // ImGuiMCP::Spacing();

            // static int currentLangIndex = -1;

            // if (ImGuiMCP::Button(TranslationService::GetString("UI_REFRESH_LANGUAGES"))) {
            //     LanguageRepository::ScanAvailableLanguages();
            //     currentLangIndex = -1;
            // }

            // ImGuiMCP::Spacing();

            // const auto& availableLanguages = LanguageRepository::GetAvailableLanguages();

            // if (!availableLanguages.empty()) {
            //     if (currentLangIndex == -1) {
            //         std::string currentLang = ConfigManager::GetLanguage();
            //         auto it = std::find(availableLanguages.begin(), availableLanguages.end(), currentLang);
            //         if (it != availableLanguages.end()) {
            //             currentLangIndex = static_cast<int>(std::distance(availableLanguages.begin(), it));
            //         } else {
            //             currentLangIndex = 0;
            //         }
            //     }

            //     std::vector<const char*> langItems;
            //     for (const auto& lang : availableLanguages) {
            //         langItems.push_back(lang.c_str());
            //     }

            //     if (ImGuiMCP::Combo(TranslationService::GetString("UI_SETTING_LANGUAGE"), &currentLangIndex, langItems.data(), static_cast<int>(langItems.size()))) {
            //         ConfigManager::SetLanguage(availableLanguages[currentLangIndex]);
            //     }
            // } else {
            //     ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_LANGUAGE_FILES"));
            // }
        }
    }

    namespace PerksUI {

        void RenderPerkTooltip(const Perks::PerkNode* node, int currentRank) {
            if (ImGuiMCP::IsItemHovered()) {
                ImGuiMCP::BeginTooltip();

                int displayRank = currentRank;
                if (displayRank >= node->ranks.size()) {
                    displayRank = static_cast<int>(node->ranks.size()) - 1;
                }

                if (displayRank >= 0 && displayRank < node->ranks.size()) {
                    auto perkForm = node->ranks[displayRank];
                    if (perkForm) {
                        RE::BSString perkDesc;
                        perkForm->GetDescription(perkDesc, perkForm);
                        ImGuiMCP::PushTextWrapPos(400.0f);
                        ImGuiMCP::TextWrapped("%s", perkDesc.c_str());
                        ImGuiMCP::PopTextWrapPos();
                    }
                }
                ImGuiMCP::EndTooltip();
            }
        }

        void RenderPerkSimpleList(RE::Actor* selectedActor, const Perks::PerkTree* tree) {
            if (!tree || tree->nodes.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_PERKS_FOUND"));
                return;
            }

            float currentSkillLevel = StatManager::GetStatValue(selectedActor, tree->skill);
            ImGuiMCP::Text(TranslationService::GetString("UI_SKILL_LEVEL"), currentSkillLevel);
            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            std::map<int, std::vector<Perks::PerkNode*>> nodesByLevel;
            for (const auto& nodePtr : tree->nodes) {
                auto* node = nodePtr.get();
                if (node->ranks.empty()) continue;

                int reqLevel = node->rankRequirements.empty() ? 0 : node->rankRequirements[0];
                nodesByLevel[reqLevel].push_back(node);
            }

            for (auto& [reqLevel, nodes] : nodesByLevel) {
                std::sort(nodes.begin(), nodes.end(), [](const Perks::PerkNode* a, const Perks::PerkNode* b) { return a->horizontalPosition < b->horizontalPosition; });

                ImGuiMCP::Text(TranslationService::GetString("UI_PERK_LEVEL"), reqLevel);
                ImGuiMCP::Separator();
                ImGuiMCP::Spacing();

                for (size_t i = 0; i < nodes.size(); ++i) {
                    auto* node = nodes[i];

                    int currentRank = PerkManager::GetCurrentRank(selectedActor, node);
                    bool isMaxedOut = PerkManager::IsMaxedOut(selectedActor, node);
                    bool canBuy = PerkManager::CanPurchase(selectedActor, node);
                    bool canRefund = PerkManager::CanRefund(selectedActor, node);

                    ImGuiMCP::PushID(node->ranks[0]->GetFormID());

                    if (!canBuy && !isMaxedOut) {
                        ImGuiMCP::BeginDisabled();
                    }

                    std::string btnText = node->name + " (" + std::to_string(currentRank) + "/" + std::to_string(node->maxRanks) + ")";
                    if (ImGuiMCP::Button(btnText.c_str())) {
                        if (canBuy) PerkManager::Purchase(selectedActor, node);
                    }

                    if (!canBuy && !isMaxedOut) {
                        ImGuiMCP::EndDisabled();
                    }

                    ImGuiMCP::SameLine();
                    ImGuiMCP::TextDisabled("(?)");
                    RenderPerkTooltip(node, currentRank);

                    if (canRefund) {
                        ImGuiMCP::SameLine();
                        if (ImGuiMCP::Button(" - ")) {
                            PerkManager::Refund(selectedActor, node);
                        }
                    }

                    ImGuiMCP::PopID();

                    if (i < nodes.size() - 1) {
                        ImGuiMCP::SameLine(0.0f, 15.0f);
                    }
                }
                ImGuiMCP::Spacing();
                ImGuiMCP::Spacing();
            }
        }

        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);

            auto followers = FollowerManager::GetActiveFollowers();

            if (followers.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_FOLLOWER"));
                return;
            }

            std::vector<const char*> names;
            static std::string unloadedStr = TranslationService::GetString("UI_UNKNOWN_UNLOADED");

            for (auto& handle : followers) {
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

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(TranslationService::GetString("UI_REFRESH_FOLLOWERS"))) {
                FollowerManager::RefreshFollowers();
                StatManager::Harmonize();
                selectedCompanionIndex = 0;
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (selectedCompanionIndex >= followers.size()) selectedCompanionIndex = 0;

            auto selectedActorNiPtr = FollowerManager::GetActorPtr(selectedCompanionIndex);
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_ACTOR_INVALID"));
                return;
            }

            auto selectedActor = selectedActorNiPtr.get();

            // if (!FollowerManager::IsUniqueNPC(selectedActor)) {
            //     ImGuiMCP::Text(TranslationService::GetString("UI_ERROR_GENERIC_NPC"));
            //     return;
            // }

            auto profile = ProfileParser::GetProfile(selectedActor);

            if (profile.skills.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_SKILLS_PROFILE"));
                return;
            }

            int remainingPoints = PerkManager::GetRemainingPoints(selectedActor);
            ImGuiMCP::Text(TranslationService::GetString("UI_PERK_POINTS_AVAILABLE"), remainingPoints);
            ImGuiMCP::Spacing();

            static int selectedSkillIndex = 0;
            if (selectedSkillIndex >= profile.skills.size()) {
                selectedSkillIndex = 0;
            }

            std::vector<const char*> skillNames;
            for (auto skill : profile.skills) {
                skillNames.push_back(GetActorValueName(skill));
            }

            ImGuiMCP::Combo("##SkillTree", &selectedSkillIndex, skillNames.data(), static_cast<int>(skillNames.size()));

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            RE::ActorValue currentSkill = profile.skills[selectedSkillIndex];
            const Perks::PerkTree* tree = PerkManager::GetPerkTree(currentSkill);

            RenderPerkSimpleList(selectedActor, tree);

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (ImGuiMCP::Button(TranslationService::GetString("UI_RESET_PERK_TREE"))) {
                PerkManager::RefundTree(selectedActor, tree);
            }
        }
    }
}