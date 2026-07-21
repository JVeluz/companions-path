#include "EventManager.h"

#include "SKSEMenuFramework.h"
#include "StatManager.h"

namespace {

    class MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static MenuOpenCloseEventSink* GetSingleton() {
            static MenuOpenCloseEventSink singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
            if (a_event && a_event->menuName == RE::StatsMenu::MENU_NAME && !a_event->opening) {
                StatManager::Harmonize();
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

}

namespace EventManager {

    void Register() {
        if (auto ui = RE::UI::GetSingleton()) {
            ui->AddEventSink(MenuOpenCloseEventSink::GetSingleton());
        }
    }

}