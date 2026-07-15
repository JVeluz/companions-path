#pragma once

#include "SKSEMenuFramework.h"

namespace UI {
    void Register();
    
    namespace Stats {
        void __stdcall Render();
    }

    namespace Settings {
        void __stdcall Render();
    }
}