#pragma once

#include <vector>

#include "Utils.h"
#include "SKSEMenuFramework.h"
#include "StatEditor.h"
#include "Stats.h"

class Menu
{
public:
    static void Register();
    static void Render();

private:
    static std::vector<RE::ActorHandle> currentFollowers;
    static int selectedCompanionIndex;
};