// DexTrainer GTA SA Android (RZL-like) — Phase 1 implementation.
// All game functions bound via aml->GetSym with VERIFIED mangled names from libGTASA.so.
//
// Vehicle spawn (2026-10-15): deferred via SAUtils::ScriptCommand.
//   Direct CREATE_CAR races streaming: SAUtils' fake script thread can't yield
//   like the game's own processor does, so LOAD_ALL_MODELS_NOW returns
//   immediately and CREATE_CAR sees an unpopulated CVehicleModelInfo
//   (m_pVehicleStruct == NULL) → SIGSEGV at CreateInstance + 0x358.
//   We now request the model, wait several frames for the DFF, verify via
//   CModelInfo::ms_modelInfoPtrs that m_pRwObject is populated, then spawn.

#include "Trainer.h"
#include "MenuSZK.h"
#include "Localization.h"
#include "vehicle_db.h"
#include "imgui/imgui.h"

#include <isautils.h>
extern ISAUtils* sautils;

#include <dlfcn.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <time.h>

// ---------------------------------------------------------------------------
// Game function pointers (bound in Initialize)
// ---------------------------------------------------------------------------
static void* (*g_FindPlayerPed)(int) = nullptr;
static void* (*g_FindPlayerVehicle)(int, bool) = nullptr;
static void* g_Players = nullptr;
static void* g_CPlayerInfo_GetPos_addr = nullptr;

#if defined(__aarch64__)
static inline TrainerVector Call_GetPos(void* info) {
    TrainerVector v{0, 0, 0};
    if (!g_CPlayerInfo_GetPos_addr || !info) return v;
    register void* x0 __asm__("x0") = info;
    __asm__ volatile(
        "blr %[fn]\n"
        "str s0, [%[v], #0]\n"
        "str s1, [%[v], #4]\n"
        "str s2, [%[v], #8]\n"
        : "+r"(x0)
        : [fn]"r"(g_CPlayerInfo_GetPos_addr), [v]"r"(&v)
        : "x1","x2","x3","x4","x5","x6","x7","x8","x9","x10",
          "x11","x12","x13","x14","x15","x16","x17","x30",
          "s0","s1","s2","s3","memory","cc"
    );
    return v;
}
#else
static inline TrainerVector Call_GetPos(void* info) {
    (void)info;
    return TrainerVector{0,0,0};
}
#endif

static void (*g_CWanted_SetWantedLevel)(void*, int) = nullptr;
static void (*g_CPed_GiveWeapon)(void*, int, uint32_t, bool) = nullptr;
static void (*g_CPed_SetCurrentWeapon)(void*, int) = nullptr;
static void (*g_CPed_Teleport)(void*, TrainerVector, uint8_t) = nullptr;
static float (*g_FindGroundZForCoord)(float, float) = nullptr;
static float (*g_FindPlayerHeading)(int) = nullptr;
static void (*g_CVehicle_Fix)(void*) = nullptr;
static void (*g_CAutomobile_Teleport)(void*, TrainerVector, uint8_t) = nullptr;
static void (*g_CStreaming_RequestModel)(int, int) = nullptr;
static void (*g_CStreaming_LoadAllRequestedModels)(bool) = nullptr;

static uint8_t* g_aInfoForModel = nullptr;
static bool* g_bDebugPlayerInvincible = nullptr;
static void (*g_CBike_Fix)(void*) = nullptr;
static void (*g_CQuadBike_Fix)(void*) = nullptr;
static bool (*g_IsBikeModel)(int) = nullptr;
static bool (*g_IsQuadBikeModel)(int) = nullptr;
static void (*g_CPed_ClearWeapons)(void*) = nullptr;
static void (*g_CCheat_ExtraSunny)(void) = nullptr;
static void (*g_CCheat_Sunny)(void) = nullptr;
static void (*g_CCheat_Cloudy)(void) = nullptr;
static void (*g_CCheat_Rainy)(void) = nullptr;
static void (*g_CCheat_Foggy)(void) = nullptr;
static void (*g_CCheat_Storm)(void) = nullptr;
static void (*g_CCheat_Sandstorm)(void) = nullptr;
static void (*g_CCheat_Fat)(void) = nullptr;
static void (*g_CCheat_Muscle)(void) = nullptr;
static void (*g_CCheat_Skinny)(void) = nullptr;
static void (*g_CCheat_Suicide)(void) = nullptr;
static void (*g_CCheat_Jetpack)(void) = nullptr;
static void (*g_CCheat_BlowUpCars)(void) = nullptr;
static void* g_RadarTrace = nullptr;
static uint8_t* g_CClock_Hours = nullptr;
static uint8_t* g_CClock_Minutes = nullptr;
static void (*g_CClock_SetGameClock)(uint8_t, uint8_t, uint8_t) = nullptr;
static void (*g_CPedClothesDesc_SetTextureAndModel)(void*, const char*, const char*, int) = nullptr;
static void (*g_CClothes_RebuildPlayer)(void*, bool) = nullptr;

