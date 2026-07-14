#include "ProfileRepository.h"
#include "json.hpp"

#include <fstream>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

namespace {
    std::unordered_map<std::string, Profile> TagProfiles;
    std::unordered_map<std::string, Profile> RaceProfiles;
    Profile DefaultHumanoidProfile;

    std::string ToLowercase(std::string_view str) {
        std::string lowerStr(str);
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return lowerStr;
    }

    RE::ActorValue StringToActorValue(const std::string& str) {
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

    Profile ParseProfile(const json& jProfile) {
        Profile profile;
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
    }
}

namespace ProfileRepository {
    
    void InitializeFromJson(const json& config) {
        TagProfiles.clear();
        RaceProfiles.clear();

        DefaultHumanoidProfile.Attributes = {
            RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina
        };
        DefaultHumanoidProfile.Skills = {
            RE::ActorValue::kOneHanded, RE::ActorValue::kTwoHanded, RE::ActorValue::kBlock,
            RE::ActorValue::kHeavyArmor, RE::ActorValue::kLightArmor, RE::ActorValue::kArchery,
            RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration,
            RE::ActorValue::kConjuration, RE::ActorValue::kIllusion,
            RE::ActorValue::kSneak, RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,
            RE::ActorValue::kSpeech, RE::ActorValue::kAlchemy, RE::ActorValue::kSmithing,
            RE::ActorValue::kEnchanting
        };
        DefaultHumanoidProfile.All = DefaultHumanoidProfile.Attributes;
        DefaultHumanoidProfile.All.insert(
            DefaultHumanoidProfile.All.end(), 
            DefaultHumanoidProfile.Skills.begin(), 
            DefaultHumanoidProfile.Skills.end()
        );

        if (config.contains("Tags")) {
            for (auto& [tag, data] : config["Tags"].items()) {
                TagProfiles[ToLowercase(tag)] = ParseProfile(data);
            }
        }

        if (config.contains("Races")) {
            for (auto& [race, data] : config["Races"].items()) {
                RaceProfiles[ToLowercase(race)] = ParseProfile(data);
            }
        }
    }

    Profile GetProfileForActor(RE::Actor* actor) {
        if (!actor) return DefaultHumanoidProfile;

        Profile finalProfile = DefaultHumanoidProfile;

        for (const auto& [tagKey, tagProfile] : TagProfiles) {
            if (actor->HasKeywordString(tagKey) || actor->HasKeywordString("actortype" + tagKey)) { 
                finalProfile = tagProfile;
                break;
            }
        }

        if (auto race = actor->GetRace()) {
            std::string raceName = ToLowercase(race->GetFormEditorID());
            
            for (const auto& [raceKey, raceProfile] : RaceProfiles) {
                if (raceName.find(raceKey) != std::string::npos) {
                    if (raceProfile.overrideAttributes) {
                        finalProfile.Attributes = raceProfile.Attributes;
                    }
                    if (raceProfile.overrideSkills) {
                        finalProfile.Skills = raceProfile.Skills;
                    }
                    if (raceProfile.overrideAttributes || raceProfile.overrideSkills) {
                        finalProfile.All = finalProfile.Attributes;
                        finalProfile.All.insert(finalProfile.All.end(), finalProfile.Skills.begin(), finalProfile.Skills.end());
                    }
                    for (const auto& [av, value] : raceProfile.BaseValues) {
                        finalProfile.BaseValues[av] = value;
                    }
                    break;
                }
            }
        }

        return finalProfile;
    }
}