# Companions' Path

**Companions' Path** is an SKSE plugin that allows you to take complete control over your followers' progression. Instead of relying on auto-leveling, this mod introduces a custom in-game UI where you can manually allocate Attribute and Skill points for your active companions as they level up, just like you do for your own character.

## 🌟 Key Features

* **Manual Stat Allocation:** Distribute points into Attributes (Health, Magicka, Stamina) and Skills (One-Handed, Destruction, Sneak, etc.) for any active follower.
* **Level-Based Progression:** Companions earn **1 Attribute point** and **5 Skill points** per level.
* **Dynamic Race & Tag Support:** Automatically applies distinct starting bonuses based on a follower's race (e.g., High Elves get +50 base Magicka, Orcs get Heavy Armor bonuses).
* **Creature Companion Support:** Fully supports non-humanoid followers via keyword tags. Creatures can be configured to use alternative skills like *Unarmed Damage* and *Damage Resist* instead of standard weapon and armor skills.
* **Hot-Reloadable Configuration:** Tweak starting stats and racial bonuses in a straightforward JSON file and reload them directly from the in-game menu without restarting Skyrim.

## 📋 Requirements

* **Skyrim Special Edition / Anniversary Edition**
* **SKSE64** (Skyrim Script Extender)
* **Dear ImGui for Skyrim** (or the specific ImGui/Menu Framework required by `ImGuiMCP` / `SKSEMenuFramework`)

## 🎮 How to Use

1. **Access the Menu:** Open the mod's interface via the "Companions' Path" section in your ImGui menu overlay (F1).
2. **Select a Follower:** Choose from your currently active followers using the dropdown menu. If a follower is missing, click **Refresh Followers**.
3. **Allocate Points:** * **Attributes:** Each point spent increases the attribute (Health/Magicka/Stamina) by **10**.
* **Skills:** Each point spent increases the skill by **1**.

4. **Reset:** Made a mistake? Use the "Reset Attributes" or "Reset All Skills" buttons to refund your spent points.

## ⚙️ Configuration (`config.json`)

The mod relies on `Data/SKSE/Plugins/CompanionsPath/config.json` to define base starting stats, racial bonuses, and custom skill profiles.

### Understanding the JSON Structure

* **`Races`**: Defines base stat overrides for specific races. For example, setting a Nord's base `kTwoHanded` to 25.0, simulating vanilla racial starting bonuses.
* **`Tags`**: Defines entirely custom stat profiles based on Actor Keywords. This is primarily used for non-humanoids.

**Example: Creature Profile**

```json
"Creature": {
  "Attributes": ["kHealth", "kStamina"],
  "Skills": ["kUnarmedDamage", "kDamageResist"],
  "BaseValues": {
    "kHealth": 100.0,
    "kStamina": 100.0,
    "kUnarmedDamage": 15.0,
    "kDamageResist": 15.0
  }
}

```

*If an actor has the "Creature" or "ActorTypeCreature" keyword, the menu will automatically swap out standard humanoid skills (like Archery or Lockpicking) for Unarmed Damage and Damage Resist.*

> **Tip:** You can edit `config.json` while the game is running. Simply click the **"Reload Config (JSON)"** button in the mod menu to instantly apply your changes to your followers.

## 🛠️ Technical Details for Modders

* **Stat Application:** The mod uses `SetBaseActorValue` for standard skills and `RestoreActorValue` with `kPermanent` modifiers for calculated stats (like Damage Resist or Unarmed Damage) to ensure compatibility with game engine mechanics.
* **Event Sinks:** The plugin hooks into `RE::StatsMenu` via a `MenuOpenCloseEvent`. When the player finishes leveling up and closes the menu, the mod automatically harmonizes follower stats, recalculating max points based on their new level.