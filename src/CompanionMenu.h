#include <vector>

class CompanionMenu
{
public:
    static void Register();
    static void Render();

private:
    static std::vector<RE::Actor*> currentFollowers;
    static int selectedCompanionIndex;
};