#include "ProfileManager.h"

#include <algorithm>
#include <cctype>
#include <format>

#include "ProfileRepository.h"

namespace {
    std::unordered_map<RE::FormID, ActorProfile> cache;

    std::string ToLowercase(std::string_view str) {
        std::string lowerStr(str);
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }

    void MergeProfile(ActorProfile& target, const ActorProfile& source) {
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

namespace ProfileManager {

    const Profile& GetProfile() { return ProfileRepository::GetProfile(); }

    const std::vector<std::string>& GetProfiles() { return ProfileRepository::GetProfiles(); }
    const std::string& GetProfileName(int index) { return ProfileRepository::GetProfileName(index); }

    void UpdateSettings(const Profile& newSettings) { ProfileRepository::UpdateSettings(newSettings); }

    void SaveCurrentProfile() { ProfileRepository::Save(); }

    void ClearCache() { cache.clear(); }

    void Load(const std::string& path) {
        ClearCache();
        ProfileRepository::Load(path);
    }

    ActorProfile GetActorProfile(RE::Actor* actor) {
        if (!actor) return ProfileRepository::GetDefaultProfile();

        auto it = cache.find(actor->GetFormID());
        if (it != cache.end()) {
            return it->second;
        }

        ActorProfile actorProfile = ProfileRepository::GetDefaultProfile();

        for (const auto& [tagKey, tagProfile] : ProfileRepository::GetTagProfiles()) {
            if (actor->HasKeywordString(tagKey) || actor->HasKeywordString("actortype" + tagKey)) {
                MergeProfile(actorProfile, tagProfile);
                break;
            }
        }

        if (auto race = actor->GetRace()) {
            std::string raceName = ToLowercase(race->GetFormEditorID());
            for (const auto& [raceKey, raceProfile] : ProfileRepository::GetRaceProfiles()) {
                if (raceName.find(raceKey) != std::string::npos) {
                    MergeProfile(actorProfile, raceProfile);
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
                MergeProfile(actorProfile, actorProfiles.at(pluginPlusLocalID));
            } else if (actorProfiles.find(actorName) != actorProfiles.end()) {
                MergeProfile(actorProfile, actorProfiles.at(actorName));
            }
        }

        cache[actor->GetFormID()] = actorProfile;

        return actorProfile;
    }
}