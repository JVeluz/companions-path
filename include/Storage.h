#pragma once

#include <vector>

namespace Storage {
    void Register();

    namespace Stats {
        int GetPoints(RE::Actor* actor, RE::ActorValue av);
        void SetPoints(RE::Actor* actor, RE::ActorValue av, int points);
        void Clear(RE::Actor* actor);
    }

    namespace Perks {
        void RecordPurchase(RE::Actor* actor, RE::BGSPerk* perk);
        void RecordRefund(RE::Actor* actor, RE::BGSPerk* perk);
        bool HasPurchased(RE::Actor* actor, RE::BGSPerk* perk);
        std::size_t GetPurchasedCount(RE::Actor* actor);
        std::vector<RE::BGSPerk*> GetPurchased(RE::Actor* actor);
        void Clear(RE::Actor* actor);
    }
    
    void ClearAll();
}