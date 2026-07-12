#include <fstream>
#include <algorithm>
#include <cctype>

#include "ConfigParser.h"
#include "json.hpp"

using json = nlohmann::json;

std::string ConfigParser::ToLowercase(std::string_view str) {
    std::string lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

RE::ActorValue ConfigParser::StringToActorValue(const std::string& str) {
    static const std::unordered_map<std::string, RE::ActorValue> map = {
        {"kHealth", RE::ActorValue::kHealth},
        {"kMagicka", RE::ActorValue::kMagicka},
        {"kStamina", RE::ActorValue::kStamina},
        {"kUnarmedDamage", RE::ActorValue::kUnarmedDamage},
        {"kDamageResist", RE::ActorValue::kDamageResist},
        {"kOneHanded", RE::ActorValue::kOneHanded},
        {"kTwoHanded", RE::ActorValue::kTwoHanded},
        {"kBlock", RE::ActorValue::kBlock},
        {"kHeavyArmor", RE::ActorValue::kHeavyArmor},
        {"kLightArmor", RE::ActorValue::kLightArmor},
        {"kArchery", RE::ActorValue::kArchery},
        {"kDestruction", RE::ActorValue::kDestruction},
        {"kRestoration", RE::ActorValue::kRestoration},
        {"kAlteration", RE::ActorValue::kAlteration},
        {"kConjuration", RE::ActorValue::kConjuration},
        {"kIllusion", RE::ActorValue::kIllusion},
        {"kSneak", RE::ActorValue::kSneak},
        {"kLockpicking", RE::ActorValue::kLockpicking},
        {"kPickpocket", RE::ActorValue::kPickpocket},
        {"kSpeech", RE::ActorValue::kSpeech},
        {"kAlchemy", RE::ActorValue::kAlchemy},
        {"kSmithing", RE::ActorValue::kSmithing},
        {"kEnchanting", RE::ActorValue::kEnchanting}
    };
    
    auto it = map.find(str);
    if (it != map.end()) return it->second;
    return RE::ActorValue::kNone; 
}

bool ConfigParser::Load(const std::string& filepath, 
                        std::unordered_map<std::string, StatProfile>& outTagProfiles,
                        std::unordered_map<std::string, StatProfile>& outRaceProfiles) {
    
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    json config;
    try {
        file >> config;
    } catch (const json::parse_error& e) {
        return false;
    }

    auto ParseProfile = [](const json& jProfile) -> StatProfile {
        StatProfile profile;
        if (jProfile.contains("Attributes")) {
            for (const auto& attr : jProfile["Attributes"]) 
                profile.Attributes.push_back(StringToActorValue(attr));
            profile.overrideAttributes = true;
        }
        if (jProfile.contains("Skills")) {
            for (const auto& skill : jProfile["Skills"]) 
                profile.Skills.push_back(StringToActorValue(skill));
            profile.overrideSkills = true;
        }
        if (jProfile.contains("BaseValues")) {
            for (auto& [key, value] : jProfile["BaseValues"].items()) {
                profile.BaseValues[StringToActorValue(key)] = value.get<float>();
            }
        }
        
        profile.All = profile.Attributes;
        profile.All.insert(profile.All.end(), profile.Skills.begin(), profile.Skills.end());
        
        return profile;
    };

    if (config.contains("Tags")) {
        for (auto& [tag, data] : config["Tags"].items()) {
            outTagProfiles[ToLowercase(tag)] = ParseProfile(data);
        }
    }

    if (config.contains("Races")) {
        for (auto& [race, data] : config["Races"].items()) {
            outRaceProfiles[ToLowercase(race)] = ParseProfile(data);
        }
    }

    return true;
}