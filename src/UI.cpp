#include "UI.h"

#include "Utils.h"
#include "StatManager.h"
#include "StatRules.h"
#include "ProfileRepository.h"
#include "LanguageRepository.h"
#include "ConfigManager.h"

#include <vector>
#include <string>

namespace {
    std::vector<RE::ActorHandle> currentFollowers;
    int selectedCompanionIndex = 0;

    const char* GetActorValueName(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kHealth:        return LanguageRepository::GetString("STAT_HEALTH");
            case RE::ActorValue::kMagicka:       return LanguageRepository::GetString("STAT_MAGICKA");
            case RE::ActorValue::kStamina:       return LanguageRepository::GetString("STAT_STAMINA");
            
            case RE::ActorValue::kUnarmedDamage: return LanguageRepository::GetString("STAT_UNARMED_DAMAGE");
            case RE::ActorValue::kDamageResist:  return LanguageRepository::GetString("STAT_DAMAGE_RESIST");
            
            case RE::ActorValue::kOneHanded:     return LanguageRepository::GetString("STAT_ONE_HANDED");
            case RE::ActorValue::kTwoHanded:     return LanguageRepository::GetString("STAT_TWO_HANDED");
            case RE::ActorValue::kBlock:         return LanguageRepository::GetString("STAT_BLOCK");
            case RE::ActorValue::kHeavyArmor:    return LanguageRepository::GetString("STAT_HEAVY_ARMOR");
            case RE::ActorValue::kLightArmor:    return LanguageRepository::GetString("STAT_LIGHT_ARMOR");
            case RE::ActorValue::kArchery:       return LanguageRepository::GetString("STAT_ARCHERY");
            
            case RE::ActorValue::kDestruction:   return LanguageRepository::GetString("STAT_DESTRUCTION");
            case RE::ActorValue::kRestoration:   return LanguageRepository::GetString("STAT_RESTORATION");
            case RE::ActorValue::kAlteration:    return LanguageRepository::GetString("STAT_ALTERATION");
            case RE::ActorValue::kConjuration:   return LanguageRepository::GetString("STAT_CONJURATION");
            case RE::ActorValue::kIllusion:      return LanguageRepository::GetString("STAT_ILLUSION");
            
            case RE::ActorValue::kSneak:         return LanguageRepository::GetString("STAT_SNEAK");
            case RE::ActorValue::kLockpicking:   return LanguageRepository::GetString("STAT_LOCKPICKING");
            case RE::ActorValue::kPickpocket:    return LanguageRepository::GetString("STAT_PICKPOCKET");
            case RE::ActorValue::kSpeech:        return LanguageRepository::GetString("STAT_SPEECH");
            case RE::ActorValue::kAlchemy:       return LanguageRepository::GetString("STAT_ALCHEMY");
            case RE::ActorValue::kSmithing:      return LanguageRepository::GetString("STAT_SMITHING");
            case RE::ActorValue::kEnchanting:    return LanguageRepository::GetString("STAT_ENCHANTING");
            
            default:                             return LanguageRepository::GetString("STAT_UNKNOWN");
        }
    }

    void RenderStatRow(RE::Actor *actor, RE::ActorValue actorValue) {
        const char* name = GetActorValueName(actorValue);

        ImGuiMCP::PushID(name);

        float value = StatManager::GetStatValue(actor, actorValue);

        ImGuiMCP::Text("%3.0f %s", value, name);

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

        ImGuiMCP::PopID();
    }
}

namespace UI {
    void Register() {
        if (SKSEMenuFramework::IsInstalled()) {
            static std::string tabStats = LanguageRepository::GetString("UI_TAB_STATS");
            SKSEMenuFramework::SetSection("Companions' Path"); 
            SKSEMenuFramework::AddSectionItem(LanguageRepository::GetString("UI_TAB_STATS"), UI::Stats::Render); 
        }
    }

    namespace Stats {
        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);
            if (currentFollowers.empty()) {
                ImGuiMCP::Text("%s", LanguageRepository::GetString("UI_NO_FOLLOWER"));
            } 
            else {
                std::vector<const char*> names;
                static std::string unloadedStr = LanguageRepository::GetString("UI_UNKNOWN_UNLOADED");
                
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
            if (ImGuiMCP::Button(LanguageRepository::GetString("UI_REFRESH_FOLLOWERS"))) {
                currentFollowers = Utils::GetActiveFollowers();
                selectedCompanionIndex = 0;
                StatManager::Harmonize();
            }

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(LanguageRepository::GetString("UI_RELOAD_CONFIG"))) {
                ConfigManager::LoadConfig("Data/SKSE/Plugins/CompanionsPath/config.json");
                StatManager::Harmonize();
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            
            if (currentFollowers.empty()) 
                return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= currentFollowers.size()) 
                return;

            auto selectedActorNiPtr = currentFollowers[selectedCompanionIndex].get();
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("%s", LanguageRepository::GetString("UI_ACTOR_INVALID"));
                return;
            }
            
            auto selectedActor = selectedActorNiPtr.get();
            auto profile = ProfileRepository::GetProfileForActor(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = StatRules::GetTotalAttributePoints(selectedActor);
            int maxSkillPoints = StatRules::GetTotalSkillPoints(selectedActor);

            ImGuiMCP::Text(LanguageRepository::GetString("UI_LEVEL"), selectedActor->GetLevel());

            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            ImGuiMCP::Spacing();
            
            ImGuiMCP::Text(LanguageRepository::GetString("UI_ATTRIBUTES"), remainingAttributePoints, maxAttributePoints);
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
            
            ImGuiMCP::Text(LanguageRepository::GetString("UI_SKILLS"), remainingSkillPoints, maxSkillPoints);
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

            if (ImGuiMCP::Button(LanguageRepository::GetString("UI_RESET_ATTRIBUTES"))) {
                StatManager::ResetAttributes(selectedActor);
            }
    
            ImGuiMCP::SameLine();
            
            if (ImGuiMCP::Button(LanguageRepository::GetString("UI_RESET_SKILLS"))) {
                StatManager::ResetSkills(selectedActor);
            }
        }
    }
}