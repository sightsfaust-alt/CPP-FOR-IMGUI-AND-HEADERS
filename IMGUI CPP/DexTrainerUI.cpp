#include "DexTrainerUI.h"
#include "Trainer.h"
#include "MenuSZK.h"
#include "Window.h"
#include "WindowItemOptions.h"
#include "Container.h"

// allWindows is a global defined in MenuSZK.cpp
extern std::vector<Window*> allWindows;

// Static member definitions
IWindow* DexTrainerUI::mainWindow = nullptr;
IWindow* DexTrainerUI::playerWindow = nullptr;
IWindow* DexTrainerUI::bodyWindow = nullptr;
IWindow* DexTrainerUI::teleportWindow = nullptr;
IWindow* DexTrainerUI::vehicleWindow = nullptr;
IWindow* DexTrainerUI::weaponsWindow = nullptr;
IWindow* DexTrainerUI::worldWindow = nullptr; // Phase 2
IWindow* DexTrainerUI::clothesWindow = nullptr;
IWindow* DexTrainerUI::clothesStoreWindow = nullptr;
IWindow* DexTrainerUI::clothesCategoryWindow = nullptr;
IWindow* DexTrainerUI::vehicleCategoryWindow = nullptr;

// Teleport locations (mirrors Trainer.cpp kLocs, hardcoded English labels)
struct DexLoc { const char* name; float x, y, z; };
static const DexLoc kDexLocs[] = {
    {"Area 69",         200.0f,  1900.0f, 18.0f},
    {"Grove Street",   2490.0f, -1680.0f, 13.0f},
    {"LS Airport",     1950.0f, -2200.0f, 13.0f},
    {"LV Airport",     1300.0f,  1250.0f, 13.0f},
    {"Mt. Chiliad",   -2300.0f, -1600.0f, 500.0f},
    {"SF Airport",    -1500.0f,  -300.0f, 13.0f},
};

// Weapon list (mirrors Trainer.cpp kWeapons) — DexWeapon defined in header.
static const DexWeapon kDexWeapons[] = {
    {4, "Knife"}, {5, "Bat"}, {8, "Katana"}, {9, "Chainsaw"},
    {22, "Pistol"}, {23, "Silenced"}, {24, "Deagle"},
    {25, "Shotgun"}, {26, "Sawn-off"}, {27, "Combat SG"},
    {28, "Micro Uzi"}, {29, "MP5"}, {30, "AK-47"}, {31, "M4"},
    {33, "Rifle"}, {34, "Sniper"}, {35, "RPG"}, {36, "HS Rocket"},
    {37, "Flamethrower"}, {38, "Minigun"}, {16, "Grenade"}, {18, "Molotov"},
};

// TASK3: weapons grouped by category (no more endless scrolling).
struct DexWeaponCat { const char* title; const DexWeapon* weapons; int count; };
static const DexWeapon kCatMelee[]    = {{5,"Bat"},{9,"Chainsaw"},{8,"Katana"},{4,"Knife"}};
static const DexWeapon kCatPistols[]  = {{24,"Deagle"},{22,"Pistol"},{23,"Silenced"}};
static const DexWeapon kCatShotguns[] = {{27,"Combat SG"},{26,"Sawn-off"},{25,"Shotgun"}};
static const DexWeapon kCatSMGs[]     = {{29,"MP5"},{28,"Micro Uzi"}};
static const DexWeapon kCatAssault[]  = {{30,"AK-47"},{31,"M4"}};
static const DexWeapon kCatRifles[]   = {{33,"Rifle"},{34,"Sniper"}};
static const DexWeapon kCatHeavy[]    = {{37,"Flamethrower"},{36,"HS Rocket"},{38,"Minigun"},{35,"RPG"}};
static const DexWeapon kCatThrown[]   = {{16,"Grenade"},{18,"Molotov"}};
static const DexWeaponCat kDexWeaponCats[] = {
    {"Melee",          kCatMelee,    4},
    {"Pistols",        kCatPistols,  3},
    {"Shotguns",       kCatShotguns, 3},
    {"SMGs",           kCatSMGs,     2},
    {"Assault Rifles", kCatAssault,  2},
    {"Rifles",         kCatRifles,   2},
    {"Heavy",          kCatHeavy,    4},
    {"Thrown",         kCatThrown,   2},
};


