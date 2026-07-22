#pragma once

#include <string>
#include <vector>

#include "structs.h"

namespace ProfileManager {

    const Profile& GetProfile();

    const std::vector<std::string>& GetProfiles();
    const std::string& GetProfileName(int index);

    ActorProfile GetActorProfile(RE::Actor* actor);

    void UpdateSettings(const Profile& newSettings);

    void SaveCurrentProfile();

    void ClearCache();
    void Load(const std::string& path);
}