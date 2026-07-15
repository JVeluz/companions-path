#include "UI.h"

#include "Utils.h"
#include "StatManager.h"
#include "StatRules.h"
#include "ConfigManager.h"
#include "language.h"
#include "profile.h"

#include <vector>
#include <string>

namespace {
    std::vector<RE::ActorHandle> currentFollowers;
    int selectedCompanionIndex = 0;

    const char* GetActorValueName(RE::ActorValue actorValue) {
        switch (actorValue) {
            case RE::ActorValue::kHealth:        return TranslationService::GetString("STAT_HEALTH");
            case RE::ActorValue::kMagicka:       return TranslationService::GetString("STAT_MAGICKA");
            case RE::ActorValue::kStamina:       return TranslationService::GetString("STAT_STAMINA");
            
            case RE::ActorValue::kUnarmedDamage: return TranslationService::GetString("STAT_UNARMED_DAMAGE");
            case RE::ActorValue::kDamageResist:  return TranslationService::GetString("STAT_DAMAGE_RESIST");
            
            case RE::ActorValue::kOneHanded:     return TranslationService::GetString("STAT_ONE_HANDED");
            case RE::ActorValue::kTwoHanded:     return TranslationService::GetString("STAT_TWO_HANDED");
            case RE::ActorValue::kBlock:         return TranslationService::GetString("STAT_BLOCK");
            case RE::ActorValue::kHeavyArmor:    return TranslationService::GetString("STAT_HEAVY_ARMOR");
            case RE::ActorValue::kLightArmor:    return TranslationService::GetString("STAT_LIGHT_ARMOR");
            case RE::ActorValue::kArchery:       return TranslationService::GetString("STAT_ARCHERY");
            
            case RE::ActorValue::kDestruction:   return TranslationService::GetString("STAT_DESTRUCTION");
            case RE::ActorValue::kRestoration:   return TranslationService::GetString("STAT_RESTORATION");
            case RE::ActorValue::kAlteration:    return TranslationService::GetString("STAT_ALTERATION");
            case RE::ActorValue::kConjuration:   return TranslationService::GetString("STAT_CONJURATION");
            case RE::ActorValue::kIllusion:      return TranslationService::GetString("STAT_ILLUSION");
            
            case RE::ActorValue::kSneak:         return TranslationService::GetString("STAT_SNEAK");
            case RE::ActorValue::kLockpicking:   return TranslationService::GetString("STAT_LOCKPICKING");
            case RE::ActorValue::kPickpocket:    return TranslationService::GetString("STAT_PICKPOCKET");
            case RE::ActorValue::kSpeech:        return TranslationService::GetString("STAT_SPEECH");
            case RE::ActorValue::kAlchemy:       return TranslationService::GetString("STAT_ALCHEMY");
            case RE::ActorValue::kSmithing:      return TranslationService::GetString("STAT_SMITHING");
            case RE::ActorValue::kEnchanting:    return TranslationService::GetString("STAT_ENCHANTING");
            
            default:                             return TranslationService::GetString("STAT_UNKNOWN");
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
            static std::string tabStats = TranslationService::GetString("UI_TAB_STATS");
            SKSEMenuFramework::SetSection("Companions' Path"); 
            SKSEMenuFramework::AddSectionItem(TranslationService::GetString("UI_TAB_STATS"), UI::Stats::Render);
            SKSEMenuFramework::AddSectionItem(TranslationService::GetString("UI_TAB_SETTINGS"), UI::Settings::Render);
        }
    }

    namespace Stats {
        void __stdcall Render() {
            ImGuiMCP::SetNextItemWidth(200.0f);
            if (currentFollowers.empty()) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_NO_FOLLOWER"));
            } 
            else {
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
            
            if (currentFollowers.empty()) 
                return;
            if (selectedCompanionIndex < 0 || selectedCompanionIndex >= currentFollowers.size()) 
                return;

            auto selectedActorNiPtr = currentFollowers[selectedCompanionIndex].get();
            if (!selectedActorNiPtr) {
                ImGuiMCP::Text("%s", TranslationService::GetString("UI_ACTOR_INVALID"));
                return;
            }
            
            auto selectedActor = selectedActorNiPtr.get();
            auto profile = ProfileParser::GetProfile(selectedActor);

            int remainingAttributePoints = StatManager::GetRemainingAttributePoints(selectedActor);
            int remainingSkillPoints = StatManager::GetRemainingSkillPoints(selectedActor);
            int maxAttributePoints = StatRules::GetTotalAttributePoints(selectedActor);
            int maxSkillPoints = StatRules::GetTotalSkillPoints(selectedActor);

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
}