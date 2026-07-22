#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "structs.h"

namespace ProfileRepository {

    const Profile& GetProfile();
    const ActorProfile& GetDefaultProfile();

    const std::vector<std::string>& GetProfiles();
    const std::string& GetProfileName(int index);

    const std::unordered_map<std::string, ActorProfile>& GetTagProfiles();
    const std::unordered_map<std::string, ActorProfile>& GetRaceProfiles();
    const std::unordered_map<std::string, ActorProfile>& GetActorProfiles();

    void UpdateSettings(const Profile& newSettings);

    void Save();
    void Scan(const std::string& path);
    void Load(const std::string& path);
}