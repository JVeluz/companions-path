#include "Data.h"

std::unordered_map<RE::FormID, std::unordered_map<RE::ActorValue, int>> Data::CompanionStats;

void Data::Register()
{
    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(kSerializationID);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    serialization->SetRevertCallback(RevertCallback);
}

void Data::SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points)
{
    if (!actor) return;
    CompanionStats[actor->GetFormID()][actorValue] = points;
}

int Data::GetStat(RE::Actor* actor, RE::ActorValue actorValue, int defaultPoints)
{
    if (!actor) return defaultPoints;
    
    auto itActor = CompanionStats.find(actor->GetFormID());
    if (itActor != CompanionStats.end()) {
        auto itStat = itActor->second.find(actorValue);
        if (itStat != itActor->second.end()) {
            return itStat->second;
        }
    }
    return defaultPoints;
}

void Data::ClearActor(RE::Actor* actor)
{
    if (!actor) return;
    CompanionStats.erase(actor->GetFormID());
}

void Data::RevertCallback(SKSE::SerializationInterface* /*a_intfc*/)
{
    CompanionStats.clear();
}

void Data::SaveCallback(SKSE::SerializationInterface* a_intfc)
{
    if (!a_intfc->OpenRecord(kRecordType, kSerializationVersion)) return;

    std::size_t numActors = CompanionStats.size();
    a_intfc->WriteRecordData(numActors);

    for (const auto& [formID, stats] : CompanionStats) {
        a_intfc->WriteRecordData(formID);
        
        std::size_t numStats = stats.size();
        a_intfc->WriteRecordData(numStats);
        
        for (const auto& [av, points] : stats) {
            a_intfc->WriteRecordData(av);
            a_intfc->WriteRecordData(points);
        }
    }
}

void Data::LoadCallback(SKSE::SerializationInterface* a_intfc)
{
    uint32_t type;
    uint32_t version;
    uint32_t length;

    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type != kRecordType) continue;

        std::size_t numActors;
        a_intfc->ReadRecordData(numActors);

        for (std::size_t i = 0; i < numActors; ++i) {
            RE::FormID oldFormID;
            a_intfc->ReadRecordData(oldFormID);

            std::size_t numStats;
            a_intfc->ReadRecordData(numStats);

            std::unordered_map<RE::ActorValue, int> tempStats;
            for (std::size_t j = 0; j < numStats; ++j) {
                RE::ActorValue av;
                int points;
                a_intfc->ReadRecordData(av);
                a_intfc->ReadRecordData(points);
                tempStats[av] = points;
            }

            RE::FormID newFormID;
            if (a_intfc->ResolveFormID(oldFormID, newFormID)) {
                CompanionStats[newFormID] = tempStats;
            }
        }
    }
}