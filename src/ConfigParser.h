#pragma once

#include <string>
#include <unordered_map>

#include "Stats.h"

class ConfigParser {
public:
    static bool Load(const std::string& filepath, 
                     std::unordered_map<std::string, StatProfile>& outTagProfiles,
                     std::unordered_map<std::string, StatProfile>& outRaceProfiles);

private:
    static RE::ActorValue StringToActorValue(const std::string& str);
    static std::string ToLowercase(std::string_view str);
};