// CModelInfo::ms_modelInfoPtrs — array of CBaseModelInfo*.
static void** g_ms_modelInfoPtrs = nullptr;

// CStreamingInfo layout (CHECKSIZE 0x14):
static constexpr int SIZEOF_STREAMINGINFO = 20;
static constexpr int OFF_LOADSTATE = 0x10;
static constexpr uint8_t LOADSTATE_LOADED = 2;

static constexpr int OFF_PLAYERINFO_MONEY        = 0xF0;
static constexpr int OFF_PED_HEALTH              = 0x6AC;
static constexpr int OFF_PED_ARMOUR              = 0x6B4;
static constexpr int OFF_PED_PLAYERDATA          = 0x540;
static constexpr int OFF_PLAYERDATA_WANTED       = 0x0;
static constexpr int OFF_PLAYERDATA_CLOTHESDESC  = 0x8;
static constexpr int OFF_WANTED_LEVEL            = 0x2C;
static constexpr int SIZEOF_PLAYERINFO           = 472;

// Offset of CBaseModelInfo::m_pRwObject on arm64.
static constexpr int OFF_MI_RWOBJECT = 0x40;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
bool Trainer::godMode = false;
bool Trainer::neverWanted = false;
bool Trainer::infiniteAmmo = false;
int Trainer::timeHour = 12;
int Trainer::timeMinute = 0;
bool Trainer::spawnAsDriver = false;

static char g_statusBuf[128] = "";

// ---------------------------------------------------------------------------
// Script opcode definitions (all with static storage duration).
// ---------------------------------------------------------------------------
static const SCRIPT_COMMAND scm_GET_GROUND_Z     = { 0x02CE, "fffv"  };
static const SCRIPT_COMMAND scm_CREATE_CAR       = { 0x00A5, "ifffv" };
static const SCRIPT_COMMAND scm_SET_CAR_HEADING  = { 0x0175, "if"    };
static const SCRIPT_COMMAND scm_MARK_CAR         = { 0x01C3, "i"     };
static const SCRIPT_COMMAND scm_GET_PLAYER_CHAR  = { 0x01F5, "iv"    };
static const SCRIPT_COMMAND scm_WARP_CHAR_IN_CAR = { 0x036A, "ii"    };
static const SCRIPT_COMMAND scm_SET_CAR_FWD_SPD  = { 0x04BA, "if"    };

// Output variables — static because SAUtils writes back through the pointers.
static float s_groundZOut     = 0.0f;
static int   s_spawnHandleOut = 0;
static int   s_pedHandleOut   = 0;

// Pending-spawn state
static int   s_pendingModelId = -1;
static int   s_pendingFrames  = 0;
static float s_pendingX       = 0.0f;
static float s_pendingY       = 0.0f;
static float s_pendingZ       = 0.0f;
static float s_pendingHeading = 0.0f;
static int   s_pendingKind    = 0;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void BindSym(void** out, const char* name) {
    void* h = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_LAZY);
    if (!h) h = dlopen("libGTASA.so", RTLD_LAZY);
    if (h) *out = dlsym(h, name);
}

