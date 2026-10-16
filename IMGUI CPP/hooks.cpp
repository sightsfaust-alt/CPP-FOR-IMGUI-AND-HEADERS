#include "hooks.h"

#include "MenuSZK.h"
#include "Input.h"
#include "Debug.h"
#include "imgui/imgui_backend.h"

unsigned int prevTime = 0;

DECL_HOOK(void, PreRenderEnd, void* self)
{
    PreRenderEnd(self);

    menuSZK->OnUpdate();
    menuSZK->OnDraw();
}

bool isFirstTest = true;

// ---- Swipe gesture: top-center swipe-down toggles the menu ----
// Trigger zone: x in [35%, 65%] of screen width, y < 15% of screen height.
// Gesture: touch begins in the zone, then moves DOWN >= 150px with
// horizontal drift < 100px. Fires on threshold (no need to lift finger).
// The whole touch stream (begin..end) is swallowed while tracked.
// Timeout: 1.5s without reaching threshold -> cancel, pass through.
namespace {
struct SwipeGestureState {
    bool active = false;
    int touchId = -1;
    float startX = 0.0f;
    float startY = 0.0f;
    unsigned int startMs = 0;
    bool fired = false;
};
SwipeGestureState g_swipeGesture;

constexpr float kSwipeMinDownPx  = 150.0f;   // min downward travel
constexpr float kSwipeMaxDriftPx = 100.0f;   // max horizontal drift
constexpr unsigned int kSwipeTimeoutMs = 1500;
}

// Returns true if this touch event was consumed by the gesture tracker
// (caller must swallow it: do not forward to game/Input/ImGui).
static bool HandleSwipeGesture(int state, int touchId, int x, int y)
{
    unsigned int now = (m_snTimeInMilliseconds != nullptr)
                     ? *m_snTimeInMilliseconds : 0;

    if (state == 2) // touch begin
    {
        if (g_swipeGesture.active || menuSZK == nullptr)
            return false; // already tracking another finger

        int scrW = (OS_ScreenGetWidth != nullptr) ? OS_ScreenGetWidth() : 1280;
        int scrH = (OS_ScreenGetHeight != nullptr) ? OS_ScreenGetHeight() : 720;
        float fx = (float)x;
        float fy = (float)y;

        bool inZone = (fx >= scrW * 0.35f) && (fx <= scrW * 0.65f)
                   && (fy < scrH * 0.15f);
        if (!inZone)
            return false;

        g_swipeGesture.active  = true;
        g_swipeGesture.touchId = touchId;
        g_swipeGesture.startX  = fx;
        g_swipeGesture.startY  = fy;
        g_swipeGesture.startMs = now;
        g_swipeGesture.fired   = false;
        return true; // swallow: this touch belongs to the gesture
    }

    if (!g_swipeGesture.active || touchId != g_swipeGesture.touchId)
        return false; // not our tracked finger

    if (state == 3) // touch move
    {
        if (now - g_swipeGesture.startMs > kSwipeTimeoutMs)
        {
            g_swipeGesture.active = false; // timeout -> cancel, pass through
            return false;
        }

        float dx = (float)x - g_swipeGesture.startX;
        if (dx < 0.0f) dx = -dx;
        float dy = (float)y - g_swipeGesture.startY;

        if (dx >= kSwipeMaxDriftPx)
        {
            g_swipeGesture.active = false; // drifted sideways: not our gesture
            return false;
        }

        if (!g_swipeGesture.fired && dy >= kSwipeMinDownPx)
        {
            g_swipeGesture.fired = true;
            menuSZK->menuVisible = !menuSZK->menuVisible;
            if (menuSZK->menuVisible && menuSZK != nullptr)
                menuSZK->EnsureMenuContent(); // never open into empty state
            fileLog->Log(menuSZK->menuVisible
                ? "[MenuSZK] Menu shown via swipe gesture"
                : "[MenuSZK] Menu hidden via swipe gesture");
        }
        return true; // swallow while tracked
    }

    if (state == 1) // touch end
    {
        g_swipeGesture.active = false;
        return true; // swallow the end (begin was swallowed too)
    }

    return false;
}

