#include "profile.h"

#include <algorithm>
#include <cctype>
#include <format>

namespace {
    std::unordered_map<std::string, Profile> TagProfiles;
    std::unordered_map<std::string, Profile> RaceProfiles;
    std::unordered_map<std::string, Profile> ActorProfiles;
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

    Profile ParseProfile(const nlohmann::json& jProfile) {
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

    void LoadProfileCategory(const nlohmann::json& config, const char* categoryName, std::unordered_map<std::string, Profile>& targetMap) {
        if (config.contains(categoryName)) {
            for (auto& [key, data] : config[categoryName].items()) {
                targetMap[ToLowercase(key)] = ParseProfile(data);
            }
        }
    }

    void MergeProfile(Profile& target, const Profile& source) {
        if (source.overrideAttributes) {
            target.Attributes = source.Attributes;
        }
        if (source.overrideSkills) {
            target.Skills = source.Skills;
        }
        if (source.overrideAttributes || source.overrideSkills) {
            target.All = target.Attributes;
            target.All.insert(target.All.end(), target.Skills.begin(), target.Skills.end());
        }
        for (const auto& [av, value] : source.BaseValues) {
            target.BaseValues[av] = value;
        }
    }
}

namespace ProfileRepository {
    
    void InitializeFromJson(const nlohmann::json& config) {
        TagProfiles.clear();
        RaceProfiles.clear();
        ActorProfiles.clear();

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

        LoadProfileCategory(config, "Tags", TagProfiles);
        LoadProfileCategory(config, "Races", RaceProfiles);
        LoadProfileCategory(config, "Actors", ActorProfiles);
    }

    const std::unordered_map<std::string, Profile>& GetTagProfiles() { return TagProfiles; }
    const std::unordered_map<std::string, Profile>& GetRaceProfiles() { return RaceProfiles; }
    const std::unordered_map<std::string, Profile>& GetActorProfiles() { return ActorProfiles; }
    const Profile& GetDefaultProfile() { return DefaultHumanoidProfile; }
}

namespace ProfileParser {

    Profile GetProfile(RE::Actor* actor) {
        if (!actor) return ProfileRepository::GetDefaultProfile();

        Profile finalProfile = ProfileRepository::GetDefaultProfile();

        for (const auto& [tagKey, tagProfile] : ProfileRepository::GetTagProfiles()) {
            if (actor->HasKeywordString(tagKey) || actor->HasKeywordString("actortype" + tagKey)) { 
                MergeProfile(finalProfile, tagProfile);
                break;
            }
        }

        if (auto race = actor->GetRace()) {
            std::string raceName = ToLowercase(race->GetFormEditorID());
            for (const auto& [raceKey, raceProfile] : ProfileRepository::GetRaceProfiles()) {
                if (raceName.find(raceKey) != std::string::npos) {
                    MergeProfile(finalProfile, raceProfile);
                    break;
                }
            }
        }

        if (auto actorBase = actor->GetActorBase()) {
            std::string pluginPlusLocalID = "";
            if (auto file = actorBase->GetFile(0)) {
                uint32_t localID = actorBase->GetFormID() & 0x00FFFFFF;
                pluginPlusLocalID = ToLowercase(std::format("{}|{:x}", file->GetFilename(), localID));
            }

            std::string actorName = ToLowercase(actorBase->GetName());
            const auto& actorProfiles = ProfileRepository::GetActorProfiles();

            if (!pluginPlusLocalID.empty() && actorProfiles.find(pluginPlusLocalID) != actorProfiles.end()) {
                MergeProfile(finalProfile, actorProfiles.at(pluginPlusLocalID));
            }
            else if (actorProfiles.find(actorName) != actorProfiles.end()) {
                MergeProfile(finalProfile, actorProfiles.at(actorName));
            }
        }

        return finalProfile;
    }
}
