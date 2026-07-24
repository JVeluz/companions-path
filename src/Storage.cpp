#include "Storage.h"
#include "logger.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace {
    const uint32_t SERIALIZATION_ID = 'CMPN';
    const uint32_t CURRENT_VERSION = 1;

    struct CompanionData {
        std::unordered_map<RE::ActorValue, int> spentStatPoints;
        std::vector<RE::FormID> purchasedPerkFormIDs; 
    };

    std::unordered_map<RE::FormID, CompanionData> storage;

    void OnRevert(SKSE::SerializationInterface*) {
        storage.clear();
        // logger::info("Storage cleared on Revert.");
    }

    void OnSave(SKSE::SerializationInterface* serde) {
        if (!serde->OpenRecord(SERIALIZATION_ID, CURRENT_VERSION)) {
            logger::error("Failed to open record for saving!");
            return;
        }

        std::size_t actorCount = storage.size();
        serde->WriteRecordData(&actorCount, sizeof(actorCount));

        for (const auto& [actorFormID, data] : storage) {
            serde->WriteRecordData(&actorFormID, sizeof(actorFormID));

            std::size_t statCount = data.spentStatPoints.size();
            serde->WriteRecordData(&statCount, sizeof(statCount));
            for (const auto& [av, points] : data.spentStatPoints) {
                serde->WriteRecordData(&av, sizeof(av));
                serde->WriteRecordData(&points, sizeof(points));
            }

            std::size_t perkCount = data.purchasedPerkFormIDs.size();
            serde->WriteRecordData(&perkCount, sizeof(perkCount));
            for (const auto& perkFormID : data.purchasedPerkFormIDs) {
                serde->WriteRecordData(&perkFormID, sizeof(perkFormID));
            }
        }
        // logger::info("Storage saved successfully.");
    }

    void OnLoad(SKSE::SerializationInterface* serde) {
        uint32_t type, version, length;
        while (serde->GetNextRecordInfo(type, version, length)) {
            if (type != SERIALIZATION_ID) continue;

            if (version != CURRENT_VERSION) {
                logger::warn("Version mismatch. Expected {}, got {}", CURRENT_VERSION, version);
                continue;
            }

            std::size_t actorCount;
            serde->ReadRecordData(&actorCount, sizeof(actorCount));

            for (std::size_t i = 0; i < actorCount; ++i) {
                RE::FormID actorFormID;
                serde->ReadRecordData(&actorFormID, sizeof(actorFormID));

                RE::FormID newActorFormID;
                if (!serde->ResolveFormID(actorFormID, newActorFormID)) continue; 

                CompanionData data;

                std::size_t statCount;
                serde->ReadRecordData(&statCount, sizeof(statCount));
                for (std::size_t j = 0; j < statCount; ++j) {
                    RE::ActorValue av;
                    int points;
                    serde->ReadRecordData(&av, sizeof(av));
                    serde->ReadRecordData(&points, sizeof(points));
                    data.spentStatPoints[av] = points;
                }

                std::size_t perkCount;
                serde->ReadRecordData(&perkCount, sizeof(perkCount));
                for (std::size_t j = 0; j < perkCount; ++j) {
                    RE::FormID perkFormID;
                    serde->ReadRecordData(&perkFormID, sizeof(perkFormID));

                    RE::FormID newPerkFormID;
                    if (serde->ResolveFormID(perkFormID, newPerkFormID)) {
                        data.purchasedPerkFormIDs.push_back(newPerkFormID);
                    }
                }

                storage[newActorFormID] = data;
            }
        }
        // logger::info("Storage loaded successfully.");
    }
}

namespace Storage {
    void Register() {
        auto serialization = SKSE::GetSerializationInterface();
        serialization->SetUniqueID(SERIALIZATION_ID);
        serialization->SetSaveCallback(OnSave);
        serialization->SetRevertCallback(OnRevert);
        serialization->SetLoadCallback(OnLoad);
        // logger::info("Storage serialization callbacks registered.");
    }

    void ClearAll() {
        storage.clear();
    }

    namespace Stats {
        int GetPoints(RE::Actor* actor, RE::ActorValue av) {
            if (!actor) return 0;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                auto statIt = it->second.spentStatPoints.find(av);
                if (statIt != it->second.spentStatPoints.end()) {
                    return statIt->second;
                }
            }
            return 0;
        }

        void SetPoints(RE::Actor* actor, RE::ActorValue av, int points) {
            if (!actor) return;
            storage[actor->GetFormID()].spentStatPoints[av] = points;
        }

        void Clear(RE::Actor* actor) {
            if (!actor) return;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                it->second.spentStatPoints.clear();
            }
        }
    }

    namespace Perks {
        void RecordPurchase(RE::Actor* actor, RE::BGSPerk* perk) {
            if (!actor || !perk) return;
            auto& perks = storage[actor->GetFormID()].purchasedPerkFormIDs;
            RE::FormID formID = perk->GetFormID();
            
            if (std::find(perks.begin(), perks.end(), formID) == perks.end()) {
                perks.push_back(formID);
            }
        }

        void RecordRefund(RE::Actor* actor, RE::BGSPerk* perk) {
            if (!actor || !perk) return;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                auto& perks = it->second.purchasedPerkFormIDs;
                RE::FormID formID = perk->GetFormID();
                perks.erase(std::remove(perks.begin(), perks.end(), formID), perks.end());
            }
        }

        bool HasPurchased(RE::Actor* actor, RE::BGSPerk* perk) {
            if (!actor || !perk) return false;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                const auto& perks = it->second.purchasedPerkFormIDs;
                return std::find(perks.begin(), perks.end(), perk->GetFormID()) != perks.end();
            }
            return false;
        }

        std::size_t GetPurchasedCount(RE::Actor* actor) {
            if (!actor) return 0;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                return it->second.purchasedPerkFormIDs.size();
            }
            return 0;
        }

        std::vector<RE::BGSPerk*> GetPurchased(RE::Actor* actor) {
            std::vector<RE::BGSPerk*> result;
            if (!actor) return result;

            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                for (RE::FormID formID : it->second.purchasedPerkFormIDs) {
                    auto form = RE::TESForm::LookupByID(formID);
                    if (form && form->Is(RE::FormType::Perk)) {
                        result.push_back(form->As<RE::BGSPerk>());
                    }
                }
            }
            return result;
        }

        void Clear(RE::Actor* actor) {
            if (!actor) return;
            auto it = storage.find(actor->GetFormID());
            if (it != storage.end()) {
                it->second.purchasedPerkFormIDs.clear();
            }
        }
    }
}