void Trainer::Initialize() {
    BindSym((void**)&g_FindPlayerPed, "_Z13FindPlayerPedi");
    BindSym((void**)&g_FindPlayerVehicle, "_Z17FindPlayerVehicleib");
    BindSym(&g_Players, "_ZN6CWorld7PlayersE");
    BindSym((void**)&g_CPlayerInfo_GetPos_addr, "_ZN11CPlayerInfo6GetPosEv");
    BindSym((void**)&g_CWanted_SetWantedLevel, "_ZN7CWanted14SetWantedLevelEi");
    BindSym((void**)&g_CPed_GiveWeapon, "_ZN4CPed10GiveWeaponE11eWeaponTypejb");
    BindSym((void**)&g_CPed_SetCurrentWeapon, "_ZN4CPed16SetCurrentWeaponE11eWeaponType");
    BindSym((void**)&g_CPed_Teleport, "_ZN4CPed8TeleportE7CVectorh");
    BindSym((void**)&g_FindGroundZForCoord, "_ZN6CWorld19FindGroundZForCoordEff");
    BindSym((void**)&g_FindPlayerHeading, "_Z17FindPlayerHeadingi");
    BindSym((void**)&g_CVehicle_Fix, "_ZN11CAutomobile3FixEv");
    BindSym((void**)&g_CAutomobile_Teleport, "_ZN11CAutomobile8TeleportE7CVectorh");
    BindSym((void**)&g_CStreaming_RequestModel, "_ZN10CStreaming12RequestModelEii");
    BindSym((void**)&g_CStreaming_LoadAllRequestedModels, "_ZN10CStreaming22LoadAllRequestedModelsEb");
    BindSym((void**)&g_aInfoForModel, "_ZN10CStreaming16ms_aInfoForModelE");
    BindSym((void**)&g_bDebugPlayerInvincible, "_ZN10CPlayerPed22bDebugPlayerInvincibleE");
    BindSym((void**)&g_CBike_Fix, "_ZN5CBike3FixEv");
    BindSym((void**)&g_CQuadBike_Fix, "_ZN9CQuadBike3FixEv");
    BindSym((void**)&g_IsBikeModel, "_ZN10CModelInfo11IsBikeModelEi");
    BindSym((void**)&g_IsQuadBikeModel, "_ZN10CModelInfo15IsQuadBikeModelEi");
    BindSym((void**)&g_CPed_ClearWeapons, "_ZN4CPed12ClearWeaponsEv");
    BindSym((void**)&g_CCheat_ExtraSunny, "_ZN6CCheat22ExtraSunnyWeatherCheatEv");
    BindSym((void**)&g_CCheat_Sunny, "_ZN6CCheat17SunnyWeatherCheatEv");
    BindSym((void**)&g_CCheat_Cloudy, "_ZN6CCheat18CloudyWeatherCheatEv");
    BindSym((void**)&g_CCheat_Rainy, "_ZN6CCheat17RainyWeatherCheatEv");
    BindSym((void**)&g_CCheat_Foggy, "_ZN6CCheat17FoggyWeatherCheatEv");
    BindSym((void**)&g_CCheat_Storm, "_ZN6CCheat10StormCheatEv");
    BindSym((void**)&g_CCheat_Sandstorm, "_ZN6CCheat14SandstormCheatEv");
    BindSym((void**)&g_CCheat_Fat, "_ZN6CCheat8FatCheatEv");
    BindSym((void**)&g_CCheat_Muscle, "_ZN6CCheat11MuscleCheatEv");
    BindSym((void**)&g_CCheat_Skinny, "_ZN6CCheat11SkinnyCheatEv");
    BindSym((void**)&g_CCheat_Suicide, "_ZN6CCheat12SuicideCheatEv");
    BindSym((void**)&g_CCheat_Jetpack, "_ZN6CCheat12JetpackCheatEv");
    BindSym((void**)&g_CCheat_BlowUpCars, "_ZN6CCheat15BlowUpCarsCheatEv");
    BindSym(&g_RadarTrace, "_ZN6CRadar13ms_RadarTraceE");
    BindSym((void**)&g_CClock_Hours, "_ZN6CClock18ms_nGameClockHoursE");
    BindSym((void**)&g_CClock_Minutes, "_ZN6CClock20ms_nGameClockMinutesE");
    BindSym((void**)&g_CClock_SetGameClock, "_ZN6CClock12SetGameClockEhhh");
    BindSym((void**)&g_CPedClothesDesc_SetTextureAndModel, "_ZN15CPedClothesDesc18SetTextureAndModelEPKcS1_i");
    BindSym((void**)&g_CClothes_RebuildPlayer, "_ZN8CClothes13RebuildPlayerEP10CPlayerPedb");
    BindSym((void**)&g_ms_modelInfoPtrs, "_ZN10CModelInfo15ms_modelInfoPtrsE");
}