void DexTrainerUI::OnWindowClosed(IWindow** slot)
{
    // Called when user closes a category window via X button.
    // The Window::ImGuiRender handles Close() internally; we just null the slot
    // next time EnsureMainWindow runs (checked via allWindows scan).
    (void)slot;
}

void DexTrainerUI::EnsureMainWindow()
{
    // Don't create duplicates: scan allWindows for existing DexTrainer main.
    {
        std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
        for (auto w : allWindows)
        {
            if (w->title == "DexTrainer v1.0")
                return; // already open
        }
    }

    auto win = menuSZK->CreateWindow(100, 100, 520, "DexTrainer v1.0");
    mainWindow = win;

    // TASK2: title-bar X button -> close everything (same as Close button).
    win->onXButton = []() { DexTrainerUI::CloseAll(); };

    // Category buttons (alphabetical order — keep sorted when adding new!)
    {
        auto btn = win->AddButton("Clothes");
        btn->onClick->Add([]() { DexTrainerUI::OpenClothesWindow(); });
    }
    {
        auto btn = win->AddButton("Player");
        btn->onClick->Add([]() { DexTrainerUI::OpenPlayerWindow(); });
    }
    {
        auto btn = win->AddButton("Teleport");
        btn->onClick->Add([]() { DexTrainerUI::OpenTeleportWindow(); });
    }
    {
        auto btn = win->AddButton("Vehicle");
        btn->onClick->Add([]() { DexTrainerUI::OpenVehicleWindow(); });
    }
    {
        auto btn = win->AddButton("Weapons");
        btn->onClick->Add([]() { DexTrainerUI::OpenWeaponsWindow(); });
    }
    {
        auto btn = win->AddButton("World"); // Phase 2: weather, time
        btn->onClick->Add([]() { DexTrainerUI::OpenWorldWindow(); });
    }
    {
        auto btn = win->AddButton("Close");
        btn->onClick->Add([]() { DexTrainerUI::CloseAll(); });
    }
}

static IWindow* FindWindowByTitle(const std::string& title)
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    for (auto w : allWindows)
    {
        if (w->title == title)
            return w;
    }
    return nullptr;
}

// BUG1 FIX: Show-only-one-window pattern. Close all DexTrainer windows
// before opening a new one — prevents the overlapping mess.
static void CloseAllDexWindows()
{
    const char* titles[] = {
        "DexTrainer v1.0",
        "Player",
        "Body",
        "Teleport",
        "Vehicle",
        "Weapons",
        "World",
        "Clothes",
        "Binco", "ProLaps", "SubUrban", "Zip", "Victim", "Didier Sachs",
        "Melee", "Pistols", "Shotguns", "SMGs",
        "Assault Rifles", "Rifles", "Heavy", "Thrown",
        "Hats", "Torso", "Legs", "Shoes", "Glasses", "Watches", "Necklaces",
        "Cars", "Bikes", "Boats", "Helis", "Planes", "Others",
    };
    std::vector<IWindow*> toClose;
    {
        std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
        for (auto w : allWindows)
        {
            for (auto t : titles)
            {
                if (w->title == t) { toClose.push_back(w); break; }
            }
        }
    }
    for (auto w : toClose)
        w->Close();
}

// Window spawn position: centered-ish, consistent. Avoids all windows
// spawning at the same spot and overlapping.
static void PlaceWindow(IWindow* win)
{
    // Window API positions via CreateWindow x,y — we pass good coords at creation.
    (void)win;
}

