#include "UI.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "ConfigManager.h"
#include "FollowerManager.h"
#include "Language.h"
#include "PerkManager.h"
#include "ProfileManager.h"
#include "Rules.h"
#include "StatManager.h"
#include "structs.h"

namespace {
    const char* GetActorValueName(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kHealth:
                return Language::GetString("STAT_HEALTH");
            case RE::ActorValue::kMagicka:
                return Language::GetString("STAT_MAGICKA");
            case RE::ActorValue::kStamina:
                return Language::GetString("STAT_STAMINA");

            case RE::ActorValue::kUnarmedDamage:
                return Language::GetString("STAT_UNARMED_DAMAGE");
            case RE::ActorValue::kDamageResist:
                return Language::GetString("STAT_DAMAGE_RESIST");

            case RE::ActorValue::kOneHanded:
                return Language::GetString("STAT_ONE_HANDED");
            case RE::ActorValue::kTwoHanded:
                return Language::GetString("STAT_TWO_HANDED");
            case RE::ActorValue::kBlock:
                return Language::GetString("STAT_BLOCK");
            case RE::ActorValue::kHeavyArmor:
                return Language::GetString("STAT_HEAVY_ARMOR");
            case RE::ActorValue::kLightArmor:
                return Language::GetString("STAT_LIGHT_ARMOR");
            case RE::ActorValue::kArchery:
                return Language::GetString("STAT_ARCHERY");

            case RE::ActorValue::kDestruction:
                return Language::GetString("STAT_DESTRUCTION");
            case RE::ActorValue::kRestoration:
                return Language::GetString("STAT_RESTORATION");
            case RE::ActorValue::kAlteration:
                return Language::GetString("STAT_ALTERATION");
            case RE::ActorValue::kConjuration:
                return Language::GetString("STAT_CONJURATION");
            case RE::ActorValue::kIllusion:
                return Language::GetString("STAT_ILLUSION");

            case RE::ActorValue::kSneak:
                return Language::GetString("STAT_SNEAK");
            case RE::ActorValue::kLockpicking:
                return Language::GetString("STAT_LOCKPICKING");
            case RE::ActorValue::kPickpocket:
                return Language::GetString("STAT_PICKPOCKET");
            case RE::ActorValue::kSpeech:
                return Language::GetString("STAT_SPEECH");
            case RE::ActorValue::kAlchemy:
                return Language::GetString("STAT_ALCHEMY");
            case RE::ActorValue::kSmithing:
                return Language::GetString("STAT_SMITHING");
            case RE::ActorValue::kEnchanting:
                return Language::GetString("STAT_ENCHANTING");

            default:
                return Language::GetString("STAT_UNKNOWN");
        }
    }
}

namespace UI {
    int selectedCompanionIndex = 0;