static bool IsModelLoaded(int modelId) {
    if (!g_aInfoForModel || modelId < 0) return false;
    uint8_t* entry = g_aInfoForModel + (size_t)modelId * SIZEOF_STREAMINGINFO;
    return entry[OFF_LOADSTATE] == LOADSTATE_LOADED;
}

static bool IsModelDffResident(int modelId) {
    if (!g_ms_modelInfoPtrs || modelId < 0) return true;
    void* modelInfo = g_ms_modelInfoPtrs[modelId];
    if (!modelInfo) return false;
    void* rwObject = *(void**)((char*)modelInfo + OFF_MI_RWOBJECT);
    return rwObject != nullptr;
}

static int WeaponModelFor(int weaponId) {
    switch (weaponId) {
        case 4:  return 335;  case 5:  return 336;  case 8:  return 339;
        case 9:  return 341;  case 16: return 342;  case 18: return 344;
        case 22: return 346;  case 23: return 347;  case 24: return 348;
        case 25: return 349;  case 26: return 350;  case 27: return 351;
        case 28: return 352;  case 29: return 353;  case 30: return 355;
        case 31: return 356;  case 33: return 357;  case 34: return 358;
        case 35: return 359;  case 36: return 360;  case 37: return 361;
        case 38: return 362;
        default: return -1;
    }
}

static int AircraftKind(int modelId) {
    switch (modelId) {
        case 460: case 476: case 511: case 512: case 513:
        case 519: case 520: case 553: case 577: case 592: case 593:
            return 1;
        case 417: case 425: case 447: case 469:
        case 487: case 488: case 497: case 548: case 563:
            return 2;
        default:
            return 0;
    }
}

// ---------------------------------------------------------------------------
// Player / state helpers
// ---------------------------------------------------------------------------
void* Trainer::GetPlayerPed() {
    return g_FindPlayerPed ? g_FindPlayerPed(0) : nullptr;
}
void* Trainer::GetPlayerInfo() { return g_Players; }
void* Trainer::GetPlayerVehicle() {
    return g_FindPlayerVehicle ? g_FindPlayerVehicle(-1, false) : nullptr;
}
TrainerVector Trainer::GetPlayerPos() {
    TrainerVector v{0, 0, 0};
    void* info = GetPlayerInfo();
    if (info && g_CPlayerInfo_GetPos_addr) v = Call_GetPos(info);
    return v;
}

void Trainer::SetHealth(float val) {
    void* ped = GetPlayerPed();
    if (ped) *(float*)((char*)ped + OFF_PED_HEALTH) = val;
}
void Trainer::SetArmour(float val) {
    void* ped = GetPlayerPed();
    if (ped) *(float*)((char*)ped + OFF_PED_ARMOUR) = val;
}
void Trainer::AddMoney(int amount) {
    void* info = GetPlayerInfo();
    if (info) *(int*)((char*)info + OFF_PLAYERINFO_MONEY) += amount;
}
void* Trainer::GetWantedPtr() {
    void* ped = GetPlayerPed();
    if (!ped) return nullptr;
    void* playerData = *(void**)((char*)ped + OFF_PED_PLAYERDATA);
    if (!playerData) return nullptr;
    return *(void**)((char*)playerData + OFF_PLAYERDATA_WANTED);
}
void Trainer::SetWantedLevel(int level) {
    void* wanted = GetWantedPtr();
    if (wanted && g_CWanted_SetWantedLevel) g_CWanted_SetWantedLevel(wanted, level);
}
int Trainer::GetWantedLevel() {
    void* wanted = GetWantedPtr();
    if (!wanted) return 0;
    return *(int*)((char*)wanted + OFF_WANTED_LEVEL);
}

void Trainer::GiveWeapon(int id, int ammo) {
    void* ped = GetPlayerPed();
    if (!ped || !g_CPed_GiveWeapon) return;
    int modelId = WeaponModelFor(id);
    if (modelId > 0 && g_CStreaming_RequestModel && g_CStreaming_LoadAllRequestedModels) {
        if (!IsModelLoaded(modelId)) {
            g_CStreaming_RequestModel(modelId, 2);
            g_CStreaming_LoadAllRequestedModels(false);
        }
    }
    g_CPed_GiveWeapon(ped, id, (uint32_t)ammo, false);
    if (g_CPed_SetCurrentWeapon) g_CPed_SetCurrentWeapon(ped, id);
}

