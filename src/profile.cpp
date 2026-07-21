#include "profile.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <json.hpp>

namespace {
    std::string ToLowercase(std::string_view str) {
        std::string lowerStr(str);
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }
}

namespace ProfileRepository {

    namespace {
        std::unordered_map<std::string, Profile> tagProfiles;
        std::unordered_map<std::string, Profile> raceProfiles;
        std::unordered_map<std::string, Profile> actorProfiles;
        Profile defaultHumanoidProfile;

        RE::ActorValue StringToActorValue(const std::string& str) {
            static const std::unordered_map<std::string, RE::ActorValue> map = {{"Health", RE::ActorValue::kHealth},
                                                                                {"Magicka", RE::ActorValue::kMagicka},
                                                                                {"Stamina", RE::ActorValue::kStamina},
                                                                                {"UnarmedDamage", RE::ActorValue::kUnarmedDamage},
                                                                                {"DamageResist", RE::ActorValue::kDamageResist},
                                                                                {"OneHanded", RE::ActorValue::kOneHanded},
                                                                                {"TwoHanded", RE::ActorValue::kTwoHanded},
                                                                                {"Block", RE::ActorValue::kBlock},
                                                                                {"HeavyArmor", RE::ActorValue::kHeavyArmor},
                                                                                {"LightArmor", RE::ActorValue::kLightArmor},
                                                                                {"Archery", RE::ActorValue::kArchery},
                                                                                {"Destruction", RE::ActorValue::kDestruction},
                                                                                {"Restoration", RE::ActorValue::kRestoration},
                                                                                {"Alteration", RE::ActorValue::kAlteration},
                                                                                {"Conjuration", RE::ActorValue::kConjuration},
                                                                                {"Illusion", RE::ActorValue::kIllusion},
                                                                                {"Sneak", RE::ActorValue::kSneak},
                                                                                {"Lockpicking", RE::ActorValue::kLockpicking},
                                                                                {"Pickpocket", RE::ActorValue::kPickpocket},
                                                                                {"Speech", RE::ActorValue::kSpeech},
                                                                                {"Alchemy", RE::ActorValue::kAlchemy},
                                                                                {"Smithing", RE::ActorValue::kSmithing},
                                                                                {"Enchanting", RE::ActorValue::kEnchanting}};

            auto it = map.find(str);
            if (it != map.end()) return it->second;
            return RE::ActorValue::kNone;
        }

        Profile ParseProfile(const nlohmann::json& jProfile) {
            Profile profile;

            if (jProfile.contains("Attributes")) {
                for (const auto& attr : jProfile["Attributes"]) {
                    auto av = StringToActorValue(attr);
                    if (av != RE::ActorValue::kNone) profile.attributes.push_back(av);
                }
                profile.overrideAttributes = true;
            }
            if (jProfile.contains("Skills")) {
                for (const auto& skill : jProfile["Skills"]) {
                    auto av = StringToActorValue(skill);
                    if (av != RE::ActorValue::kNone) profile.skills.push_back(av);
                }
                profile.overrideSkills = true;
            }
            if (jProfile.contains("BaseValues")) {
                for (auto& [key, value] : jProfile["BaseValues"].items()) {
                    auto av = StringToActorValue(key);
                    if (av != RE::ActorValue::kNone) {
                        profile.baseValues[av] = value.get<float>();
                    }
                }
            }

            profile.all = profile.attributes;
            profile.all.insert(profile.all.end(), profile.skills.begin(), profile.skills.end());

            return profile;
        }

        void LoadProfileCategory(const nlohmann::json& config, const char* categoryName, std::unordered_map<std::string, Profile>& targetMap) {
            if (config.contains(categoryName)) {
                for (auto& [key, data] : config[categoryName].items()) {
                    targetMap[ToLowercase(key)] = ParseProfile(data);
                }
            }
        }

    }

    void Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            logger::error("No profiles files found at : {}", path);
            return;
        }

        nlohmann::json profilesFile;
        file >> profilesFile;

        tagProfiles.clear();
        raceProfiles.clear();
        actorProfiles.clear();

        defaultHumanoidProfile.attributes = {RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina};
        defaultHumanoidProfile.skills = {RE::ActorValue::kOneHanded,   RE::ActorValue::kTwoHanded,   RE::ActorValue::kBlock,      RE::ActorValue::kHeavyArmor,  RE::ActorValue::kLightArmor, RE::ActorValue::kArchery,
                                         RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration, RE::ActorValue::kConjuration, RE::ActorValue::kIllusion,   RE::ActorValue::kSneak,
                                         RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,  RE::ActorValue::kSpeech,     RE::ActorValue::kAlchemy,     RE::ActorValue::kSmithing,   RE::ActorValue::kEnchanting};
        defaultHumanoidProfile.all = defaultHumanoidProfile.attributes;
        defaultHumanoidProfile.all.insert(defaultHumanoidProfile.all.end(), defaultHumanoidProfile.skills.begin(), defaultHumanoidProfile.skills.end());

        LoadProfileCategory(profilesFile, "Tags", tagProfiles);
        LoadProfileCategory(profilesFile, "Races", raceProfiles);
        LoadProfileCategory(profilesFile, "Actors", actorProfiles);
    }

    const std::unordered_map<std::string, Profile>& GetTagProfiles() { return tagProfiles; }
    const std::unordered_map<std::string, Profile>& GetRaceProfiles() { return raceProfiles; }
    const std::unordered_map<std::string, Profile>& GetActorProfiles() { return actorProfiles; }
    const Profile& GetDefaultProfile() { return defaultHumanoidProfile; }
}

namespace ProfileParser {

    namespace {
        std::unordered_map<RE::FormID, Profile> profileCache;

        void MergeProfile(Profile& target, const Profile& source) {
            if (source.overrideAttributes) {
                target.attributes = source.attributes;
            }
            if (source.overrideSkills) {
                target.skills = source.skills;
            }
            if (source.overrideAttributes || source.overrideSkills) {
                target.all = target.attributes;
                target.all.insert(target.all.end(), target.skills.begin(), target.skills.end());
            }
            for (const auto& [av, value] : source.baseValues) {
                target.baseValues[av] = value;
            }
        }
    }

    void ClearCache() {
        profileCache.clear();
    }

    Profile GetProfile(RE::Actor* actor) {
        if (!actor) return ProfileRepository::GetDefaultProfile();

        auto it = profileCache.find(actor->GetFormID());
        if (it != profileCache.end()) {
            return it->second;
        }

        Profile profile = ProfileRepository::GetDefaultProfile();

        for (const auto& [tagKey, tagProfile] : ProfileRepository::GetTagProfiles()) {
            if (actor->HasKeywordString(tagKey) || actor->HasKeywordString("actortype" + tagKey)) {
                MergeProfile(profile, tagProfile);
                break;
            }
        }

        if (auto race = actor->GetRace()) {
            std::string raceName = ToLowercase(race->GetFormEditorID());
            for (const auto& [raceKey, raceProfile] : ProfileRepository::GetRaceProfiles()) {
                if (raceName.find(raceKey) != std::string::npos) {
                    MergeProfile(profile, raceProfile);
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
                MergeProfile(profile, actorProfiles.at(pluginPlusLocalID));
            } else if (actorProfiles.find(actorName) != actorProfiles.end()) {
                MergeProfile(profile, actorProfiles.at(actorName));
            }
        }

        return profile;
    }
}