void DexTrainerUI::OpenPlayerWindow()
{
    // BUG1 FIX: close everything else first — one window at a time.
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Player");
    playerWindow = win;

    // TASK2: title-bar X -> back to main (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    // Player menu — alphabetical order (keep sorted when adding new!)
    {
        auto btn = win->AddButton("Armor Full");
        btn->onClick->Add([]() { Trainer::SetArmour(100.0f); });
    }
    // Body stats — grouped into sub-menu (CCheat::*)
    {
        auto btn = win->AddButton("Body");
        btn->onClick->Add([]() { DexTrainerUI::OpenBodyWindow(); });
    }
    {
        auto btn = win->AddButton("Clear Wanted");
        btn->onClick->Add([]() { Trainer::SetWantedLevel(0); });
    }
    win->AddCheckbox("God Mode", &Trainer::godMode);
    {
        auto btn = win->AddButton("Heal Full");
        btn->onClick->Add([]() { Trainer::SetHealth(100.0f); });
    }
    {
        auto btn = win->AddButton("Jetpack");
        btn->onClick->Add([]() { Trainer::CheatJetpack(); });
    }
    win->AddCheckbox("Never Wanted", &Trainer::neverWanted);
    {
        auto btn = win->AddButton("Suicide");
        btn->onClick->Add([]() { Trainer::CheatSuicide(); });
    }
    {
        auto btn = win->AddButton("+$100,000");
        btn->onClick->Add([]() { Trainer::AddMoney(100000); });
    }
    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

// Body stats sub-menu (grouped from Player — CCheat::*).
void DexTrainerUI::OpenBodyWindow()
{
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Body");
    bodyWindow = win;

    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::OpenPlayerWindow();
    };

    {
        auto btn = win->AddButton("Fat");
        btn->onClick->Add([]() { Trainer::CheatBody(0); });
    }
    {
        auto btn = win->AddButton("Muscle");
        btn->onClick->Add([]() { Trainer::CheatBody(1); });
    }
    {
        auto btn = win->AddButton("Skinny");
        btn->onClick->Add([]() { Trainer::CheatBody(2); });
    }
    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::OpenPlayerWindow();
        });
    }
}

