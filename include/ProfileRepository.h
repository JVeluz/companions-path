#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Profile {
    std::vector<RE::ActorValue> Attributes;
    std::vector<RE::ActorValue> Skills;
    std::vector<RE::ActorValue> All;
    std::unordered_map<RE::ActorValue, float> BaseValues;
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

namespace ProfileRepository {
    void Initialize(const std::string& configPath);
    Profile GetProfileForActor(RE::Actor* actor);
}