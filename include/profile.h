#pragma once

#include "structs.h"

#include <string>
#include <unordered_map>

namespace ProfileParser {
    void ClearCache();
    Profile GetProfile(RE::Actor* actor);
}

namespace ProfileRepository {
    void Load(const std::string& path);
    const std::unordered_map<std::string, Profile>& GetTagProfiles();
    const std::unordered_map<std::string, Profile>& GetRaceProfiles();
    const std::unordered_map<std::string, Profile>& GetActorProfiles();
    const Profile& GetDefaultProfile();
}