#pragma once

// Trainer GTA SA Android (RZL-like) — Phase 1 + Phase 2
// Built on top of MenuSZK. All game symbols verified from libGTASA.so (ARM64).
//
// Phase 2 (2026-10-15): Adopted CheatMenu v2.53 techniques:
//   - GodMode via CPlayerPed::bDebugPlayerInvincible (not per-frame health)
//   - Per-type Fix (CAutomobile/CBike/CQuadBike)
//   - CCheat::* for weather/body/misc
//
// Vehicle spawn (2026-10-15, FIXED):
//   The direct-factory path (CCarCtrl::GetNewVehicleDependingOnCarModel) raced
//   the async streaming layer. Crash signature:
//     CStreamingInfo::m_status reached LOADED (=2) BEFORE
//     CVehicleModelInfo::m_pVehicleStruct was allocated. CreateInstance then
//     dereferenced NULL + 0x358 (m_pVehicleStruct->m_nMaskComponentsDamagable).
//   Replaced with two correct paths, tried in order:
//     1. SAUtils::CreateVehicle — blocking model load, game-script VM
//        internally. Only used if SAUtils is loaded.
//     2. Command<CREATE_CAR> (opcode 0x00A5) via PSDK OpcodeCaller.
//        Always available — pure game script processor, no extra module.
//
// Verified symbols/offsets (see TRAINER_DESIGN.md for RE notes):
//   _Z13FindPlayerPedi                      -> CPlayerPed* FindPlayerPed(int)
//   _ZN6CWorld7PlayersE                      -> CPlayerInfo[2], sizeof=472
//   CPlayerInfo+0xF0                         -> m_nMoney (int)
//   CPed+0x6AC                               -> m_fHealth (float)
//   CPed+0x6B4                               -> m_fArmour (float)
//   CPed+0x540 -> CPlayerData* -> +0x0        -> m_pWanted (CWanted*)
//   _ZN7CWanted14SetWantedLevelEi            -> void CWanted::SetWantedLevel(int)
//   _ZN4CPed10GiveWeaponE11eWeaponTypejb     -> void CPed::GiveWeapon(int, uint32, bool)
//   _ZN4CPed8TeleportE7CVectorh              -> void CPed::Teleport(CVector, uint8)
//   _ZN11CPlayerInfo6GetPosEv                -> CVector CPlayerInfo::GetPos()
//   _Z17FindPlayerVehicleib                  -> CVehicle* FindPlayerVehicle(int, bool)
//   _ZN11CAutomobile3FixEv                   -> void CAutomobile::Fix()
//   _ZN11CAutomobile8TeleportE7CVectorh       -> void CAutomobile::Teleport(CVector, uint8)
//   _ZN10CStreaming12RequestModelEii         -> void CStreaming::RequestModel(int, int)
//   _ZN10CStreaming22LoadAllRequestedModelsEb-> void CStreaming::LoadAllRequestedModels(bool)
//
// Phase 2 symbols (all verified via llvm-nm on libGTASA.so 2026-10-15):
//   _ZN10CPlayerPed22bDebugPlayerInvincibleE -> bool (BSS) — GodMode flag
//   _ZN5CBike3FixEv / _ZN9CQuadBike3FixEv    -> per-type Fix
//   _ZN10CModelInfo11IsBikeModelEi / 15IsQuadBikeModelEi -> type detection for Fix
//   _ZN4CPed12ClearWeaponsEv                 -> void CPed::ClearWeapons()
//   CCheat: ExtraSunny/Sunny/Cloudy/Rainy/Foggy/Storm/Sandstorm WeatherCheat,
//           Fat/Muscle/SkinnyCheat, SuicideCheat, BlowUpCarsCheat (all void(void))

#include "pch.h"

struct TrainerVector {
    float x, y, z;
};

class Trainer {
public:
    static void Initialize();   // bind game functions (call from MenuSZK::Initialize)
    static void Render();       // ImGui window with tabs (call from OnDraw when menuVisible)
    static void Update();       // per-frame toggles (call from MenuSZK::OnGameUpdate)

    // State (persisted in-memory; checkboxes bind directly)
    static bool godMode;
    static bool neverWanted;
    static bool infiniteAmmo;
    static int timeHour;
    static int timeMinute;

    // --- Game actions (public for DexTrainerUI) ---
    static void SetHealth(float v);
    static void SetArmour(float v);
    static void AddMoney(int amount);
    static void SetWantedLevel(int level);
    static int  GetWantedLevel();
    static void GiveWeapon(int id, int ammo);
    static void ClearWeapons();
    static void TeleportPlayer(float x, float y, float z);
    static void TeleportToMarker();
    static void FixPlayerVehicle();
    static void FlipPlayerVehicle();

    // Vehicle spawn — 10m in front of player. Uses SAUtils if loaded,
    // otherwise falls back to CREATE_CAR opcode via PSDK OpcodeCaller.
    // Never touches the direct factory (that path crashes — see header note).
    static void SpawnVehicle(int modelId);
    static bool spawnAsDriver; // unused — retained for API compatibility

    // Vehicle database (6 categories, 209 vehicles)
    static int GetVehicleCatCount();
    static const char* GetVehicleCatName(int cat);
    static int GetVehicleItemCount(int cat);
    static void GetVehicleItem(int cat, int idx, const char** display, int* modelId);

    // Phase 2 — CCheat wrappers
    static void CheatWeather(int idx);   // 0=ExtraSunny 1=Sunny 2=Cloudy 3=Rainy 4=Foggy 5=Storm 6=Sandstorm
    static void CheatBody(int idx);      // 0=Fat 1=Muscle 2=Skinny
    static void CheatSuicide();
    static void CheatJetpack();
    static void CheatBlowUpCars();
    static void SetGodMode(bool on);
    static void SetTime(int hour, int minute);

    // Clothing
    static void SetClothing(const char* tex, const char* model, int bodyPart);
    static int GetClothesCount(int store, int category);
    static void GetClothesItem(int store, int category, int idx, const char** display,
                               const char** tex, const char** model, int* bodyPart);
    static int GetClothesStoreCount();
    static const char* GetClothesStoreName(int store);
    static int GetClothesCatCount();
    static const char* GetClothesCatName(int category);

private:
    // --- Game bindings ---
    static void* GetPlayerPed();
    static void* GetPlayerInfo();
    static void* GetPlayerVehicle();
    static void* GetWantedPtr();
    static TrainerVector GetPlayerPos();

    static void RenderPlayerTab();
    static void RenderTeleportTab();
    static void RenderVehicleTab();
    static void RenderWeaponsTab();
};