DECL_HOOKv(AND_TouchEvent, int state, int touchId, int x, int y)
{
    if(isFirstTest)
    {
        menuDebug->AddLine("[Input] Testing touch...");

        ModData::CreateFile(modData->GetFile("TOUCH_ERROR"));
    }

    // Toggle button: fixed rect drawn via ImGui background draw list.
    // Handled HERE directly (no ImGui widget involved): on touch begin,
    // flip menuVisible and swallow the tap so it never reaches the game,
    // the menu system, or ImGui.
    if (menuSZK && MenuSZK::IsPointOnToggleButton((float)x, (float)y))
    {
        if (state == 2) // touch begin
        {
            menuSZK->menuVisible = !menuSZK->menuVisible;
            if (menuSZK->menuVisible)
                menuSZK->EnsureMenuContent(); // never open into empty state
            fileLog->Log(menuSZK->menuVisible ? "[MenuSZK] Menu shown via toggle"
                                              : "[MenuSZK] Menu hidden via toggle");
        }

        if(isFirstTest)
        {
            menuDebug->AddLine("[Input] Touch is working");

            const std::string errorFile = modData->GetFile("TOUCH_ERROR");
            if(remove(errorFile.c_str()) != 0) {}
        }

        isFirstTest = false;
        return;
    }

    // Swipe gesture: top-center swipe-down toggles the menu.
    // Alternative trigger alongside the draw-list toggle button.
    if (HandleSwipeGesture(state, touchId, x, y))
        return; // swallowed by the gesture tracker

    bool result = Input::ProcessTouchEvent(state, touchId, x, y);

    // Feed raw touch to ImGui (raw OS pixels; ImGui display size = OS screen size)
    ImGuiBackend::ProcessTouch(state, touchId, (float)x, (float)y);

    // SAFE MODAL (anti-trap): only swallow a touch when it actually hits
    // menu UI. Never blanket-swallow just because the menu is open.
    //  - result == false  -> an RW container consumed the touch
    //  - WantsMouseCapture -> touch is over an ImGui window/widget
    //  - otherwise -> forward to the game, even with the menu open.
    // Touches on empty space reach the game so the user can never get stuck
    // in an invisible/blocking state.
    bool uiConsumed = !result;
    if (!uiConsumed)
        uiConsumed = ImGuiBackend::WantsMouseCapture();
    if (!uiConsumed)
        AND_TouchEvent(state, touchId, x, y);

    if(isFirstTest)
    {
        menuDebug->AddLine("[Input] Touch is working");

        const std::string errorFile = modData->GetFile("TOUCH_ERROR");
        if(remove(errorFile.c_str()) != 0) {}
    }

    isFirstTest = false;
}

DECL_HOOK(void*, CGame_Process)
{
    //fileLog->Log("CGame::Process");
    
    if (BASS)
    {
        soundsys->Update();
    }
    
    auto time = *m_snTimeInMilliseconds;
    if(prevTime == 0) prevTime = time;
    menuSZK->deltaTime = time - prevTime;
    prevTime = time;

    if(menuSZK->deltaTime > 100) menuSZK->deltaTime = 100;

    menuSZK->OnGameUpdate();

    auto result = CGame_Process();

    return result;
}

DECL_HOOKv(InitRenderWare)
{
    InitRenderWare();

    flMenuMapScaling = (float)RsGlobal->maximumHeight / 448.0f;
}

DECL_HOOKv(PostRadarDraw, bool b)
{
    PostRadarDraw(b);

    menuSZK->OnPostDrawRadar();
}

