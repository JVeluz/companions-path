class StatEditor
{
public:
    static int GetMaxAttributePoints(RE::Actor *actor);
    static int GetMaxSkillPoints(RE::Actor *actor);
    
    static int GetRemainingAttributePoints(RE::Actor *actor);
    static int GetRemainingSkillPoints(RE::Actor *actor);
    
    static bool HasAttributePointsLeft(RE::Actor *actor);
    static bool HasSkillPointsLeft(RE::Actor *actor);
    
    static void AddPoint(RE::Actor *actor, RE::ActorValue actorValue);
    static void RemovePoint(RE::Actor *actor, RE::ActorValue actorValue);
    
    static void ResetAttributes(RE::Actor *actor);
    static void ResetSkills(RE::Actor *actor);
};