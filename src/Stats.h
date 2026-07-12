#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <span>

struct StatProfile {
    std::vector<RE::ActorValue> Attributes;
    std::vector<RE::ActorValue> Skills;
    std::vector<RE::ActorValue> All;
    std::unordered_map<RE::ActorValue, float> BaseValues;
    
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

class Stats {
public:
    static void Initialize(const std::string& configPath);
    static StatProfile GetProfileForActor(RE::Actor* actor);
    static bool IsAttribute(RE::Actor* actor, RE::ActorValue actorValue);
    static int GetMaxPoints(RE::Actor* actor, RE::ActorValue actorValue);
    static float GetStepValue(RE::Actor* actor, RE::ActorValue actorValue);
    static float GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue);

private:
    static std::unordered_map<std::string, StatProfile> TagProfiles;
    static std::unordered_map<std::string, StatProfile> RaceProfiles;
    static StatProfile DefaultHumanoidProfile;
    static std::string ToLowercase(std::string_view str);
};