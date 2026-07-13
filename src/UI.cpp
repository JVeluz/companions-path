#include "UI.h"

#include <vector>

#include "StatManager.h"
#include "StatRules.h"
#include "ProfileRepository.h"
#include "Utils.h"

namespace {
    std::vector<RE::ActorHandle> currentFollowers;
    int selectedCompanionIndex = 0;

    const char* GetActorValueName(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kHealth:        return "Health";
            case RE::ActorValue::kMagicka:       return "Magicka";
            case RE::ActorValue::kStamina:       return "Stamina";
            
            case RE::ActorValue::kUnarmedDamage: return "Unarmed Damage";
            case RE::ActorValue::kDamageResist:  return "Damage Resist";
            
            case RE::ActorValue::kOneHanded:     return "One-Handed";
            case RE::ActorValue::kTwoHanded:     return "Two-Handed";
            case RE::ActorValue::kBlock:         return "Block";
            case RE::ActorValue::kHeavyArmor:    return "Heavy Armor";
            case RE::ActorValue::kLightArmor:    return "Light Armor";
            case RE::ActorValue::kArchery:       return "Archery";
            
            case RE::ActorValue::kDestruction:   return "Destruction";
            case RE::ActorValue::kRestoration:   return "Restoration";
            case RE::ActorValue::kAlteration:    return "Alteration";
            case RE::ActorValue::kConjuration:   return "Conjuration";
            case RE::ActorValue::kIllusion:      return "Illusion";
            
            case RE::ActorValue::kSneak:         return "Sneak";
            case RE::ActorValue::kLockpicking:   return "Lockpicking";
            case RE::ActorValue::kPickpocket:    return "Pickpocket";
            case RE::ActorValue::kSpeech:        return "Speech";
            case RE::ActorValue::kAlchemy:       return "Alchemy";
            case RE::ActorValue::kSmithing:      return "Smithing";
            case RE::ActorValue::kEnchanting:    return "Enchanting";
            
            default:                             return "Unknown";
        }
    }

    void RenderStatRow(RE::Actor *actor, RE::ActorValue actorValue) {
        const char* name = GetActorValueName(actorValue);

        ImGuiMCP::PushID(name);

        float value = StatManager::GetStatValue(actor, actorValue);

        if (ImGuiMCP::SmallButton(" << ")) {
            for(int i = 0; i < 5; ++i) 
                StatManager::RemovePoint(actor, actorValue);
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
            for(int i = 0; i < 5; ++i) 
                StatManager::AddPoint(actor, actorValue);
        }

        ImGuiMCP::SameLine();

        ImGuiMCP::Text(" %3.0f  %s", value, name);

        ImGuiMCP::PopID();
    }
}

namespace UI {
    void Register() {
        SKSEMenuFramework::SetSection("Companions' Path"); 
        SKSEMenuFramework::AddSectionItem("Stats", UI::Stats::Render); 
    }

    namespace Stats {
        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);
            if (currentFollowers.empty()) {
                ImGuiMCP::Text("No active follower.");
            } 
            else {
                std::vector<const char*> names;
                for (auto& handle : currentFollowers) {
                    auto actorPtr = handle.get();
                    if (actorPtr) {
                        names.push_back(actorPtr->GetName());
                    } else {
                        names.push_back("Unknown (Unloaded)");
                    }
                }
                ImGuiMCP::Combo("##Target", &selectedCompanionIndex, names.data(), static_cast<int>(names.size()));
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button("Refresh Followers")) {
                currentFollowers = Utils::GetActiveFollowers();
                selectedCompanionIndex = 0;
                StatManager::Harmonize();
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button("Reload Config")) {
                ProfileRepository::Initialize("Data/SKSE/Plugins/CompanionsPath/config.json");
                StatManager::Harmonize();
            }

            ImGuiMCP::Separator();
            
            if (currentFollowers.empty()) return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= currentFollowers.size()) return;

            auto selectedActorNiPtr = currentFollowers[selectedCompanionIndex].get();
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("Actor is currently unloaded or invalid.");
                return;
            }
            
            auto selectedActor = selectedActorNiPtr.get();
            auto profile = ProfileRepository::GetProfileForActor(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = StatRules::GetTotalAttributePoints(selectedActor);
            int maxSkillPoints = StatRules::GetTotalSkillPoints(selectedActor);

            ImGuiMCP::Text("Level : %d", selectedActor->GetLevel());
            ImGuiMCP::Text("Attribute Points: %d / %d", remainingAttributePoints, maxAttributePoints);
            ImGuiMCP::Text("Skill Points:     %d / %d", remainingSkillPoints, maxSkillPoints);
            
            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text("Attributes");

            ImGuiMCP::Columns(3, "AttributesColumns", false);
            
            for (const auto& attr : profile.Attributes) {
                RenderStatRow(selectedActor, attr);
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1); 

            ImGuiMCP::Spacing();
            if (ImGuiMCP::Button("Reset Attributes")) {
                StatManager::ResetAttributes(selectedActor);
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::Text("Skills");
            ImGuiMCP::Spacing();
            
            ImGuiMCP::Columns(3, "StatsColumns", false);

            for (const auto& skill : profile.Skills) {
                RenderStatRow(selectedActor, skill);
                ImGuiMCP::NextColumn();
            }

            ImGuiMCP::Columns(1); 

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();
            
            if (ImGuiMCP::Button("Reset All Skills", ImGuiMCP::ImVec2(-1, 0))) {
                StatManager::ResetSkills(selectedActor);
            }
        }
    }
}