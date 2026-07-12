#pragma once

#include <unordered_map>

class Data
{
public:
    static std::unordered_map<RE::FormID, std::unordered_map<RE::ActorValue, int>> CompanionStats;

    static void Register();

    static void SetStat(RE::Actor* actor, RE::ActorValue actorValue, int points);
    static int GetStat(RE::Actor* actor, RE::ActorValue actorValue, int defaultPoints = 0);
    
    static void ClearActor(RE::Actor* actor);

private:
    static constexpr uint32_t kSerializationID = 'CPTH';
    static constexpr uint32_t kRecordType = 'STAT';
    static constexpr uint32_t kSerializationVersion = 2; // Version bump

    static void SaveCallback(SKSE::SerializationInterface* a_intfc);
    static void LoadCallback(SKSE::SerializationInterface* a_intfc);
    static void RevertCallback(SKSE::SerializationInterface* a_intfc);
};