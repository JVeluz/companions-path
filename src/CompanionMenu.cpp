#include "CompanionMenu.h"
#include "SKSEMenuFramework.h"
#include "StatEditor.h"

std::vector<RE::Actor*> CompanionMenu::currentFollowers;
int CompanionMenu::selectedCompanionIndex = 0;

std::vector<RE::Actor*> GetActiveFollowers()
{
    std::vector<RE::Actor*> followers;
    auto followerFaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x5C84E, "Skyrim.esm");
    auto processLists = RE::ProcessLists::GetSingleton();
    
    if (!followerFaction || !processLists) return followers;

    for (auto& actorHandle : processLists->highActorHandles) {
        auto actorPtr = actorHandle.get();
        if (actorPtr && actorPtr->IsInFaction(followerFaction)) {
            followers.push_back(actorPtr.get());
        }
    }
    return followers;
}

void RenderStatRow(RE::Actor *selectedActor, const char* name, RE::ActorValue actorValue)
{
    ImGuiMCP::PushID(name);

    auto owner = selectedActor->AsActorValueOwner();
    float val = owner->GetActorValue(actorValue);

    if (ImGuiMCP::SmallButton(" - ")) {
        StatEditor::RemovePoint(selectedActor, actorValue);
    }
    
    ImGuiMCP::SameLine();
    
    if (ImGuiMCP::SmallButton(" + ")) {
        StatEditor::AddPoint(selectedActor, actorValue);
    }

    ImGuiMCP::SameLine();

    ImGuiMCP::Text(" %3.0f  %s", val, name);

    ImGuiMCP::PopID();
}

void CompanionMenu::Register()
{
    SKSEMenuFramework::SetSection("Companions"); 
    SKSEMenuFramework::AddSectionItem("Level Up", CompanionMenu::Render); 
}

void CompanionMenu::Render()
{
    ImGuiMCP::SetNextItemWidth(200.0f);
    if (currentFollowers.empty()) {
        ImGuiMCP::Text("No active follower.");
    } 
    else {
        std::vector<const char*> names;
        for (auto* actor : currentFollowers) {
            names.push_back(actor->GetName());
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

    auto selectedActor = currentFollowers[selectedCompanionIndex];

    int remainingAttributePoints = StatEditor::GetRemainingAttributePoints(selectedActor);
    int remainingSkillPoints = StatEditor::GetRemainingSkillPoints(selectedActor);
    int maxAttributePoints = StatEditor::GetMaxAttributePoints(selectedActor);
    int maxSkillPoints = StatEditor::GetMaxSkillPoints(selectedActor);

    ImGuiMCP::Text("Level : %d", selectedActor->GetLevel());
    ImGuiMCP::Spacing();

    ImGuiMCP::Columns(2, "StatsColumns", true);

    ImGuiMCP::Text("=== ATTRIBUTES ===");
    ImGuiMCP::Text("Points: %d / %d", remainingAttributePoints, maxAttributePoints);
    ImGuiMCP::Spacing();

    RenderStatRow(selectedActor, "Health", RE::ActorValue::kHealth);
    RenderStatRow(selectedActor, "Magicka", RE::ActorValue::kMagicka);
    RenderStatRow(selectedActor, "Stamina", RE::ActorValue::kStamina);
    
    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button("Reset Attributes", ImGuiMCP::ImVec2(-1, 0)))
        StatEditor::ResetAttributes(selectedActor);

    ImGuiMCP::NextColumn();

    ImGuiMCP::Text("=== SKILLS ===");
    ImGuiMCP::Text("Points: %d / %d", remainingSkillPoints, maxSkillPoints);
    ImGuiMCP::Spacing();

    RenderStatRow(selectedActor, "One-Handed", RE::ActorValue::kOneHanded);
    RenderStatRow(selectedActor, "Two-Handed", RE::ActorValue::kTwoHanded);
    RenderStatRow(selectedActor, "Block", RE::ActorValue::kBlock);
    RenderStatRow(selectedActor, "Heavy Armor", RE::ActorValue::kHeavyArmor);
    RenderStatRow(selectedActor, "Light Armor", RE::ActorValue::kLightArmor);
    RenderStatRow(selectedActor, "Archery", RE::ActorValue::kArchery);
    RenderStatRow(selectedActor, "Sneak", RE::ActorValue::kSneak);
    RenderStatRow(selectedActor, "Destruction", RE::ActorValue::kDestruction);
    RenderStatRow(selectedActor, "Restoration", RE::ActorValue::kRestoration);
    RenderStatRow(selectedActor, "Conjuration", RE::ActorValue::kConjuration);
    RenderStatRow(selectedActor, "Illusion", RE::ActorValue::kIllusion);
    RenderStatRow(selectedActor, "Alteration", RE::ActorValue::kAlteration);
    
    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button("Reset Skills", ImGuiMCP::ImVec2(-1, 0)))
        StatEditor::ResetSkills(selectedActor);

    ImGuiMCP::Columns(1);
}