void Trainer::TeleportPlayer(float x, float y, float z) {
    void* ped = GetPlayerPed();
    if (ped && g_CPed_Teleport) {
        TrainerVector v{x, y, z};
        g_CPed_Teleport(ped, v, 0);
    }
}

void Trainer::SetGodMode(bool on) {
    godMode = on;
    if (g_bDebugPlayerInvincible) *g_bDebugPlayerInvincible = on;
}

void Trainer::FixPlayerVehicle() {
    void* veh = GetPlayerVehicle();
    if (!veh) return;
    int16_t modelId = *(int16_t*)((char*)veh + 0x32);
    if (g_IsBikeModel && g_CBike_Fix && g_IsBikeModel(modelId))      g_CBike_Fix(veh);
    else if (g_IsQuadBikeModel && g_CQuadBike_Fix && g_IsQuadBikeModel(modelId)) g_CQuadBike_Fix(veh);
    else if (g_CVehicle_Fix)                                          g_CVehicle_Fix(veh);
}

void Trainer::FlipPlayerVehicle() {
    void* veh = GetPlayerVehicle();
    if (!veh || !g_CAutomobile_Teleport) return;
    TrainerVector p = GetPlayerPos();
    TrainerVector v{p.x, p.y, p.z + 2.0f};
    g_CAutomobile_Teleport(veh, v, 0);
}

void Trainer::ClearWeapons() {
    void* ped = GetPlayerPed();
    if (ped && g_CPed_ClearWeapons) g_CPed_ClearWeapons(ped);
}

void Trainer::TeleportToMarker() {
    if (!g_RadarTrace) { snprintf(g_statusBuf, sizeof(g_statusBuf), "marker: radar not bound"); return; }
    float mx = 0, my = 0; bool found = false;
    for (int i = 0; i < 249; i++) {
        uint8_t* e = (uint8_t*)g_RadarTrace + (size_t)i * 0x30;
        uint8_t sprite = e[0x28];
        bool inUse = (e[0x29] & 0x2) != 0;
        if (inUse && sprite == 41) {
            mx = *(float*)(e + 0x8); my = *(float*)(e + 0xC);
            found = true; break;
        }
    }
    if (!found) { snprintf(g_statusBuf, sizeof(g_statusBuf), "marker: no waypoint set"); return; }
    void* ped = GetPlayerPed();
    if (ped && g_CPed_Teleport) {
        float mz = 0.0f;
        if (g_FindGroundZForCoord) mz = g_FindGroundZForCoord(mx, my);
        if (mz <= 0.0f) mz = 2.0f;
        TrainerVector v{mx, my, mz + 1.0f};
        g_CPed_Teleport(ped, v, 0);
        snprintf(g_statusBuf, sizeof(g_statusBuf), "teleported to marker");
    }
}

void Trainer::CheatWeather(int idx) {
    switch (idx) {
        case 0: if (g_CCheat_ExtraSunny) g_CCheat_ExtraSunny(); break;
        case 1: if (g_CCheat_Sunny)      g_CCheat_Sunny();      break;
        case 2: if (g_CCheat_Cloudy)     g_CCheat_Cloudy();     break;
        case 3: if (g_CCheat_Rainy)      g_CCheat_Rainy();      break;
        case 4: if (g_CCheat_Foggy)      g_CCheat_Foggy();      break;
        case 5: if (g_CCheat_Storm)      g_CCheat_Storm();      break;
        case 6: if (g_CCheat_Sandstorm)  g_CCheat_Sandstorm();  break;
    }
}
void Trainer::CheatBody(int idx) {
    switch (idx) {
        case 0: if (g_CCheat_Fat)    g_CCheat_Fat();    break;
        case 1: if (g_CCheat_Muscle) g_CCheat_Muscle(); break;
        case 2: if (g_CCheat_Skinny) g_CCheat_Skinny(); break;
    }
}
void Trainer::CheatSuicide()      { if (g_CCheat_Suicide)      g_CCheat_Suicide();      }
void Trainer::CheatJetpack()      { if (g_CCheat_Jetpack)      g_CCheat_Jetpack();      }
void Trainer::CheatBlowUpCars()   { if (g_CCheat_BlowUpCars)   g_CCheat_BlowUpCars();   }

