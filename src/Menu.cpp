#include "Menu.h"

std::vector<RE::ActorHandle> Menu::currentFollowers;
int Menu::selectedCompanionIndex = 0;

static const char* GetActorValueName(RE::ActorValue actorValue) {
    switch (actorValue) {
        case RE::ActorValue::kHealth:      return "Health";
        case RE::ActorValue::kMagicka:     return "Magicka";
        case RE::ActorValue::kStamina:     return "Stamina";
        
        case RE::ActorValue::kOneHanded:   return "One-Handed";
        case RE::ActorValue::kTwoHanded:   return "Two-Handed";
        case RE::ActorValue::kBlock:       return "Block";
        case RE::ActorValue::kHeavyArmor:  return "Heavy Armor";
        case RE::ActorValue::kLightArmor:  return "Light Armor";
        case RE::ActorValue::kArchery:     return "Archery";
        
        case RE::ActorValue::kDestruction: return "Destruction";
        case RE::ActorValue::kRestoration: return "Restoration";
        case RE::ActorValue::kAlteration:  return "Alteration";
        case RE::ActorValue::kConjuration: return "Conjuration";
        case RE::ActorValue::kIllusion:    return "Illusion";
        
        case RE::ActorValue::kSneak:       return "Sneak";
        case RE::ActorValue::kLockpicking: return "Lockpicking";
        case RE::ActorValue::kPickpocket:  return "Pickpocket";
        case RE::ActorValue::kSpeech:      return "Speech";
        case RE::ActorValue::kAlchemy:     return "Alchemy";
        case RE::ActorValue::kSmithing:    return "Smithing";
        case RE::ActorValue::kEnchanting:  return "Enchanting";
        
        default:                           return "Unknown";
    }
}

static std::vector<RE::ActorHandle> GetActiveFollowers()
{
    std::vector<RE::ActorHandle> followers;
    auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
    auto processLists = RE::ProcessLists::GetSingleton();
    
    if (!followerFaction || !processLists) return followers;

    for (auto& actorHandle : processLists->highActorHandles) {
        auto actorPtr = actorHandle.get();
        if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
            followers.push_back(actorHandle);
        }
    }
    return followers;
}

static void RenderStatRow(RE::Actor *selectedActor, RE::ActorValue actorValue)
{
    const char* name = GetActorValueName(actorValue);

    ImGuiMCP::PushID(name);

    auto owner = selectedActor->AsActorValueOwner();
    float value = owner->GetBaseActorValue(actorValue);

    if (ImGuiMCP::SmallButton(" << ")) {
        for(int i = 0; i < 5; ++i) 
            StatEditor::RemovePoint(selectedActor, actorValue);
    }
    ImGuiMCP::SameLine();
    
    if (ImGuiMCP::SmallButton(" - ")) {
        StatEditor::RemovePoint(selectedActor, actorValue);
    }
    
    ImGuiMCP::SameLine();
    
    if (ImGuiMCP::SmallButton(" + ")) {
        StatEditor::AddPoint(selectedActor, actorValue);
    }
    
    ImGuiMCP::SameLine();

    if (ImGuiMCP::SmallButton(" >> ")) {
        for(int i = 0; i < 5; ++i) 
            StatEditor::AddPoint(selectedActor, actorValue);
    }

    ImGuiMCP::SameLine();

    ImGuiMCP::Text(" %3.0f  %s", value, name);

    ImGuiMCP::PopID();
}

void Menu::Register()
{
    SKSEMenuFramework::SetSection("Companions' Path"); 
    SKSEMenuFramework::AddSectionItem("Level Up", Menu::Render); 
}

void Menu::Render()
{
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
        currentFollowers = GetActiveFollowers();
        selectedCompanionIndex = 0;
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

    int remainingAttributePoints = StatEditor::GetRemainingAttributePoints(selectedActor);
    int remainingSkillPoints = StatEditor::GetRemainingSkillPoints(selectedActor);
    int maxAttributePoints = StatEditor::GetTotalAttributePoints(selectedActor);
    int maxSkillPoints = StatEditor::GetTotalSkillPoints(selectedActor);

    ImGuiMCP::Text("Level : %d", selectedActor->GetLevel());
    ImGuiMCP::Text("Attribute Points: %d / %d", remainingAttributePoints, maxAttributePoints);
    ImGuiMCP::Text("Skill Points:     %d / %d", remainingSkillPoints, maxSkillPoints);
    
    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    ImGuiMCP::Text("Attributes");

    ImGuiMCP::Columns(3, "AttributesColumns", false);
    
    RenderStatRow(selectedActor, RE::ActorValue::kHealth);
    ImGuiMCP::NextColumn();
    
    RenderStatRow(selectedActor, RE::ActorValue::kMagicka);
    ImGuiMCP::NextColumn();

    RenderStatRow(selectedActor, RE::ActorValue::kStamina);

    ImGuiMCP::Columns(1); 

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button("Reset Attributes")) {
        StatEditor::ResetAttributes(selectedActor);
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    ImGuiMCP::Columns(3, "StatsColumns", false);

    ImGuiMCP::Text("Combat");
    ImGuiMCP::Spacing();
    for (const auto& skill : Stats::CombatSkills) {
        RenderStatRow(selectedActor, skill);
    }

    ImGuiMCP::NextColumn();

    ImGuiMCP::Text("Magic");
    ImGuiMCP::Spacing();
    for (const auto& skill : Stats::MagicSkills) {
        RenderStatRow(selectedActor, skill);
    }

    ImGuiMCP::NextColumn();

    ImGuiMCP::Text("Miscellaneous");
    ImGuiMCP::Spacing();
    for (const auto& skill : Stats::MiscSkills) {
        RenderStatRow(selectedActor, skill);
    }

    ImGuiMCP::Columns(1); 

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();
    
    if (ImGuiMCP::Button("Reset All Skills", ImGuiMCP::ImVec2(-1, 0))) {
        StatEditor::ResetSkills(selectedActor);
    }
}