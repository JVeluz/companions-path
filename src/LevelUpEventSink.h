#pragma once

#include <vector>

#include "Data.h"
#include "Stats.h"
#include "StatEditor.h"

class LevelUpEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static LevelUpEventSink* GetSingleton();
    static void Register();
    virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;
};