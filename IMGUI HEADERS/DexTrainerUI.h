#pragma once

// DexTrainerUI — multi-window UI for DexTrainer (RZL-like trainer)
// Built on MenuSZK Window API. Game logic lives in Trainer (Trainer.h).
//
// Structure:
//   Main window "DexTrainer v1.0" -> 5 category buttons
//     -> Player | Teleport | Vehicle | Weapons | World (Phase 2)
//   Player -> Clothes (top) -> Store -> Category -> items

#include "pch.h"
#include "menuSZK/IWindow.h"

// Shared weapon entry (id + display name). Defined here so the
// category API can reference it.
struct DexWeapon { int id; const char* name; };

class DexTrainerUI {
public:
    // Called from MenuSZK::EnsureMenuContent() — creates main window if none open.
    static void EnsureMainWindow();

    // Category window creators (called from main window buttons).
    static void OpenPlayerWindow();
    static void OpenBodyWindow(); // Body stats sub-menu (Fat/Muscle/Skinny)
    static void OpenTeleportWindow();
    static void OpenVehicleWindow();
    static void OpenWeaponsWindow();
    static void OpenWorldWindow(); // Phase 2: weather, time

    // Weapon sub-category window (TASK3: weapons grouped by category).
    static void OpenWeaponCategory(const char* catTitle,
                                   const struct DexWeapon* weapons, int count);

    // Clothes windows (CheatMenu method: SetTextureAndModel).
    // Player -> Clothes -> Store -> Category -> items.
    static void OpenClothesWindow();
    static void OpenClothesStore(int store);
    static void OpenClothesCategory(int store, int category);

    // Vehicle spawn sub-category window.
    static void OpenVehicleCategory(int cat);

    // Close all DexTrainer windows (e.g. when menu hides).
    static void CloseAll();

private:
    // Track open windows to avoid duplicates. Null when closed.
    static IWindow* mainWindow;
    static IWindow* playerWindow;
    static IWindow* bodyWindow; // Body stats sub-menu
    static IWindow* teleportWindow;
    static IWindow* vehicleWindow;
    static IWindow* weaponsWindow;
    static IWindow* worldWindow; // Phase 2
    static IWindow* clothesWindow;
    static IWindow* clothesStoreWindow;
    static IWindow* clothesCategoryWindow;
    static IWindow* vehicleCategoryWindow;

    static void OnWindowClosed(IWindow** slot);
};