void Trainer::SetTime(int hour, int minute) {
    if (hour < 0) hour = 0; if (hour > 23) hour = 23;
    if (minute < 0) minute = 0; if (minute > 59) minute = 59;
    if (g_CClock_SetGameClock) {
        g_CClock_SetGameClock((uint8_t)hour, (uint8_t)minute, 0);
    } else {
        if (g_CClock_Hours)   *g_CClock_Hours   = (uint8_t)hour;
        if (g_CClock_Minutes) *g_CClock_Minutes = (uint8_t)minute;
    }
    timeHour = hour; timeMinute = minute;
}

void Trainer::SetClothing(const char* tex, const char* model, int bodyPart) {
    if (!g_CPedClothesDesc_SetTextureAndModel || !g_CClothes_RebuildPlayer) {
        snprintf(g_statusBuf, sizeof(g_statusBuf), "clothes: N/A"); return;
    }
    void* player = GetPlayerPed();
    if (!player) { snprintf(g_statusBuf, sizeof(g_statusBuf), "clothes: no player"); return; }
    void* playerData = *(void**)((uint8_t*)player + OFF_PED_PLAYERDATA);
    if (!playerData) { snprintf(g_statusBuf, sizeof(g_statusBuf), "clothes: no playerData"); return; }
    void* clothesDesc = *(void**)((uint8_t*)playerData + OFF_PLAYERDATA_CLOTHESDESC);
    if (!clothesDesc) { snprintf(g_statusBuf, sizeof(g_statusBuf), "clothes: no clothesDesc"); return; }
    g_CPedClothesDesc_SetTextureAndModel(clothesDesc, tex, model, bodyPart);
    g_CClothes_RebuildPlayer(player, false);
    snprintf(g_statusBuf, sizeof(g_statusBuf), "wore %s", tex ? tex : "?");
}

#include "clothes_db.h"

int Trainer::GetClothesCount(int store, int category) {
    if (store < 0 || store >= 6 || category < 0 || category >= 7) return 0;
    return g_clothes_db[store][category].count;
}
void Trainer::GetClothesItem(int store, int category, int idx, const char** display,
                             const char** tex, const char** model, int* bodyPart) {
    if (store < 0 || store >= 6 || category < 0 || category >= 7) return;
    const ClothesDBCat& cat = g_clothes_db[store][category];
    if (idx < 0 || idx >= cat.count) return;
    const ClothesDBItem& item = cat.items[idx];
    if (display) *display = item.display;
    if (tex) *tex = item.tex;
    if (model) *model = item.model;
    if (bodyPart) *bodyPart = item.bodyPart;
}
int Trainer::GetClothesStoreCount() { return 6; }
const char* Trainer::GetClothesStoreName(int store) {
    if (store < 0 || store >= 6) return nullptr;
    return g_clothes_stores[store];
}
int Trainer::GetClothesCatCount() { return 7; }
const char* Trainer::GetClothesCatName(int category) {
    if (category < 0 || category >= 7) return nullptr;
    return g_clothes_cats[category];
}

// ---------------------------------------------------------------------------
// Vehicle spawn — deferred, DFF-aware.
// ---------------------------------------------------------------------------
static void DoSpawnNow(int modelId, int kind, float heading,
                       float x, float y, float z) {
    if (!sautils) return;

    if (kind == 0) {
        s_groundZOut = z;
        sautils->ScriptCommand(&scm_GET_GROUND_Z, x, y, z + 10.0f, &s_groundZOut);
        if (s_groundZOut > 0.0f) z = s_groundZOut + 0.5f;
        else                     z = z + 0.5f;
    }

    s_spawnHandleOut = 0;
    sautils->ScriptCommand(&scm_CREATE_CAR, modelId, x, y, z, &s_spawnHandleOut);

    if (s_spawnHandleOut > 0) {
        sautils->ScriptCommand(&scm_SET_CAR_HEADING, s_spawnHandleOut, heading);

        if (kind != 0) {
            s_pedHandleOut = 0;
            sautils->ScriptCommand(&scm_GET_PLAYER_CHAR, 0, &s_pedHandleOut);
            if (s_pedHandleOut > 0) {
                sautils->ScriptCommand(&scm_WARP_CHAR_IN_CAR, s_pedHandleOut, s_spawnHandleOut);
            }
            const float speed = (kind == 1) ? 90.0f : 30.0f;
            sautils->ScriptCommand(&scm_SET_CAR_FWD_SPD, s_spawnHandleOut, speed);
        }

        sautils->ScriptCommand(&scm_MARK_CAR, s_spawnHandleOut);
        snprintf(g_statusBuf, sizeof(g_statusBuf), "spawned (model %d)", modelId);
    } else {
        snprintf(g_statusBuf, sizeof(g_statusBuf), "CREATE_CAR failed for %d", modelId);
    }

    sautils->MarkModelAsNotNeeded(modelId);
}

