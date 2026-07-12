# Technical Architecture

This document outlines the internal architecture of **Companions' Path**. The mod is structured with a clear separation of concerns, ensuring that the UI, the core logic, data persistence, and Skyrim engine interactions remain decoupled.

## 🏗️ Core Modules

The codebase is divided into five main layers. The general flow of dependency is strictly one-way: **UI -> Logic -> Data & Engine**.

### 1. `Menu` (Frontend / UI)
* **Responsibility:** Renders the ImGui interface. It handles user inputs (button clicks, dropdown selections).
* **Call Direction:** It **reads** from `Stats` and **calls** `StatEditor` to perform any mutations. It *never* directly modifies Skyrim actor values.

### 2. `StatEditor` (Core Logic)
* **Responsibility:** The central mediator. It enforces the rules (e.g., checking if the actor has enough points left).
* **Call Direction:** 
  * Receives commands from `Menu`.
  * Fetches step/base values from `Stats`.
  * Commits "spent points" to `Data`.
  * Pushes the final computed values directly to the **Skyrim Engine** (`SetBaseActorValue` or `RestoreActorValue`).

### 3. `Stats` & `ConfigParser` (Configuration)
* **Responsibility:** `ConfigParser` reads the `config.json` at startup. `Stats` holds these parsed `StatProfile` objects in memory and determines which profile applies to a given `RE::Actor` based on their Race or Keywords.
* **Call Direction:** Read-only reference class. Queried by `Menu` and `StatEditor`.

### 4. `Data` (Persistence)
* **Responsibility:** SKSE Co-save serialization. 
* **Important Note:** We do *not* save the absolute stat values. We only save the **number of points spent** (integers) by the player per `ActorValue` per `FormID`. The actual float values are recalculated dynamically to ensure safe uninstallation or config tweaks.
* **Call Direction:** Handles SKSE serialization callbacks (`SaveCallback`, `LoadCallback`). Modified only by `StatEditor`.

### 5. `LevelUpEventSink` (Event Listeners)
* **Responsibility:** Hooks into Skyrim's UI events.
* **Call Direction:** Listens for the vanilla `StatsMenu` closing, then calls `StatEditor::Harmonize()` to automatically recalculate follower point caps when the player levels up.

## 🔄 Call Flow Examples

Here is the step-by-step execution flow for common actions.

### Action: The player clicks the "+" button to increase Health
1. `Menu::Render()` detects a click on the "+" button.
2. ➔ Calls `StatEditor::AddPoint(actor, kHealth)`.
3. ➔ `StatEditor` queries `HasPointsLeft()` to validate the transaction.
4. ➔ `StatEditor` gets the current spent points from `Data::GetStat()`.
5. ➔ `StatEditor` calculates the new actual health value using `Stats::GetBaseValue` and `Stats::GetStepValue`.
6. ➔ `StatEditor` applies the new health to the game engine: `ApplyStatToEngine()`.
7. ➔ `StatEditor` saves the new spent point total: `Data::SetStat()`.

### Action: The player closes the Vanilla Level-Up Menu
1. `LevelUpEventSink` detects `MenuOpenCloseEvent` for `StatsMenu` (closing).
2. ➔ Calls `StatEditor::Harmonize()`.
3. ➔ `StatEditor` calls `Utils::GetActiveFollowers()`.
4. ➔ For each follower, it fetches their profile (`Stats::GetProfileForActor()`).
5. ➔ It forces a recalculation and engine update for all stats based on the points currently saved in `Data`.

### Action: The player clicks "Reload Config (JSON)"
1. `Menu` detects the click.
2. ➔ Calls `Stats::Initialize(".../config.json")`.
3. ➔ `Stats` clears old profiles and calls `ConfigParser::Load()`.
4. ➔ `Menu` calls `StatEditor::Harmonize()` to immediately apply any base stat changes to currently loaded followers.