#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <json.hpp>

struct Profile {
    std::vector<RE::ActorValue> Attributes;
    std::vector<RE::ActorValue> Skills;
    std::vector<RE::ActorValue> All;
    std::unordered_map<RE::ActorValue, float> BaseValues;
    bool overrideAttributes = false;
    bool overrideSkills = false;
};

namespace ProfileParser {
    Profile GetProfile(RE::Actor* actor);
}

namespace ProfileRepository {
    void InitializeFromJson(const nlohmann::json& config);
    const std::unordered_map<std::string, Profile>& GetTagProfiles();
    const std::unordered_map<std::string, Profile>& GetRaceProfiles();
    const std::unordered_map<std::string, Profile>& GetActorProfiles();
    const Profile& GetDefaultProfile();
}