void Trainer::SpawnVehicle(int modelId) {
    if (modelId <= 0) return;
    if (!sautils) {
        snprintf(g_statusBuf, sizeof(g_statusBuf), "spawn: SAUtils not loaded");
        return;
    }
    if (!GetPlayerPed()) {
        snprintf(g_statusBuf, sizeof(g_statusBuf), "spawn: no player");
        return;
    }

    TrainerVector p = GetPlayerPos();
    float heading = g_FindPlayerHeading ? g_FindPlayerHeading(0) : 0.0f;
    int kind = AircraftKind(modelId);

    if (s_pendingModelId >= 0 && sautils) {
        sautils->MarkModelAsNotNeeded(s_pendingModelId);
    }

    sautils->LoadModelId(modelId);

    s_pendingModelId = modelId;
    s_pendingFrames  = 0;
    s_pendingKind    = kind;
    s_pendingHeading = heading;

    const float rad  = heading * 3.14159265f / 180.0f;
    const float dist = (kind != 0) ? 15.0f : 4.0f;
    s_pendingX = p.x + sinf(rad) * dist;
    s_pendingY = p.y + cosf(rad) * dist;
    s_pendingZ = p.z + ((kind != 0) ? 60.0f : 1.0f);

    snprintf(g_statusBuf, sizeof(g_statusBuf), "loading model %d...", modelId);
}

int Trainer::GetVehicleCatCount() { return 6; }
const char* Trainer::GetVehicleCatName(int cat) {
    if (cat < 0 || cat >= 6) return nullptr;
    return g_vehicle_db[cat].title;
}
int Trainer::GetVehicleItemCount(int cat) {
    if (cat < 0 || cat >= 6) return 0;
    return g_vehicle_db[cat].count;
}
void Trainer::GetVehicleItem(int cat, int idx, const char** display, int* modelId) {
    if (cat < 0 || cat >= 6) return;
    const VehicleDBCat& c = g_vehicle_db[cat];
    if (idx < 0 || idx >= c.count) return;
    if (display) *display = c.items[idx].display;
    if (modelId) *modelId = c.items[idx].modelId;
}

// ---------------------------------------------------------------------------
// Per-frame update
// ---------------------------------------------------------------------------
void Trainer::Update() {
    if (g_bDebugPlayerInvincible) {
        if (*g_bDebugPlayerInvincible != godMode) {
            *g_bDebugPlayerInvincible = godMode;
        }
    }
    if (neverWanted) {
        if (GetWantedLevel() > 0) SetWantedLevel(0);
    }

    if (s_pendingModelId >= 0 && sautils) {
        s_pendingFrames++;

        const int timeout = (s_pendingKind != 0) ? 400 : 300;
        if (s_pendingFrames > timeout) {
            snprintf(g_statusBuf, sizeof(g_statusBuf),
                     "spawn timed out (model %d - not in game files?)",
                     s_pendingModelId);
            sautils->MarkModelAsNotNeeded(s_pendingModelId);
            s_pendingModelId = -1;
            return;
        }

        if (s_pendingFrames < 2) return;

        if (g_ms_modelInfoPtrs && !g_ms_modelInfoPtrs[s_pendingModelId]) {
            return;
        }

        if (!IsModelDffResident(s_pendingModelId)) {
            return;
        }

        const int   modelId = s_pendingModelId;
        const int   kind    = s_pendingKind;
        const float heading = s_pendingHeading;
        const float x       = s_pendingX;
        const float y       = s_pendingY;
        const float z       = s_pendingZ;
        s_pendingModelId = -1;

        DoSpawnNow(modelId, kind, heading, x, y, z);
    }
}

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------
#define TR(key) GetTranslatedText(key).c_str()

