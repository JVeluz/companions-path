#include "LevelUpEventSink.h"

LevelUpEventSink* LevelUpEventSink::GetSingleton()
{
    static LevelUpEventSink singleton;
    return &singleton;
}

void LevelUpEventSink::Register()
{
    auto ui = RE::UI::GetSingleton();
    if (ui) {
        ui->AddEventSink(GetSingleton());
    }
}

RE::BSEventNotifyControl LevelUpEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* /*a_eventSource*/)
{
    if (a_event && a_event->menuName == RE::StatsMenu::MENU_NAME && !a_event->opening) {
        StatEditor::Harmonize();
    }
    return RE::BSEventNotifyControl::kContinue;
}