void DoHooks()
{
    [[maybe_unused]] void* hGTASA = dlopen("libGTASA.so", RTLD_LAZY);
    [[maybe_unused]] uintptr_t pGTASA = aml->GetLib("libGTASA.so");

    SET_TO(CSprite2d_DrawRect, aml->GetSym(hGTASA, "_ZN9CSprite2d8DrawRectERK5CRectRK5CRGBA"));
    SET_TO(CSprite_CalcScreenCoors, aml->GetSym(hGTASA, "_ZN7CSprite15CalcScreenCoorsERK5RwV3dPS0_PfS4_bb"));
    SET_TO(FontSetOrientation,                  aml->GetSym(hGTASA, "_ZN5CFont14SetOrientationEh"));
    SET_TO(FontSetColor,                        aml->GetSym(hGTASA, "_ZN5CFont8SetColorE5CRGBA"));
    SET_TO(FontSetBackground,                   aml->GetSym(hGTASA, "_ZN5CFont13SetBackgroundEhh"));
    SET_TO(FontSetWrapx,                        aml->GetSym(hGTASA, "_ZN5CFont8SetWrapxEf"));
    SET_TO(FontSetStyle,                        aml->GetSym(hGTASA, "_ZN5CFont12SetFontStyleEh"));
    SET_TO(FontSetScale,                        aml->GetSym(hGTASA, "_ZN5CFont8SetScaleEf"));
    SET_TO(FontSetEdge,                         aml->GetSym(hGTASA, "_ZN5CFont7SetEdgeEa"));
    SET_TO(FontSetProportional,                 aml->GetSym(hGTASA, "_ZN5CFont15SetProportionalEh"));
    SET_TO(FontSetDropShadowPosition,           aml->GetSym(hGTASA, "_ZN5CFont21SetDropShadowPositionEa"));
    SET_TO(FontSetDropColor,                    aml->GetSym(hGTASA, "_ZN5CFont12SetDropColorE5CRGBA"));
    SET_TO(FontPrintString,                     aml->GetSym(hGTASA, "_ZN5CFont11PrintStringEffPt"));
    SET_TO(AsciiToGxtChar,                      aml->GetSym(hGTASA, "_Z14AsciiToGxtCharPKcPt"));
    SET_TO(RenderFontBuffer,                    aml->GetSym(hGTASA, "_ZN5CFont16RenderFontBufferEv"));
    SET_TO(OS_ScreenGetWidth, aml->GetSym(hGTASA, "_Z17OS_ScreenGetWidthv"));
    SET_TO(OS_ScreenGetHeight, aml->GetSym(hGTASA, "_Z18OS_ScreenGetHeightv"));
    SET_TO(RwRasterCreate, aml->GetSym(hGTASA, "_Z14RwRasterCreateiiii"));
    SET_TO(RwRasterDestroy, aml->GetSym(hGTASA, "_Z15RwRasterDestroyP8RwRaster"));
    SET_TO(RwRasterUnlock, aml->GetSym(hGTASA, "_Z14RwRasterUnlockP8RwRaster"));
    SET_TO(RwTextureSetMipmapping, aml->GetSym(hGTASA, "_Z22RwTextureSetMipmappingi"));
    SET_TO(RwTextureSetAutoMipmapping, aml->GetSym(hGTASA, "_Z26RwTextureSetAutoMipmappingi"));
    SET_TO(RwTextureCreate, aml->GetSym(hGTASA, "_Z15RwTextureCreateP8RwRaster"));
    SET_TO(RwRasterLock, aml->GetSym(hGTASA, "_Z12RwRasterLockP8RwRasterhi"));
    SET_TO(RwImageFindRasterFormat, aml->GetSym(hGTASA, "_Z23RwImageFindRasterFormatP7RwImageiPiS1_S1_S1_"));
    SET_TO(RwImageDestroy, aml->GetSym(hGTASA, "_Z14RwImageDestroyP7RwImage"));
    SET_TO(RwRasterSetFromImage, aml->GetSym(hGTASA, "_Z20RwRasterSetFromImageP8RwRasterP7RwImage"));
    SET_TO(RtPNGImageRead, aml->GetSym(hGTASA, "_Z14RtPNGImageReadPKc"));
    SET_TO(RtBMPImageRead, aml->GetSym(hGTASA, "_Z14RtBMPImageReadPKc"));
    SET_TO(CSprite2d_DrawSprite, aml->GetSym(hGTASA, "_ZN9CSprite2d4DrawERK5CRectRK5CRGBA"));
    SET_TO(GetVehicleRef, aml->GetSym(hGTASA, "_ZN6CPools13GetVehicleRefEP8CVehicle"));
    SET_TO(GetPedRef, aml->GetSym(hGTASA, "_ZN6CPools9GetPedRefEP4CPed"));
    SET_TO(GetVehicleFromRef, aml->GetSym(hGTASA, "_ZN6CPools10GetVehicleEi"));
    SET_TO(GetPedFromRef, aml->GetSym(hGTASA, "_ZN6CPools6GetPedEi"));
    SET_TO(TransformRealWorldPointToRadarSpace, aml->GetSym(hGTASA, "_ZN6CRadar35TransformRealWorldPointToRadarSpaceER9CVector2DRKS0_"));
    SET_TO(TransformRadarPointToScreenSpace, aml->GetSym(hGTASA, "_ZN6CRadar32TransformRadarPointToScreenSpaceER9CVector2DRKS0_"));
    SET_TO(LimitRadarPoint, aml->GetSym(hGTASA, "_ZN6CRadar15LimitRadarPointER9CVector2D"));

    SET_TO(userPaused, aml->GetSym(hGTASA, "_ZN6CTimer11m_UserPauseE"));
    SET_TO(codePaused, aml->GetSym(hGTASA, "_ZN6CTimer11m_CodePauseE"));
    SET_TO(camera, aml->GetSym(hGTASA, "TheCamera"));
    SET_TO(pPedPool, aml->GetSym(hGTASA, "_ZN6CPools11ms_pPedPoolE"));
    SET_TO(pVehiclePool, aml->GetSym(hGTASA, "_ZN6CPools15ms_pVehiclePoolE"));
    SET_TO(RsGlobal, aml->GetSym(hGTASA, "RsGlobal"));
    SET_TO(m_snTimeInMilliseconds, aml->GetSym(hGTASA, "_ZN6CTimer22m_snTimeInMillisecondsE"));
    SET_TO(CTouchInterface_m_bTouchDown, aml->GetSym(hGTASA, "_ZN15CTouchInterface12m_bTouchDownE"));
    SET_TO(m_vecCachedPos, aml->GetSym(hGTASA, "_ZN15CTouchInterface14m_vecCachedPosE"));

    HOOK(PreRenderEnd, aml->GetSym(hGTASA, "_ZN6CDebug22DebugDisplayTextBufferEv"));
    HOOK(CGame_Process, aml->GetSym(hGTASA, "_ZN5CGame7ProcessEv"));
    HOOK(InitRenderWare, aml->GetSym(hGTASA, "_ZN5CGame20InitialiseRenderWareEv"));
    HOOK(PostRadarDraw, aml->GetSym(hGTASA, "_ZN6CRadar20DrawRadarGangOverlayEb"));

    if(g_useOldTouchSystem == false)
        HOOK(AND_TouchEvent, aml->GetSym(hGTASA, "_Z14AND_TouchEventiiii"));
}

void DoLateHooks()
{
    fileLog->Log("Applying late hooks...");
}