# Companions' Path - Technical Architecture

This document outlines the internal architecture of the Companions' Path SKSE mod.

## 🏗️ Core Modules

The architecture is divided into five distinct layers:

* **`ProfileRepository` (Data Loading & Templates):** Responsible for parsing the `config.json` file. It holds the static definitions of "Base Stats" based on tags and races. It does not know about current game state or actors' current points.
* **`StatStorage` (State & Persistence):** The internal database. It manages the SKSE co-save integration (`Load`, `Save`, `Revert` callbacks) and holds the in-memory map of how many points each specific companion has spent.
* **`StatRules` (Domain Logic):** A pure, stateless rules engine. It dictates the math of the mod (e.g., maximum points, step increments, calculating the final stat value based on base + points). It never writes data; it only answers questions.
* **`StatManager` (The Orchestrator):** The command center. It bridges the UI, the Rules, the Storage, and the Skyrim Engine. It is the only module authorized to actually apply stat changes to the game world.
* **`UI` & `EventManager` (Presentation & Triggers):** The front-end. `UI` draws the ImGui interface, while `EventManager` listens for Skyrim events (like opening the Level Up menu) or SKSE menu events. They are completely "dumb" and simply pass user intent to the `StatManager`

## 🔄 Data & Execution Flow

To understand how the mod works, here are the three primary execution flows.

### 1. Initialization Flow (Game Launch)
When the game starts and SKSE loads the plugin, the environment is set up without touching any live game data.

1.  **`plugin.cpp`** receives `kDataLoaded` and `kPostLoad` messages.
2.  Calls **`StatStorage::Register()`** -> Hooks into SKSE save/load system.
3.  Calls **`EventManager::Register()`** -> Hooks into Skyrim's UI event sinks.
4.  Calls **`UI::Register()`** -> Registers the ImGui menu with SKSEMenuFramework.
5.  Calls **`ProfileRepository::Initialize()`** -> Parses `config.json` and builds static profiles in memory.

### 2. The Read Flow (Opening the Menu)
When the user opens the mod menu and views a companion's stats.

1.  **`UI::Stats::Render()`** is triggered.
2.  UI needs to display current points -> Calls **`StatManager::GetRemainingAttributePoints(actor)`**.
3.  **`StatManager`** queries **`StatStorage`** for spent points, and **`StatRules`** for the max allowed points, returning the difference to the UI.
4.  UI needs to display the actual stat value (e.g., Health) -> Calls **`StatManager::GetStat(actor, kHealth)`**.
5.  **`StatManager`** gets the raw points from **`StatStorage`**, passes them into **`StatRules::CalculateStatValue()`**, and returns the final mathematical result to the UI.

### 3. The Write Flow (Adding a Stat Point)
When the user clicks the `+` button in the UI to increase a stat.

1.  **`UI::Stats::Render()`** registers the click and calls **`StatManager::AddPoint(actor, kHealth)`**.
2.  **`StatManager`** calls **`StatManager::HasPointsLeft()`** (which consults `StatRules`) to validate the action.
3.  If valid, **`StatManager`** orchestrates the update:
    * Calculates the new total value using **`StatRules::CalculateStatValue()`**.
    * Applies the physical change to the game engine via `actor->AsActorValueOwner()->RestoreActorValue()`.
    * Saves the new point allocation into memory via **`StatStorage::SetStat()`**.

### 4. The Harmonize Flow (Synchronization)
Triggered on events (like the player leveling up) to ensure companion stats match their expected values based on level and spent points.

1.  **`EventManager`** detects a level up (or user clicks "Refresh").
2.  Calls **`StatManager::Harmonize()`**.
3.  **`StatManager`** iterates through all active followers.
4.  For each follower, it re-applies their base attributes (from `ProfileRepository`) and their spent points (from `StatStorage`) to the Skyrim Engine to prevent desyncs.
