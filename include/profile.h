#pragma once

#include "structs.h"

#include <string>
#include <unordered_map>
#include <json.hpp>

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
