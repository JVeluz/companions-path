#pragma once

namespace Rules {

    int GetLevel(RE::Actor* actor);

    namespace Stats {
        bool IsCalculatedStat(RE::ActorValue actorValue);
        bool IsAttribute(RE::Actor* actor, RE::ActorValue actorValue);
        
        int GetMaxValue(RE::Actor* actor, RE::ActorValue actorValue);
        float GetStepValue(RE::Actor* actor, RE::ActorValue actorValue);
        float GetBaseValue(RE::Actor* actor, RE::ActorValue actorValue);
        
        float CalculateStatValue(RE::Actor* actor, RE::ActorValue actorValue, int points);
    
        int GetAttributePoints(RE::Actor* actor);
        int GetSkillPoints(RE::Actor* actor);
    }

    namespace Perks {
        int GetPoints(RE::Actor* actor);
    }

}
