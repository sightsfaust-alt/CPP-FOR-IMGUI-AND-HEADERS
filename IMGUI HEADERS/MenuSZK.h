#pragma once

#include "pch.h"

#include "menuSZK/IMenuSZK.h"

#include "Window.h"
#include "Widget.h"

class MenuSZK : public IMenuSZK {
public:
    MenuSZK();

    // ---- Global menu visibility (custom toggle button) ----
    // The menu starts hidden; a small floating button ("SZK") drawn via the
    // ImGui background draw list (no ImGui window/button) top-left toggles it.
    // Tap handling is in HookOf_AND_TouchEvent (hooks.cpp).
    bool menuVisible = false;
    void RenderToggleButton();
    // Toggle button rect (OS pixels) — must match RenderToggleButton().
    static constexpr float kToggleBtnX = 12.0f;
    static constexpr float kToggleBtnY = 12.0f;
    static constexpr float kToggleBtnW = 110.0f;
    static constexpr float kToggleBtnH = 60.0f;
    static bool IsPointOnToggleButton(float x, float y)
    {
        return x >= kToggleBtnX && x <= kToggleBtnX + kToggleBtnW &&
               y >= kToggleBtnY && y <= kToggleBtnY + kToggleBtnH;
    }

    // ---- Debug mode (settings.ini + in-menu toggle, persisted) ----
    // Bound to the in-menu checkbox. OnUpdate() detects changes,
    // applies them to debug->enabled and writes settings.ini.
    bool debugToggleValue = false;
    void SetDebugMode(bool enabled);
    void SaveSettings();

    void Initialize();
    void CreateIntroductionWidget();
    void OnUpdate();
    void OnGameUpdate();
    void OnDraw();
    void OnPostDrawRadar();

    void CloseWindow(Window* window);
    void DestroyWindowNow(Window* window);
    void FindWindowsToDestroy();

    void CloseWidget(Widget* widget);
    void DestroyWidgetNow(Widget* widget);
    void FindWidgetsToDestroy();
    
    void CreateTestWindow();

    // If the menu is opened but allWindows is empty (user closed every
    // window), there would be nothing to show -> recreate the test window.
    // Called whenever menuVisible flips to true.
    void EnsureMenuContent();

    void ForceUpdateRootContainer();

    void CreateChangeLanguageWindow();

    //

    IWindow* CreateWindow(float x, float y, float width, std::string title) override;
    IWidget* CreateWidget(float x, float y, float size, std::string bgImage, std::string image) override;

    IContainer* CreateContainer(std::string tag) override;
    IContainer* GetRootContainer() override;

    void SetGlobalIntVariable(const std::string& key, int value) override;
    int GetGlobalIntVariable(const std::string& key) override;

    void* GetCPedFromRef(int pedRef) override;
    void* GetCVehicleFromRef(int vehicleRef) override;
    
    void* LoadTexture(std::string src) override;
    void* CreateEmptyRwTexture(int width, int height, const std::string& textureName) override;
    void DrawTextureOnRadar(void* texture, CVector worldPosition, CRGBA color, float size) override;
    void DrawTextureOnWorld(void* texture, CVector worldPosition, CRGBA color, CVector2D size) override;

    CVector2D ConvertWorldPositionToScreenPosition(CVector worldPosition) override;

    std::string GetLocalizationsPath() override;
    void RegisterLocalizationFolder(std::string folderPath) override;
    std::string GetLocalizationText(std::string key) override;

    IAudio* LoadAudio(std::string src) override;
    IAudio* Load3DAudio(std::string src) override;
    void UnloadAudio(IAudio* audio) override;
};

extern MenuSZK* menuSZK;