void Trainer::RenderPlayerTab() {
    ImGui::Checkbox(TR("trainer_godmode"), &godMode);
    if (ImGui::Button(TR("trainer_heal"), ImVec2(-1, 0)))  SetHealth(100.0f);
    if (ImGui::Button(TR("trainer_armor"), ImVec2(-1, 0))) SetArmour(100.0f);
    ImGui::Separator();
    if (ImGui::Button(TR("trainer_clearwanted"), ImVec2(-1, 0))) SetWantedLevel(0);
    ImGui::Checkbox(TR("trainer_neverwanted"), &neverWanted);
    ImGui::Separator();
    if (ImGui::Button(TR("trainer_addmoney"), ImVec2(-1, 0))) AddMoney(100000);
}

struct TeleportLoc { const char* key; float x, y, z; };
static const TeleportLoc kLocs[] = {
    {"trainer_loc_area69",     200.0f, 1900.0f, 18.0f},
    {"trainer_loc_grove",     2490.0f, -1680.0f, 13.0f},
    {"trainer_loc_ls_airport", 1950.0f, -2200.0f, 13.0f},
    {"trainer_loc_lv_airport", 1300.0f, 1250.0f, 13.0f},
    {"trainer_loc_chiliad",   -2300.0f, -1600.0f, 500.0f},
    {"trainer_loc_sf_airport", -1500.0f, -300.0f, 13.0f},
};

void Trainer::RenderTeleportTab() {
    for (auto& loc : kLocs) {
        if (ImGui::Button(TR(loc.key), ImVec2(-1, 0)))
            TeleportPlayer(loc.x, loc.y, loc.z);
    }
}

void Trainer::RenderVehicleTab() {
    if (g_statusBuf[0]) ImGui::TextDisabled("%s", g_statusBuf);
    if (ImGui::Button(TR("trainer_fixveh"), ImVec2(-1, 0)))  FixPlayerVehicle();
    if (ImGui::Button(TR("trainer_flipveh"), ImVec2(-1, 0))) FlipPlayerVehicle();
}

struct WeaponEntry { int id; const char* name; };
static const WeaponEntry kWeapons[] = {
    {4,"Knife"},{5,"Bat"},{8,"Katana"},{9,"Chainsaw"},
    {22,"Pistol"},{23,"Silenced"},{24,"Deagle"},
    {25,"Shotgun"},{26,"Sawn-off"},{27,"Combat SG"},
    {28,"Micro Uzi"},{29,"MP5"},{30,"AK-47"},{31,"M4"},
    {33,"Rifle"},{34,"Sniper"},{35,"RPG"},{36,"HS Rocket"},
    {37,"Flamethrower"},{38,"Minigun"},{16,"Grenade"},{18,"Molotov"},
};

void Trainer::RenderWeaponsTab() {
    if (ImGui::Button(TR("trainer_giveall"), ImVec2(-1, 0))) {
        for (auto& w : kWeapons) GiveWeapon(w.id, 99999);
    }
    ImGui::Checkbox(TR("trainer_infammo"), &infiniteAmmo);
    ImGui::Separator();
    ImGui::TextDisabled("%s", TR("trainer_weapons_hint"));
    for (auto& w : kWeapons) {
        if (ImGui::Button(w.name, ImVec2(-1, 0))) GiveWeapon(w.id, 99999);
    }
}

void Trainer::Render() {
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520, 480), ImGuiCond_FirstUseEver);
    bool open = true;
    if (!ImGui::Begin("DexTrainer v1.0", &open)) { ImGui::End(); return; }
    if (ImGui::BeginTabBar("##trainer_tabs")) {
        if (ImGui::BeginTabItem(TR("trainer_tab_player")))   { RenderPlayerTab();   ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem(TR("trainer_tab_teleport"))) { RenderTeleportTab(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem(TR("trainer_tab_vehicle")))  { RenderVehicleTab();  ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem(TR("trainer_tab_weapons")))  { RenderWeaponsTab();  ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}