#include "ProfileRepository.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "logger.h"
#include "structs.h"

namespace {
    std::string ToLowercase(std::string_view str) {
        std::string lowerStr(str);
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }
}

namespace ProfileRepository {

    namespace fs = std::filesystem;

    namespace {
        Profile profile;

        std::vector<std::string> availableProfiles;

        std::unordered_map<std::string, ActorProfile> tagProfiles;
        std::unordered_map<std::string, ActorProfile> raceProfiles;
        std::unordered_map<std::string, ActorProfile> actorProfiles;
        ActorProfile defaultHumanoidProfile;

        std::string currentLoadedPath;
        nlohmann::json currentProfileJson;

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

        ActorProfile ParseProfile(const nlohmann::json& jProfile) {
            ActorProfile actorProfile;

            if (jProfile.contains("Attributes")) {
                for (const auto& attr : jProfile["Attributes"]) {
                    auto av = StringToActorValue(attr);
                    if (av != RE::ActorValue::kNone) actorProfile.attributes.push_back(av);
                }
                actorProfile.overrideAttributes = true;
            }
            if (jProfile.contains("Skills")) {
                for (const auto& skill : jProfile["Skills"]) {
                    auto av = StringToActorValue(skill);
                    if (av != RE::ActorValue::kNone) actorProfile.skills.push_back(av);
                }
                actorProfile.overrideSkills = true;
            }
            if (jProfile.contains("BaseValues")) {
                for (auto& [key, value] : jProfile["BaseValues"].items()) {
                    auto av = StringToActorValue(key);
                    if (av != RE::ActorValue::kNone) {
                        actorProfile.baseValues[av] = value.get<float>();
                    }
                }
            }

            actorProfile.all = actorProfile.attributes;
            actorProfile.all.insert(actorProfile.all.end(), actorProfile.skills.begin(), actorProfile.skills.end());

            return actorProfile;
        }

        void LoadProfileCategory(const nlohmann::json& config, const char* categoryName, std::unordered_map<std::string, ActorProfile>& targetMap) {
            if (config.contains(categoryName)) {
                for (auto& [key, data] : config[categoryName].items()) {
                    targetMap[ToLowercase(key)] = ParseProfile(data);
                }
            }
        }
    }

    const Profile& GetProfile() { return profile; }
    const ActorProfile& GetDefaultProfile() { return defaultHumanoidProfile; }

    const std::vector<std::string>& GetProfiles() { return availableProfiles; }
    const std::string& GetProfileName(int index) { return availableProfiles[index]; }

    const std::unordered_map<std::string, ActorProfile>& GetTagProfiles() { return tagProfiles; }
    const std::unordered_map<std::string, ActorProfile>& GetRaceProfiles() { return raceProfiles; }
    const std::unordered_map<std::string, ActorProfile>& GetActorProfiles() { return actorProfiles; }

    void UpdateSettings(const Profile& newSettings) {
        profile.harmonizeStats = newSettings.harmonizeStats;
        profile.harmonizePerks = newSettings.harmonizePerks;
        profile.syncLevel = newSettings.syncLevel;
        profile.levelMultiplier = newSettings.levelMultiplier;
        profile.attributeMultiplier = newSettings.attributeMultiplier;
        profile.skillMultiplier = newSettings.skillMultiplier;
        profile.perkMultiplier = newSettings.perkMultiplier;

        currentProfileJson["Settings"]["HarmonizeStats"] = profile.harmonizeStats;
        currentProfileJson["Settings"]["HarmonizePerks"] = profile.harmonizePerks;
        currentProfileJson["Settings"]["SyncLevel"] = profile.syncLevel;
        currentProfileJson["Settings"]["LevelMultiplier"] = profile.levelMultiplier;
        currentProfileJson["Settings"]["AttributeMultiplier"] = profile.attributeMultiplier;
        currentProfileJson["Settings"]["SkillMultiplier"] = profile.skillMultiplier;
        currentProfileJson["Settings"]["PerkMultiplier"] = profile.perkMultiplier;
    }

    void Save() {
        if (currentLoadedPath.empty()) {
            logger::error("Cannot save profile: No profile currently loaded.");
            return;
        }

        std::ofstream file(currentLoadedPath);
        if (file.is_open()) {
            file << currentProfileJson.dump(4);
            logger::info("Profile successfully saved to : {}", currentLoadedPath);
        } else {
            logger::error("Failed to save profile at : {}", currentLoadedPath);
        }
    }

    void Scan(const std::string& path) {
        availableProfiles.clear();
        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".json") {
                        availableProfiles.push_back(entry.path().stem().string());
                    }
                }
            } else {
                logger::error("Directory does not exist: {}", path);
            }
        } catch (const fs::filesystem_error& e) {
            logger::error("Filesystem error while scanning profiles: {}", e.what());
        }
    }

    void Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            logger::error("No profiles files found at : {}", path);
            return;
        }

        currentLoadedPath = path;
        file >> currentProfileJson;

        tagProfiles.clear();
        raceProfiles.clear();
        actorProfiles.clear();
        profile = Profile();

        if (currentProfileJson.contains("Settings")) {
            const auto& gs = currentProfileJson["Settings"];
            if (gs.contains("HarmonizeStats")) profile.harmonizeStats = gs["HarmonizeStats"].get<bool>();
            if (gs.contains("HarmonizePerks")) profile.harmonizePerks = gs["HarmonizePerks"].get<bool>();
            if (gs.contains("SyncLevel")) profile.syncLevel = gs["SyncLevel"].get<bool>();
            if (gs.contains("LevelMultiplier")) profile.levelMultiplier = gs["LevelMultiplier"].get<float>();
            if (gs.contains("AttributeMultiplier")) profile.attributeMultiplier = gs["AttributeMultiplier"].get<float>(); 
            if (gs.contains("SkillMultiplier")) profile.skillMultiplier = gs["SkillMultiplier"].get<float>();
            if (gs.contains("PerkMultiplier")) profile.perkMultiplier = gs["PerkMultiplier"].get<float>();
        }

        defaultHumanoidProfile.attributes = {RE::ActorValue::kHealth, RE::ActorValue::kMagicka, RE::ActorValue::kStamina};
        defaultHumanoidProfile.skills = {RE::ActorValue::kOneHanded,   RE::ActorValue::kTwoHanded,   RE::ActorValue::kBlock,      RE::ActorValue::kHeavyArmor,  RE::ActorValue::kLightArmor, RE::ActorValue::kArchery,
                                         RE::ActorValue::kDestruction, RE::ActorValue::kRestoration, RE::ActorValue::kAlteration, RE::ActorValue::kConjuration, RE::ActorValue::kIllusion,   RE::ActorValue::kSneak,
                                         RE::ActorValue::kLockpicking, RE::ActorValue::kPickpocket,  RE::ActorValue::kSpeech,     RE::ActorValue::kAlchemy,     RE::ActorValue::kSmithing,   RE::ActorValue::kEnchanting};
        defaultHumanoidProfile.all = defaultHumanoidProfile.attributes;
        defaultHumanoidProfile.all.insert(defaultHumanoidProfile.all.end(), defaultHumanoidProfile.skills.begin(), defaultHumanoidProfile.skills.end());

        LoadProfileCategory(currentProfileJson, "Tags", tagProfiles);
        LoadProfileCategory(currentProfileJson, "Races", raceProfiles);
        LoadProfileCategory(currentProfileJson, "Actors", actorProfiles);
    }
}