void DexTrainerUI::OpenTeleportWindow()
{
    // BUG1 FIX: close everything else first — one window at a time.
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Teleport");
    teleportWindow = win;

    // TASK2: title-bar X -> back to main (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    // To Marker FIRST (user priority 2026-10-15), then locations alphabetical.
    {
        auto btn = win->AddButton("To Marker (Waypoint)");
        btn->onClick->Add([]() { Trainer::TeleportToMarker(); });
    }
    // 6 locations (alphabetical)
    for (auto& loc : kDexLocs)
    {
        auto btn = win->AddButton(loc.name);
        float x = loc.x, y = loc.y, z = loc.z;
        btn->onClick->Add([x, y, z]() { Trainer::TeleportPlayer(x, y, z); });
    }
    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

void DexTrainerUI::OpenVehicleWindow()
{
    // BUG1 FIX: close everything else first — one window at a time.
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Vehicle");
    vehicleWindow = win;

    // TASK2: title-bar X -> back to main (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    {
        auto btn = win->AddButton("Fix Vehicle");
        btn->onClick->Add([]() { Trainer::FixPlayerVehicle(); });
    }
    {
        auto btn = win->AddButton("Flip Vehicle");
        btn->onClick->Add([]() { Trainer::FlipPlayerVehicle(); });
    }
    {
        auto btn = win->AddButton("Blow Up Cars");
        btn->onClick->Add([]() { Trainer::CheatBlowUpCars(); });
    }

    // Vehicle spawn via script VM (CheatMenu method: Command<CREATE_CAR>)
    {
        win->AddCheckbox("Spawn as Driver", &Trainer::spawnAsDriver);
    }
    for (int i = 0; i < Trainer::GetVehicleCatCount(); i++)
    {
        const char* name = Trainer::GetVehicleCatName(i);
        auto btn = win->AddButton(name ? name : "?");
        int cat = i;
        btn->onClick->Add([cat]() { DexTrainerUI::OpenVehicleCategory(cat); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

// Vehicle category window — list vehicles, click to spawn in front of player.
void DexTrainerUI::OpenVehicleCategory(int cat)
{
    CloseAllDexWindows();

    const char* catName = Trainer::GetVehicleCatName(cat);
    auto win = menuSZK->CreateWindow(140, 120, 480, catName ? catName : "Vehicles");
    vehicleCategoryWindow = win;

    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::OpenVehicleWindow();
    };

    int count = Trainer::GetVehicleItemCount(cat);
    for (int i = 0; i < count; i++)
    {
        const char* display = nullptr;
        int modelId = 0;
        Trainer::GetVehicleItem(cat, i, &display, &modelId);
        auto btn = win->AddButton(display ? display : "?");
        btn->onClick->Add([modelId]() { Trainer::SpawnVehicle(modelId); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::OpenVehicleWindow();
        });
    }
}

void DexTrainerUI::OpenWeaponsWindow()
{
    // BUG1 FIX: close everything else first — one window at a time.
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Weapons");
    weaponsWindow = win;

    // TASK2: title-bar X -> back to main (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    {
        auto btn = win->AddButton("Give All Weapons");
        btn->onClick->Add([]() {
            for (auto& w : kDexWeapons)
                Trainer::GiveWeapon(w.id, 99999);
        });
    }

    {
        auto btn = win->AddButton("Clear Weapons");
        btn->onClick->Add([]() { Trainer::ClearWeapons(); });
    }

    win->AddCheckbox("Infinite Ammo", &Trainer::infiniteAmmo);

    // TASK3: weapon categories — no more endless scrolling.
    for (auto& cat : kDexWeaponCats)
    {
        auto btn = win->AddButton(cat.title);
        const char* t = cat.title;
        const DexWeapon* ws = cat.weapons;
        int n = cat.count;
        btn->onClick->Add([t, ws, n]() { DexTrainerUI::OpenWeaponCategory(t, ws, n); });
    }
    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

void DexTrainerUI::OpenWorldWindow()
{
    // World-only: weather + time control.
    // (Body/Suicide moved to Player, BlowUpCars moved to Vehicle.)
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "World");
    worldWindow = win;

    // TASK2: title-bar X -> back to main (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    // Time control FIRST (user priority) — CClock
    win->AddIntRange("Hour", &Trainer::timeHour, 0, 23);
    win->AddIntRange("Minute", &Trainer::timeMinute, 0, 59);
    {
        auto btn = win->AddButton("Set Time");
        btn->onClick->Add([]() { Trainer::SetTime(Trainer::timeHour, Trainer::timeMinute); });
    }

    // REMOVED 2026-10-15: Game speed slider removed (user: no slider features)

    // Weather (CCheat::*)
    const char* weatherNames[] = {"Extra Sunny", "Sunny", "Cloudy", "Rainy", "Foggy", "Storm", "Sandstorm"};
    for (int i = 0; i < 7; ++i)
    {
        auto btn = win->AddButton(weatherNames[i]);
        int idx = i;
        btn->onClick->Add([idx]() { Trainer::CheatWeather(idx); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

// Clothes main window — 6 stores (opened from Player window).
void DexTrainerUI::OpenClothesWindow()
{
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, "Clothes");
    clothesWindow = win;

    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::EnsureMainWindow();
    };

    for (int i = 0; i < Trainer::GetClothesStoreCount(); i++)
    {
        const char* name = Trainer::GetClothesStoreName(i);
        std::string label = (name ? name : "?");
        auto btn = win->AddButton(label);
        int store = i;
        btn->onClick->Add([store]() { DexTrainerUI::OpenClothesStore(store); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::EnsureMainWindow();
        });
    }
}

// Clothes store window — 7 categories for the chosen store.
void DexTrainerUI::OpenClothesStore(int store)
{
    CloseAllDexWindows();

    const char* storeName = Trainer::GetClothesStoreName(store);
    auto win = menuSZK->CreateWindow(140, 120, 480, storeName ? storeName : "Store");
    clothesStoreWindow = win;

    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::OpenClothesWindow();
    };

    for (int i = 0; i < Trainer::GetClothesCatCount(); i++)
    {
        const char* name = Trainer::GetClothesCatName(i);
        auto btn = win->AddButton(name ? name : "?");
        int cat = i;
        btn->onClick->Add([store, cat]() { DexTrainerUI::OpenClothesCategory(store, cat); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::OpenClothesWindow();
        });
    }
}

// Clothes item window — items for store + category.
void DexTrainerUI::OpenClothesCategory(int store, int category)
{
    CloseAllDexWindows();

    const char* catName = Trainer::GetClothesCatName(category);
    auto win = menuSZK->CreateWindow(140, 120, 480, catName ? catName : "Items");
    clothesCategoryWindow = win;

    win->onXButton = [store]() {
        CloseAllDexWindows();
        DexTrainerUI::OpenClothesStore(store);
    };

    int count = Trainer::GetClothesCount(store, category);
    for (int i = 0; i < count; ++i)
    {
        const char* display; const char* tex; const char* model; int bodyPart;
        Trainer::GetClothesItem(store, category, i, &display, &tex, &model, &bodyPart);
        auto btn = win->AddButton(display ? display : "?");
        std::string t = tex ? tex : "";
        std::string m = model ? model : "";
        btn->onClick->Add([t, m, bodyPart]() { Trainer::SetClothing(t.c_str(), m.c_str(), bodyPart); });
    }

    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([store]() {
            CloseAllDexWindows();
            DexTrainerUI::OpenClothesStore(store);
        });
    }
}


void DexTrainerUI::OpenWeaponCategory(const char* catTitle,
                                     const DexWeapon* weapons, int count)
{
    // One window at a time.
    CloseAllDexWindows();

    auto win = menuSZK->CreateWindow(140, 120, 480, catTitle);

    // TASK2: title-bar X -> back to Weapons (same as < Back).
    win->onXButton = []() {
        CloseAllDexWindows();
        DexTrainerUI::OpenWeaponsWindow();
    };

    for (int i = 0; i < count; ++i)
    {
        auto btn = win->AddButton(weapons[i].name);
        int id = weapons[i].id;
        btn->onClick->Add([id]() { Trainer::GiveWeapon(id, 99999); });
    }
    {
        auto btn = win->AddButton("< Back");
        win->stickyBackButton = true; // Sticky: Back always visible, outside pagination
        btn->onClick->Add([]() {
            CloseAllDexWindows();
            DexTrainerUI::OpenWeaponsWindow();
        });
    }
}


void DexTrainerUI::CloseAll()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    // Close by title to be safe (pointers may dangle after Close)
    const char* titles[] = {
        "DexTrainer v1.0",
        "Player",
        "Body",
        "Teleport",
        "Vehicle",
        "Weapons",
        "World",
        "Clothes",
        "Binco", "ProLaps", "SubUrban", "Zip", "Victim", "Didier Sachs",
        "Melee", "Pistols", "Shotguns", "SMGs",
        "Assault Rifles", "Rifles", "Heavy", "Thrown",
        "Hats", "Torso", "Legs", "Shoes", "Glasses", "Watches", "Necklaces",
        "Cars", "Bikes", "Boats", "Helis", "Planes", "Others",
    };
    // Collect first, close after (Close mutates allWindows)
    std::vector<IWindow*> toClose;
    for (auto w : allWindows)
    {
        for (auto t : titles)
        {
            if (w->title == t) { toClose.push_back(w); break; }
        }
    }
    for (auto w : toClose)
        w->Close();

    mainWindow = playerWindow = teleportWindow = vehicleWindow = weaponsWindow = worldWindow = nullptr;
    clothesWindow = clothesStoreWindow = clothesCategoryWindow = nullptr;
}