    void Register() {
        if (SKSEMenuFramework::IsInstalled()) {
            SKSEMenuFramework::SetSection("Companions' Path");
            SKSEMenuFramework::AddSectionItem(Language::GetString("UI_TAB_STATS"), UI::Stats::Render);
            SKSEMenuFramework::AddSectionItem(Language::GetString("UI_TAB_PERKS"), UI::PerksUI::Render);
            SKSEMenuFramework::AddSectionItem(Language::GetString("UI_TAB_SETTINGS"), UI::Settings::Render);
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

            auto followers = FollowerManager::GetActorPtrs();

            if (followers.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_FOLLOWER"));
            } else {
                std::vector<const char*> names;
                static std::string unloadedStr = Language::GetString("UI_UNKNOWN_UNLOADED");

                for (auto actorPtr : followers) {
                    if (auto actor = actorPtr.get()) {
                        names.push_back(actorPtr->GetName());
                    } else {
                        names.push_back(Language::GetString("UI_UNKNOWN_UNLOADED"));
                    }
                }
                ImGuiMCP::Combo("##Target", &selectedCompanionIndex, names.data(), static_cast<int>(names.size()));
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(Language::GetString("UI_REFRESH_FOLLOWERS"))) {
                FollowerManager::Refresh();
                selectedCompanionIndex = 0;
                StatManager::Harmonize();
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (followers.empty()) return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= followers.size()) return;

            auto selectedActor = FollowerManager::GetActor(selectedCompanionIndex);
            if (!selectedActor) {
                ImGuiMCP::Text("%s", Language::GetString("UI_ACTOR_INVALID"));
                return;
            }

            auto profile = ProfileManager::GetActorProfile(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = Rules::Stats::GetAttributePoints(selectedActor);
            int maxSkillPoints = Rules::Stats::GetSkillPoints(selectedActor);

            ImGuiMCP::Text(Language::GetString("UI_LEVEL"), Rules::GetLevel(selectedActor));

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(Language::GetString("UI_ATTRIBUTES"), remainingAttributePoints, maxAttributePoints);
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

            ImGuiMCP::Text(Language::GetString("UI_SKILLS"), remainingSkillPoints, maxSkillPoints);
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

            if (ImGuiMCP::Button(Language::GetString("UI_RESET_ATTRIBUTES"))) {
                StatManager::ResetAttributes(selectedActor);
            }

            ImGuiMCP::SameLine();

            if (ImGuiMCP::Button(Language::GetString("UI_RESET_SKILLS"))) {
                StatManager::ResetSkills(selectedActor);
            }
        }
    }

    namespace Settings {

        std::vector<const char*> profiles;
        int selectedProfileIndex = 0;

        void RefreshProfiles() {
            profiles.clear();
            for (const std::string& profileName : ProfileManager::GetProfiles()) {
                profiles.push_back(profileName.c_str());
            }
        }

        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);
            if (ImGuiMCP::Combo("##TargetProfiles", &selectedProfileIndex, profiles.data(), static_cast<int>(profiles.size()))) {
                ConfigManager::SetProfile(ProfileManager::GetProfileName(selectedProfileIndex));
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            Profile profile = ProfileManager::GetProfile();
            bool hasChanged = false;

            if (ImGuiMCP::Checkbox(Language::GetString("UI_SETTING_HARMONIZE"), &profile.harmonize)) hasChanged = true;
            if (ImGuiMCP::Checkbox(Language::GetString("UI_SETTING_SYNC_LEVEL"), &profile.syncLevel)) hasChanged = true;

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::SetNextItemWidth(200.0f);
            if (ImGuiMCP::InputFloat(Language::GetString("UI_SETTING_LEVEL_MULTIPLIER"), &profile.levelMultiplier, 0.0f, 0.0f, "%.2f")) hasChanged = true;

            ImGuiMCP::SetNextItemWidth(200.0f);
            if (ImGuiMCP::InputFloat(Language::GetString("UI_SETTING_ATTRIBUTE_MULTIPLIER"), &profile.attributeMultiplier, 0.0f, 0.0f, "%.2f")) hasChanged = true;

            ImGuiMCP::SetNextItemWidth(200.0f);
            if (ImGuiMCP::InputFloat(Language::GetString("UI_SETTING_SKILL_MULTIPLIER"), &profile.skillMultiplier, 0.0f, 0.0f, "%.2f")) hasChanged = true;

            ImGuiMCP::SetNextItemWidth(200.0f);
            if (ImGuiMCP::InputFloat(Language::GetString("UI_SETTING_PERK_MULTIPLIER"), &profile.perkMultiplier, 0.0f, 0.0f, "%.2f")) hasChanged = true;

            if (hasChanged) {
                ProfileManager::UpdateSettings(profile);
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            const char* saveString = Language::GetString("UI_SAVE_PROFILE");
            if (!saveString || saveString[0] == '\0') saveString = "Save Profile to JSON";

            if (ImGuiMCP::Button(saveString)) {
                ProfileManager::SaveCurrentProfile();
            }
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
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_PERKS_FOUND"));
                return;
            }

            float currentSkillLevel = StatManager::GetStatValue(selectedActor, tree->skill);
            ImGuiMCP::Text(Language::GetString("UI_SKILL_LEVEL"), currentSkillLevel);
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

                ImGuiMCP::Text(Language::GetString("UI_PERK_LEVEL"), reqLevel);
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

            auto followers = FollowerManager::GetActorPtrs();

            if (followers.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_FOLLOWER"));
                return;
            }

            std::vector<const char*> names;
            static std::string unloadedStr = Language::GetString("UI_UNKNOWN_UNLOADED");

            for (auto actorPtr : followers) {
                if (auto actor = actorPtr.get()) {
                    names.push_back(actorPtr->GetName());
                } else {
                    names.push_back(Language::GetString("UI_UNKNOWN_UNLOADED"));
                }
            }

            ImGuiMCP::Combo("##TargetPerks", &selectedCompanionIndex, names.data(), static_cast<int>(names.size()));

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(Language::GetString("UI_REFRESH_FOLLOWERS"))) {
                FollowerManager::Refresh();
                StatManager::Harmonize();
                selectedCompanionIndex = 0;
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (selectedCompanionIndex >= followers.size()) selectedCompanionIndex = 0;

            auto selectedActor = FollowerManager::GetActor(selectedCompanionIndex);
            if (!selectedActor) {
                ImGuiMCP::Text("%s", Language::GetString("UI_ACTOR_INVALID"));
                return;
            }

            // if (!FollowerManager::IsUniqueNPC(selectedActor)) {
            //     ImGuiMCP::Text(Language::GetString("UI_ERROR_GENERIC_NPC"));
            //     return;
            // }

            auto profile = ProfileManager::GetActorProfile(selectedActor);

            if (profile.skills.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_SKILLS_PROFILE"));
                return;
            }

            int remainingPoints = PerkManager::GetRemainingPoints(selectedActor);
            ImGuiMCP::Text(Language::GetString("UI_PERK_POINTS_AVAILABLE"), remainingPoints);
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

            if (ImGuiMCP::Button(Language::GetString("UI_RESET_PERK_TREE"))) {
                PerkManager::RefundTree(selectedActor, tree);
            }
        }
    }
}