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
    uintptr_t selectedCompanionID = 0;
    int currentMenuContext = 0;
    bool needsTabSync = false;

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

        void RenderSingleFollowerTab(RE::Actor* selectedActor) {
            auto profile = ProfileManager::GetActorProfile(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = Rules::Stats::GetAttributePoints(selectedActor);
            int maxSkillPoints = Rules::Stats::GetSkillPoints(selectedActor);

            ImGuiMCP::Text(Language::GetString("UI_LEVEL_FORMAT"), Rules::GetLevel(selectedActor));

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text(Language::GetString("UI_ATTRIBUTES_POINTS_FORMAT"), remainingAttributePoints, maxAttributePoints);
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

            ImGuiMCP::Text(Language::GetString("UI_SKILLS_POINTS_FORMAT"), remainingSkillPoints, maxSkillPoints);
            ImGuiMCP::Spacing();

            ImGuiMCP::Columns(3, "StatsColumns", false);

            for (const auto& skill : profile.skills) {
                RenderStatRow(selectedActor, skill);
                ImGuiMCP::Spacing();
                ImGuiMCP::Spacing();
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1);
        }

        void __stdcall Render() {
            if (UI::currentMenuContext != 1) {
                UI::currentMenuContext = 1;
                UI::needsTabSync = true;
                FollowerManager::Refresh();
                StatManager::Harmonize();
            }

            if (ImGuiMCP::Button(Language::GetString("UI_REFRESH_FOLLOWERS"))) {
                FollowerManager::Refresh();
                StatManager::Harmonize();
            }

            auto followers = FollowerManager::GetActorPtrs();
            
            if (followers.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_FOLLOWER"));
                return;
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (ImGuiMCP::BeginTabBar("FollowersSharedTabBar")) {
                int overviewFlags = (UI::needsTabSync && UI::selectedCompanionID == 0) ? ImGuiMCP::ImGuiTabItemFlags_SetSelected : 0;

                if (ImGuiMCP::BeginTabItem(Language::GetString("UI_OVERVIEW_TAB"), nullptr, overviewFlags)) {
                    UI::selectedCompanionID = 0;

                    ImGuiMCP::BeginChild("OverviewScrollArea", ImGuiMCP::ImVec2(0, 0), false, ImGuiMCP::ImGuiWindowFlags_HorizontalScrollbar);

                    for (size_t i = 0; i < followers.size(); ++i) {
                        auto actorPtr = followers[i];
                        auto actor = actorPtr.get();
                        std::string name = actor ? actorPtr->GetName() : Language::GetString("UI_UNKNOWN_UNLOADED");

                        ImGuiMCP::PushID(actor);
                        ImGuiMCP::BeginChild("FollowerBox", ImGuiMCP::ImVec2(320, 0), true);

                        ImGuiMCP::Text("%s", name.c_str());
                        ImGuiMCP::Separator();

                        if (actor) {
                            auto profile = ProfileManager::GetActorProfile(actor);

                            ImGuiMCP::Text(Language::GetString("UI_LEVEL_FORMAT"), Rules::GetLevel(actor));
                            ImGuiMCP::Spacing();

                            ImGuiMCP::TextDisabled("%s", Language::GetString("UI_ATTRIBUTES_TITLE"));
                            for (const auto& attr : profile.attributes) {
                                float val = StatManager::GetStatValue(actor, attr);
                                ImGuiMCP::TextWrapped("- %s: %.0f", GetActorValueName(attr), val);
                            }

                            ImGuiMCP::Spacing();
                            ImGuiMCP::TextDisabled("%s", Language::GetString("UI_SKILLS_TITLE"));
                            for (const auto& skill : profile.skills) {
                                float val = StatManager::GetStatValue(actor, skill);
                                ImGuiMCP::TextWrapped("- %s: %.0f", GetActorValueName(skill), val);
                            }

                            ImGuiMCP::Spacing();
                            ImGuiMCP::Separator();
                            ImGuiMCP::Spacing();

                            if (ImGuiMCP::Button(Language::GetString("UI_RESET_ATTRIBUTES"))) {
                                StatManager::ResetAttributes(actor);
                            }

                            ImGuiMCP::SameLine();

                            if (ImGuiMCP::Button(Language::GetString("UI_RESET_SKILLS"))) {
                                StatManager::ResetSkills(actor);
                            }
                        } else {
                            ImGuiMCP::TextDisabled("%s", Language::GetString("UI_ACTOR_INVALID"));
                        }

                        ImGuiMCP::EndChild();
                        ImGuiMCP::PopID();

                        if (i < followers.size() - 1) {
                            ImGuiMCP::SameLine();
                        }
                    }

                    ImGuiMCP::EndChild();
                    ImGuiMCP::EndTabItem();
                }

                for (size_t i = 0; i < followers.size(); ++i) {
                    auto actor = followers[i].get();
                    uintptr_t actorID = reinterpret_cast<uintptr_t>(actor);
                    std::string baseName = actor ? followers[i]->GetName() : Language::GetString("UI_UNKNOWN_UNLOADED");
                    std::string tabName = baseName + "###FollowerTab_" + std::to_string(actorID);

                    int tabFlags = (UI::needsTabSync && UI::selectedCompanionID == actorID) ? ImGuiMCP::ImGuiTabItemFlags_SetSelected : 0;

                    if (ImGuiMCP::BeginTabItem(tabName.c_str(), nullptr, tabFlags)) {
                        UI::selectedCompanionID = actorID;

                        if (actor) {
                            RenderSingleFollowerTab(actor);
                        } else {
                            ImGuiMCP::Text("%s", Language::GetString("UI_ACTOR_INVALID"));
                        }
                        ImGuiMCP::EndTabItem();
                    }
                }

                ImGuiMCP::EndTabBar();
            }
            UI::needsTabSync = false;
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

            if (ImGuiMCP::Checkbox(Language::GetString("UI_SETTING_HARMONIZE_STATS"), &profile.harmonizeStats)) hasChanged = true;
            if (ImGuiMCP::Checkbox(Language::GetString("UI_SETTING_HARMONIZE_PERKS"), &profile.harmonizePerks)) hasChanged = true;
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
                        std::string descStr = perkDesc.c_str();
                        
                        if (descStr.empty()) {
                            descStr = Language::GetString("UI_NO_DESCRIPTION");
                        }
                        
                        ImGuiMCP::PushTextWrapPos(400.0f);
                        ImGuiMCP::TextWrapped("%s", descStr.c_str());
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
                        if (ImGuiMCP::Button("-")) {
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

        void RenderSingleFollowerTab(RE::Actor* selectedActor) {
            auto profile = ProfileManager::GetActorProfile(selectedActor);

            if (profile.skills.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_SKILLS_PROFILE"));
                return;
            }

            ImGuiMCP::PushID(selectedActor);

            static std::map<RE::Actor*, int> selectedSkillIndices;
            int& selectedSkillIndex = selectedSkillIndices[selectedActor];

            if (selectedSkillIndex >= profile.skills.size()) {
                selectedSkillIndex = 0;
            }

            RE::ActorValue currentSkill = profile.skills[selectedSkillIndex];
            const Perks::PerkTree* tree = PerkManager::GetPerkTree(currentSkill);

            int remainingPoints = PerkManager::GetRemainingPoints(selectedActor);
            ImGuiMCP::Text(Language::GetString("UI_PERK_POINTS_AVAILABLE"), remainingPoints);
            ImGuiMCP::Spacing();

            std::vector<const char*> skillNames;
            for (auto skill : profile.skills) {
                skillNames.push_back(GetActorValueName(skill));
            }

            ImGuiMCP::Combo("##SkillTree", &selectedSkillIndex, skillNames.data(), static_cast<int>(skillNames.size()));

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            RenderPerkSimpleList(selectedActor, tree);

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            static RE::Actor* cachedActor = nullptr;
            static std::vector<std::string> cachedPurchasedPerks;

            if (cachedActor != selectedActor) {
                cachedActor = selectedActor;
                cachedPurchasedPerks.clear();

                if (auto base = selectedActor->GetActorBase(); base && base->perks) {
                    for (std::uint32_t i = 0; i < base->perkCount; ++i) {
                        if (auto perk = base->perks[i].perk) {
                            const char* perkName = perk->GetFullName();
                            if (perkName && perkName[0] != '\0') {
                                cachedPurchasedPerks.push_back(perkName);
                            }
                        }
                    }
                }
                std::sort(cachedPurchasedPerks.begin(), cachedPurchasedPerks.end());
            }

            ImGuiMCP::PopID();
        }

        void __stdcall Render() {
            if (UI::currentMenuContext != 2) {
                UI::currentMenuContext = 2;
                UI::needsTabSync = true;
                FollowerManager::Refresh();
                StatManager::Harmonize();
            }

            if (ImGuiMCP::Button(Language::GetString("UI_REFRESH_FOLLOWERS"))) {
                FollowerManager::Refresh();
                StatManager::Harmonize();
            }

            auto followers = FollowerManager::GetActorPtrs();

            if (followers.empty()) {
                ImGuiMCP::Text("%s", Language::GetString("UI_NO_FOLLOWER"));
                return;
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (ImGuiMCP::BeginTabBar("FollowersSharedTabBar")) {
                int overviewFlags = (UI::needsTabSync && UI::selectedCompanionID == 0) ? ImGuiMCP::ImGuiTabItemFlags_SetSelected : 0;

                if (ImGuiMCP::BeginTabItem(Language::GetString("UI_OVERVIEW_TAB"), nullptr, overviewFlags)) {
                    UI::selectedCompanionID = 0;

                    ImGuiMCP::BeginChild("OverviewScrollArea", ImGuiMCP::ImVec2(0, 0), false, ImGuiMCP::ImGuiWindowFlags_HorizontalScrollbar);

                    for (size_t i = 0; i < followers.size(); ++i) {
                        auto actorPtr = followers[i];
                        auto actor = actorPtr.get();
                        std::string name = actor ? actorPtr->GetName() : Language::GetString("UI_UNKNOWN_UNLOADED");

                        ImGuiMCP::PushID(actor);
                        ImGuiMCP::BeginChild("FollowerBox", ImGuiMCP::ImVec2(320, 0), true);

                        ImGuiMCP::Text("%s", name.c_str());
                        ImGuiMCP::Separator();

                        if (actor) {
                            auto profile = ProfileManager::GetActorProfile(actor);
                            bool hasAnyPerks = false;
                            std::set<RE::FormID> displayedPerkIDs;

                            if (!profile.skills.empty()) {
                                for (const auto& skill : profile.skills) {
                                    const Perks::PerkTree* tree = PerkManager::GetPerkTree(skill);
                                    if (!tree || tree->nodes.empty()) continue;

                                    std::vector<std::pair<std::string, const Perks::PerkNode*>> ownedPerks;
                                    
                                    for (const auto& nodePtr : tree->nodes) {
                                        auto* node = nodePtr.get();
                                        int rank = PerkManager::GetCurrentRank(actor, node);
                                        if (rank > 0) {
                                            std::string perkLabel = node->name + " (" + std::to_string(rank) + "/" + std::to_string(node->maxRanks) + ")";
                                            ownedPerks.push_back({perkLabel, node});
                                            
                                            for (int r = 0; r < rank && r < node->ranks.size(); ++r) {
                                                if (node->ranks[r]) {
                                                    displayedPerkIDs.insert(node->ranks[r]->GetFormID());
                                                }
                                            }
                                        }
                                    }

                                    if (!ownedPerks.empty()) {
                                        hasAnyPerks = true;
                                        
                                        ImGuiMCP::Spacing();
                                        ImGuiMCP::TextDisabled("%s", GetActorValueName(skill));
                                        
                                        for (const auto& [perkLabel, node] : ownedPerks) {
                                            ImGuiMCP::TextWrapped("- %s", perkLabel.c_str());
                                            ImGuiMCP::SameLine();
                                            ImGuiMCP::TextDisabled("(?)");
                                            RenderPerkTooltip(node, PerkManager::GetCurrentRank(actor, node));
                                        }

                                        ImGuiMCP::Spacing();
                                        ImGuiMCP::PushID(GetActorValueName(skill));
                                        if (ImGuiMCP::Button(Language::GetString("UI_RESET_PERK_TREE"))) {
                                            PerkManager::Refund(actor, tree);
                                        }
                                        ImGuiMCP::PopID();
                                        ImGuiMCP::Separator();
                                    }
                                }
                            }

                            bool hasNativePerks = false;
                            if (auto base = actor->GetActorBase(); base && base->perks) {
                                for (std::uint32_t j = 0; j < base->perkCount; ++j) {
                                    if (auto perk = base->perks[j].perk) {
                                        if (displayedPerkIDs.find(perk->GetFormID()) == displayedPerkIDs.end()) {
                                            if (!hasNativePerks) {
                                                if (hasAnyPerks) ImGuiMCP::Spacing();
                                                ImGuiMCP::TextDisabled("%s", Language::GetString("UI_NATIVE_PERKS"));
                                                hasNativePerks = true;
                                                hasAnyPerks = true;
                                            }
                                            
                                            const char* perkName = perk->GetFullName();
                                            if (perkName && perkName[0] != '\0') {
                                                ImGuiMCP::TextWrapped("- %s", perkName);
                                                ImGuiMCP::SameLine();
                                                ImGuiMCP::TextDisabled("(?)");
                                                if (ImGuiMCP::IsItemHovered()) {
                                                    ImGuiMCP::BeginTooltip();
                                                    RE::BSString perkDesc;
                                                    perk->GetDescription(perkDesc, perk);
                                                    std::string descStr = perkDesc.c_str();
                                                    if (descStr.empty()) {
                                                        descStr = Language::GetString("UI_NO_DESCRIPTION");
                                                    }
                                                    ImGuiMCP::PushTextWrapPos(400.0f);
                                                    ImGuiMCP::TextWrapped("%s", descStr.c_str());
                                                    ImGuiMCP::PopTextWrapPos();
                                                    ImGuiMCP::EndTooltip();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            
                            if (hasNativePerks) {
                                ImGuiMCP::Spacing();
                                ImGuiMCP::Separator();
                            }

                            if (!hasAnyPerks) {
                                ImGuiMCP::TextDisabled("%s", Language::GetString("UI_NO_PERKS_OWNED"));
                            }
                        } else {
                            ImGuiMCP::TextDisabled("%s", Language::GetString("UI_ACTOR_INVALID"));
                        }

                        if (ImGuiMCP::Button(Language::GetString("UI_RESET_ALL_PERK_TREES"))) {
                            PerkManager::Refund(actor);
                        }

                        if (ImGuiMCP::Button(Language::GetString("UI_RESET_ALL_PERKS"))) {
                            PerkManager::RefundAll(actor);
                        }

                        ImGuiMCP::EndChild();
                        ImGuiMCP::PopID();

                        if (i < followers.size() - 1) {
                            ImGuiMCP::SameLine();
                        }
                    }

                    ImGuiMCP::EndChild();
                    ImGuiMCP::EndTabItem();
                }

                for (size_t i = 0; i < followers.size(); ++i) {
                    auto actor = followers[i].get();
                    uintptr_t actorID = reinterpret_cast<uintptr_t>(actor);
                    std::string baseName = actor ? followers[i]->GetName() : Language::GetString("UI_UNKNOWN_UNLOADED");
                    std::string tabName = baseName + "###FollowerTab_" + std::to_string(actorID);

                    int tabFlags = (UI::needsTabSync && UI::selectedCompanionID == actorID) ? ImGuiMCP::ImGuiTabItemFlags_SetSelected : 0;

                    if (ImGuiMCP::BeginTabItem(tabName.c_str(), nullptr, tabFlags)) {
                        UI::selectedCompanionID = actorID;

                        if (actor) {
                            RenderSingleFollowerTab(actor);
                        } else {
                            ImGuiMCP::Text("%s", Language::GetString("UI_ACTOR_INVALID"));
                        }
                        ImGuiMCP::EndTabItem();
                    }
                }

                ImGuiMCP::EndTabBar();
            }
            UI::needsTabSync = false;
        }
    }
}