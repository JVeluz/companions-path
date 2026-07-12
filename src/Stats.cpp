#include "Stats.h"
#include "ConfigParser.h"
#include <algorithm>
#include <cctype>

std::unordered_map<std::string, StatProfile> Stats::TagProfiles;
std::unordered_map<std::string, StatProfile> Stats::RaceProfiles;
StatProfile Stats::DefaultHumanoidProfile;

std::string Stats::ToLowercase(std::string_view str) {
    std::string lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

void Stats::Initialize(const std::string& configPath) {
    TagProfiles.clear();
    RaceProfiles.clear();

    ConfigParser::Load(configPath, TagProfiles, RaceProfiles);

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
}

StatProfile Stats::GetProfileForActor(RE::Actor* actor) {
    if (!actor) return DefaultHumanoidProfile;

    StatProfile finalProfile = DefaultHumanoidProfile;

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

bool Stats::IsAttribute(RE::Actor* actor, RE::ActorValue actorValue) {
    StatProfile profile = GetProfileForActor(actor);
    return std::find(profile.Attributes.begin(), profile.Attributes.end(), actorValue) != profile.Attributes.end();
}

int Stats::GetMaxPoints(RE::Actor* actor, RE::ActorValue actorValue) {
    if (IsAttribute(actor, actorValue))
        return -1;
    return 100;
}

float Stats::GetStepValue(RE::Actor* actor, RE::ActorValue actorValue) {
    if (IsAttribute(actor, actorValue))
        return 10.f;
    return 1.f;
}

float Stats::GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue) {
    StatProfile profile = GetProfileForActor(actor);
    
    auto it = profile.BaseValues.find(actorValue);
    if (it != profile.BaseValues.end()) {
        return it->second;
    }

    return IsAttribute(actor, actorValue) ? 100.f : 15.f;
}