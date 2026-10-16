#include "MenuSZK.h"

#include "Audio.h"
#include "Container.h"
#include "Debug.h"
#include "DexTrainerUI.h"
#include "Entities.h"
#include "EventListener.h"
#include "IniReaderWriter.hpp"
#include "Input.h"
#include "InputOLD.h"
#include "Introduction.h"
#include "Localization.h"
#include "Render.h"
#include "Texture.h"
#include "Trainer.h"
#include "Widget.h"
#include "Window.h"
#include "imgLoader.h"
#include "imgui/imgui.h"
#include "imgui/imgui_backend.h"

Container* rootContainer = nullptr;

MenuSZK* menuSZK = new MenuSZK();

std::vector<Window*> allWindows;
std::vector<Window*> windowsToRemove;

std::vector<Widget*> allWidgets;
std::vector<Widget*> widgetsToRemove;

std::unordered_map<std::string, int> globalIntVariables;

bool hasPlayerBeenReady = false;
bool hasFirstUpdated = false;

int prevVehicle = 0;

MenuSZK::MenuSZK()
{
    localDebug = new Debug();
    debug = localDebug;

    onGameFirstUpdate = new EventListener<>();
    onGamePlayerReady = new EventListener<>();

    onBeforeMenuUpdate = new EventListener<>();
    onAfterMenuUpdate = new EventListener<>();

    onGameUpdate = new EventListener<>();

    onDrawBeforeMenu = new EventListener<>();
    onDrawAfterMenu = new EventListener<>();

    onPostDrawRadar = new EventListener<>();

    onPedAdded = new EventListener<int>();
    onPedRemoved = new EventListener<int>();
    onVehicleAdded = new EventListener<int>();
    onVehicleRemoved = new EventListener<int>();
}

void MenuSZK::Initialize()
{
    g_textureLeft = new Texture(modData->GetFileFromAssets("left.png"));
    g_textureRight = new Texture(modData->GetFileFromAssets("right.png"));

    rootContainer = new Container("rootContainer");
    rootContainer->horizontalAlign = HorizontalAlign::Left;
    rootContainer->verticalAlign = VerticalAlign::Top;
    rootContainer->fillHorizontal = true;
    rootContainer->fillVertical = true;
    rootContainer->clickThroughMode = ClickThroughMode::ClickThroughThisOnly;
    rootContainer->drawBackground = false;

#if defined(AML64)
    fileLog->Log("AML64 defined, so creating test window");
#endif

    // IMPORTANT: Load localization FIRST, before any window/widget is created.
    // Windows capture translated strings at creation time — if localization
    // isn't loaded yet, they permanently bake in "[MISSING:key]".
    RegisterLocalizationFolder(GetLocalizationsPath() + "dextrainer/");

    // NOTE: "menu_author" and all other UI strings now live in the
    // localization .ini files (en.ini / id.ini). Do not RegisterLine here.

    // NOTE: Test window permanently disabled. DexTrainerUI main window is
    // created on-demand via EnsureMenuContent() when the menu opens.

    // Trainer (RZL-like) — Phase 1. Binds game functions; UI renders via Trainer::Render().
    Trainer::Initialize();

    //if (!HasScriptCommands()) { CreateIntroductionWidget(); }
}

// Custom toggle button — floating top-left, always visible.
// Drawn DIRECTLY on the ImGui background draw list (no ImGui window/button:
// the Begin()+Button() version never became visible on device).
// Tap handling is done in HookOf_AND_TouchEvent (hooks.cpp), not via ImGui.
void MenuSZK::RenderToggleButton()
{
    auto* dl = ImGui::GetBackgroundDrawList();
    ImVec2 p0(kToggleBtnX, kToggleBtnY);
    ImVec2 p1(kToggleBtnX + kToggleBtnW, kToggleBtnY + kToggleBtnH);

    // Fill: dark when closed, blue tint when the menu is open.
    ImU32 colFill = menuVisible ? IM_COL32(41, 61, 92, 217)
                                : IM_COL32(26, 26, 36, 184);
    dl->AddRectFilled(p0, p1, colFill, 12.0f);
    dl->AddRect(p0, p1, IM_COL32(255, 255, 255, 70), 12.0f, 0, 1.5f);

    // Centered label, slightly enlarged for presence.
    const char* label = menuVisible ? "SZK [x]" : "SZK";
    ImFont* font = ImGui::GetFont();
    const float fontSize = 20.0f;
    ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
    dl->AddText(font, fontSize,
                ImVec2(p0.x + (kToggleBtnW - ts.x) * 0.5f,
                       p0.y + (kToggleBtnH - ts.y) * 0.5f),
                IM_COL32(255, 255, 255, 255), label);

    static bool firstRenderLogged = false;
    if (!firstRenderLogged)
    {
        firstRenderLogged = true;
        fileLog->Log("[MenuSZK] Draw-list toggle button first render");
    }
}

void MenuSZK::CreateIntroductionWidget()
{
    auto res = Render::GetBaseResolution();
    float size = 200.0f;

    auto widget = CreateWidget(res.width / 2 - size / 2,
        res.height / 2 - size / 2,
        size,
        modData->GetFileFromAssets("widget_background1.png"),
        modData->GetFileFromAssets("widget_start.png"));
    widget->onClick->Add(
        [widget]()
        {
            widget->Close();

            Introduction::ShowWindow();
        });
}

void MenuSZK::OnUpdate()
{
    if (GetGlobalIntVariable("open_menuszk") == 1)
    {
        SetGlobalIntVariable("open_menuszk", 0);
        DexTrainerUI::EnsureMainWindow();
    }

    if (GetGlobalIntVariable("menu_changelanguage") == 1)
    {
        SetGlobalIntVariable("menu_changelanguage", 0);
        CreateChangeLanguageWindow();
    }

    for (auto widget : allWidgets) { widget->mainContainer->isVisible = widget->visible; }

    // Debug-mode checkbox (in-menu toggle) -> apply + persist to settings.ini.
    if (debugToggleValue != debug->enabled)
    {
        SetDebugMode(debugToggleValue);
    }

    for (auto window : allWindows)
    {
        window->mainContainer->text = window->title;
        window->subtitleContainer->text = window->subTitle;
    }

    onBeforeMenuUpdate->Emit();

    if (rootContainer) { rootContainer->Update(); }

    if (g_useOldTouchSystem) { InputOLD::Update(); }

    Input::Update();

    FindWindowsToDestroy();
    FindWidgetsToDestroy();

    onAfterMenuUpdate->Emit();
}

void MenuSZK::OnGameUpdate()
{
    if (!g_hasFirstUpdated)
    {
        g_hasFirstUpdated = true;

        DoLateHooks();

        audioChangePage = LoadAudio(modData->GetFileFromAssets("audios/menu_change_page.wav"));
        audioSelect = LoadAudio(modData->GetFileFromAssets("audios/menu_select.wav"));
    }

    Entities::Update();

    Input::OnGameUpdate();

    // DexTrainer per-frame toggles (god mode, never wanted, ...)
    Trainer::Update();

    onGameUpdate->Emit();

    if (!hasFirstUpdated)
    {
        hasFirstUpdated = true;
        onGameFirstUpdate->Emit();
    }

    if (!hasPlayerBeenReady)
    {
        if (HasScriptCommands() && IS_PLAYER_PLAYING(0))
        {
            hasPlayerBeenReady = true;
            //CreateIntroductionWidget();

            onGamePlayerReady->Emit();
        }
    }
}

void MenuSZK::OnDraw()
{
    g_currentFrame++;

    if (!g_hasFirstDrawn)
    {
        g_hasFirstDrawn = true;
        // menuDebug->AddLine("[MenuSZK] Author: ~y~DaniloSZK");
        // menuDebug->AddLine("[MenuSZK] Language: " +
        // Localization::currentLanguage);

        if (debug->enabled) menuDebug->AddLine("[MenuSZK] ~g~Debug mode enabled");

        if (g_useOldTouchSystem) menuDebug->AddLine("[MenuSZK_v1] ~y~Using OLD touch system. Change if possible!");
    }

    if (g_imgLoaderEnabled)
    {
        if (g_currentFrame <= 10)
        {
            auto res = Render::GetBaseResolution();

            auto textPos = CVector2D(res.width / 2, res.height / 2);

            Render::DrawRect(CVector2D(0, 0), CVector2D(res.width, res.height), CRGBA(0, 0, 0, 150));

            Render::ResetFont();
            Render::CurrentFont.fontSize = 3.0f;
            Render::CurrentFont.align = eFontAlignment::ALIGN_CENTER;
            Render::CurrentFont.color = CRGBA(255, 255, 255);
            Render::DrawText(GetTranslatedText("loading_dffs"), textPos);

            if (g_currentFrame == 10) { RunIMGLoader(); }
        }
    }

    Textures::TryLoadTextures();

    auto debug = (Debug*)this->debug;

    Render::CheckSize();

    onDrawBeforeMenu->Emit();

    if (rootContainer) { rootContainer->Draw(); }

    debug->OnDraw();

    onDrawAfterMenu->Emit();

    Input::Draw();
    Input::ResetFlags(); // always on end of frame

    // test
    if (hasFirstUpdated && g_drawTestInfo)
    {
        Render::ResetFont();
        Render::CurrentFont.fontSize = 1.6f;

        // test
        auto car = ACTOR_USED_CAR(GET_PLAYER_ACTOR(0));

        if (car > 0)
        {
            auto model = GET_CAR_MODEL(car);

            Render::DrawText(GetTranslatedText("vehicle_model") + std::to_string(model), CVector2D(400, 400));
        }
        else
        {
            Render::DrawText(GetTranslatedText("no_vehicle"), CVector2D(400, 400));
        }
    }

    // ---- Dear ImGui (bundled) ----
    // Rendered AFTER the RW UI so it draws on top. Full GL state save/restore
    // inside ImGuiBackend::Render() keeps the game untouched.
    //
    // TAHAP 5: Windows are rendered via ImGui (Window::ImGuiRender).
    // Their RW containers are hidden to avoid double-drawing.
    // Widgets still use the RW Container system (image-based).
    {
        int scrW = OS_ScreenGetWidth ? OS_ScreenGetWidth() : 1280;
        int scrH = OS_ScreenGetHeight ? OS_ScreenGetHeight() : 720;
        float dt = deltaTime > 0 ? (deltaTime / 1000.0f) : (1.0f / 60.0f);

        bool newFrameOk = ImGuiBackend::NewFrame(scrW, scrH, dt);

        if (newFrameOk)
        {
            // Custom toggle button (draw-list) — always visible (it's how the
            // user shows/hides the menu). Rendered before the menu windows.
            RenderToggleButton();

            // Hide RW window containers (ImGui draws them now)
            {
                std::lock_guard<std::recursive_mutex> treeLock(g_containerTreeMutex);
                for (auto window : allWindows)
                {
                    if (window->mainContainer)
                        window->mainContainer->isVisible = false;
                }
            }

            if (menuVisible)
            {
                // Render each menu window as an ImGui window.
                // Copy the vector under lock: Close() during render and
                // EnsureMenuContent() from the touch thread mutate allWindows.
                std::vector<Window*> windowsCopy;
                {
                    std::lock_guard<std::recursive_mutex> treeLock(g_containerTreeMutex);
                    windowsCopy = allWindows;
                }
                for (auto window : windowsCopy)
                {
                    window->ImGuiRender();
                }

                // NOTE: DexTrainer now uses the Window API (DexTrainerUI).
                // The old direct-ImGui Trainer::Render() is disabled.

                // FAILSAFE (anti-trap): if the menu is open but nothing is
                // actually rendered for 120 consecutive frames, force-close it.
                // This guarantees the user can never be stuck in an
                // "open but invisible" state.
                static int emptyMenuFrames = 0;
                if (windowsCopy.empty())
                {
                    if (++emptyMenuFrames >= 120)
                    {
                        emptyMenuFrames = 0;
                        menuVisible = false;
                        fileLog->Log("[MenuSZK] WARNING: menu open with no windows for 120 frames — force-closed (anti-trap failsafe)");
                    }
                }
                else
                {
                    emptyMenuFrames = 0;
                }
            }

            ImGuiBackend::Render();
        }
    }
}

void MenuSZK::OnPostDrawRadar()
{
    onPostDrawRadar->Emit();
}

void MenuSZK::CloseWindow(Window* window)
{
    // Thread-safe — windowsToRemove is read by FindWindowsToDestroy
    // on the game thread while CloseWindow can be called from the touch thread
    // (onClick handlers). Without the lock: vector reallocation race → crash.
    //
    // DEDUP (2026-10-14): prevent double-close → double-free crash.
    // Root cause of Back-button force close: Back handler does
    //   CloseAllDexWindows() → OpenWeaponsWindow()
    // and OpenWeaponsWindow() calls CloseAllDexWindows() AGAIN internally.
    // The category window is still in allWindows (not yet destroyed), so it
    // gets pushed to windowsToRemove TWICE → DestroyWindowNow deletes it twice.
    // The mutex did not help — this is a logic bug, not a race.
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    if (std::find(windowsToRemove.begin(), windowsToRemove.end(), window)
        == windowsToRemove.end()) {
        windowsToRemove.push_back(window);
    }
}

void MenuSZK::DestroyWindowNow(Window* window)
{
    fileLog->Log("Destroying window " + window->title);

    allWindows.erase(std::remove(allWindows.begin(), allWindows.end(), window), allWindows.end());

    fileLog->Log("Deleting");

    delete window;

    fileLog->Log("Window destroyed!");
}

void MenuSZK::FindWindowsToDestroy()
{
    // BUG1 FIX: hold the lock for the entire destroy pass. DestroyWindowNow
    // erases from allWindows and deletes — must not race with CloseWindow
    // pushing to windowsToRemove from another thread.
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    if (windowsToRemove.empty()) return;

    for (auto window : windowsToRemove) { DestroyWindowNow(window); }

    windowsToRemove.clear();
}

void MenuSZK::CloseWidget(Widget* widget)
{
    widgetsToRemove.push_back(widget);
}

void MenuSZK::DestroyWidgetNow(Widget* widget)
{
    allWidgets.erase(std::remove(allWidgets.begin(), allWidgets.end(), widget), allWidgets.end());

    delete widget;
}

void MenuSZK::FindWidgetsToDestroy()
{
    if (widgetsToRemove.empty()) return;

    for (auto w : widgetsToRemove) { DestroyWidgetNow(w); }

    widgetsToRemove.clear();
}

void MenuSZK::ForceUpdateRootContainer()
{
    if (rootContainer) { rootContainer->Update(); }
}

void MenuSZK::SetDebugMode(bool enabled)
{
    debug->enabled = enabled;
    debugToggleValue = enabled;
    SaveSettings();
}

void MenuSZK::SaveSettings()
{
    // Persist the current runtime settings back to settings.ini so the
    // in-menu toggles (debug mode, language, ...) survive restarts.
    // Other keys are written back from their live values to avoid clobbering.
    auto pathSettings = modData->GetFile("settings.ini");

    IniReaderWriter ini;
    ini.LoadFromFile(pathSettings); // keep unknown keys if the file exists

    ini.SetBool("settings", "debug_mode", debug->enabled);
    ini.Set("settings", "language", Localization::currentLanguage);
    ini.SetBool("settings", "modloader_enabled", g_imgLoaderEnabled);
    ini.SetBool("settings", "use_old_touch_system", g_useOldTouchSystem);
    ini.SetBool("settings", "draw_test_info", g_drawTestInfo);

    if (!ini.SaveToFile(pathSettings))
    {
        fileLog->Error("Failed to save settings.ini");
    }
}

void MenuSZK::CreateChangeLanguageWindow()
{
    auto window = CreateWindow(500, 200, 800, GetTranslatedText("change_language"));
    window->AddText(GetTranslatedText("choose_language"));

    auto languages = Localization::GetLanguages();

    // Show friendly names where we know them; fall back to the raw code.
    auto friendlyName = [](const std::string& code) -> std::string {
        if (code == "en") return "English";
        if (code == "id") return "Indonesia";
        return code;
    };

    for (auto language : languages)
    {
        auto button = window->AddButton("- " + friendlyName(language));
        button->onClick->Add(
            [window, language]()
            {
                window->Close();

                Localization::currentLanguage = language;
            });
    }

    {
        auto button = window->AddButton(GetTranslatedText("close"));
        button->onClick->Add(
            [window]()
            {
                window->Close();
            });
    }
}

// If the menu is opened but allWindows is empty (user closed every window),
// there would be nothing to show -> recreate the test window so the menu
// never opens into an empty invisible state.
void MenuSZK::EnsureMenuContent()
{
    // Test window permanently disabled. Create DexTrainer main window instead.
    // (g_containerTreeMutex is recursive, so EnsureMainWindow may re-lock safely.)
    std::lock_guard<std::recursive_mutex> treeLock(g_containerTreeMutex);
    if (allWindows.empty())
    {
        fileLog->Log("[MenuSZK] No windows open — creating DexTrainer main window");
        DexTrainerUI::EnsureMainWindow();
    }
}

void MenuSZK::CreateTestWindow()
{    static bool bool1 = false;

    // Sync the debug checkbox with the current setting (from settings.ini).
    debugToggleValue = debug->enabled;

    auto window = CreateWindow(200, 200, 800, GetTranslatedText("test_window"));
    window->AddText(GetTranslatedText("test_text1"));

    window->AddCheckbox(GetTranslatedText("debug_mode"), &debugToggleValue);

    {
        auto button = window->AddButton(GetTranslatedText("test_play_audio"));
        button->onClick->Add([]() {

    });
    }

    window->AddCheckbox(GetTranslatedText("test_checkbox"), &bool1);

    {
        auto options = window->AddOptions(GetTranslatedText("test_options"));
        options->AddOption(GetTranslatedText("test_option") + " 1", 1);
        options->AddOption(GetTranslatedText("test_option") + " 2", 2);
        options->AddOption(GetTranslatedText("test_option") + " 25", 25);
    }

    {
        auto button = window->AddButton(GetTranslatedText("close"));
        button->onClick->Add(
            [window]()
            {
                window->Close();
            });
    }
}

IWindow* MenuSZK::CreateWindow(float x, float y, float width, std::string title)
{
    auto window = new Window();
    window->title = title;
    window->subTitle = "";

    window->Init(x, y, width);

    allWindows.push_back(window);

    return window;
}

IWidget* MenuSZK::CreateWidget(float x, float y, float size, std::string bgImage, std::string image)
{
    auto widget = new Widget();

    widget->Init(size, bgImage, image);
    widget->SetPosition(x, y);

    allWidgets.push_back(widget);

    return widget;
}

IContainer* MenuSZK::CreateContainer(std::string name)
{
    auto container = new Container(name);
    return (IContainer*)container;
}

IContainer* MenuSZK::GetRootContainer()
{
    return (IContainer*)rootContainer;
}

void MenuSZK::SetGlobalIntVariable(const std::string& key, int value)
{
    globalIntVariables[key] = value;
}

int MenuSZK::GetGlobalIntVariable(const std::string& key)
{
    auto it = globalIntVariables.find(key);
    if (it != globalIntVariables.end()) return it->second;
    return -1;
}

void* MenuSZK::GetCPedFromRef(int pedRef)
{
    return GetPedFromRef(pedRef);
}

void* MenuSZK::GetCVehicleFromRef(int vehicleRef)
{
    return GetVehicleFromRef(vehicleRef);
}

void* MenuSZK::LoadTexture(std::string src)
{
    auto texture = new Texture(src);
    return (void*)texture;
}

void* MenuSZK::CreateEmptyRwTexture(int width, int height, const std::string& textureName)
{
    return Textures::CreateEmptyTexture(width, height, textureName);
}

void MenuSZK::DrawTextureOnRadar(void* texture, CVector worldPosition, CRGBA color, float size)
{
    Render::DrawTextureOnRadar((Texture*)texture, worldPosition, color, size);
}

void MenuSZK::DrawTextureOnWorld(void* texture, CVector worldPosition, CRGBA color, CVector2D size)
{
    Render::DrawTextureOnWorld((Texture*)texture, worldPosition, color, size);
}

CVector2D MenuSZK::ConvertWorldPositionToScreenPosition(CVector worldPosition)
{
    RwV3d rwp = { worldPosition.x, worldPosition.y, worldPosition.z };
    RwV3d screenCoors;
    float w, h;

    CSprite_CalcScreenCoors(rwp, &screenCoors, &w, &h, true, true);

    screenCoors.x = Render::MapWidthFromOS(screenCoors.x);
    screenCoors.y = Render::MapHeightFromOS(screenCoors.y);

    return CVector2D(screenCoors.x, screenCoors.y);
}

std::string MenuSZK::GetLocalizationsPath()
{
    std::string dataRootPath = aml->GetAndroidDataRootPath();
    return dataRootPath + "/mods/DexTrainer/localization/";
}

void MenuSZK::RegisterLocalizationFolder(std::string folderPath)
{
    namespace fs = std::filesystem;

    fileLog->Log("[Localization] Loading from: " + folderPath);

    // garante que a pasta existe
    if (!fs::exists(folderPath))
    {
        fileLog->Log("[Localization] ERROR: folder not found: " + folderPath);
        menuDebug->AddLine("~r~[Localization] Could not find " + folderPath);
        return;
    }

    int filesLoaded = 0;

    // percorre todos os .ini dentro da pasta
    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (!entry.is_regular_file()) continue;

        auto path = entry.path();
        if (path.extension() != ".ini") continue;

        // nome do arquivo vira código do idioma, ex: "pt-br.ini" → "pt-br"
        std::string language = path.stem().string();

        IniReaderWriter ini;
        if (!ini.LoadFromFile(path.string()))
        {
            fileLog->Log("[Localization] ERROR: failed to parse " + path.string());
            continue;
        }

        auto data = *ini.GetData();

        int keysRegistered = 0;
        for (const auto& [sectionName, section] : data)
        {
            for (const auto& [key, value] : section)
            {
                std::string fullKey = sectionName.empty() ? key : (sectionName + "." + key);

                Localization::RegisterLine(language, fullKey, value);
                keysRegistered++;
            }
        }

        fileLog->Log("[Localization] Loaded " + path.filename().string() +
                     " (language=" + language + ", keys=" + std::to_string(keysRegistered) + ")");
        filesLoaded++;
    }

    fileLog->Log("[Localization] Done. Files loaded: " + std::to_string(filesLoaded) +
                 ", currentLanguage=" + Localization::currentLanguage);
}

std::string MenuSZK::GetLocalizationText(std::string key)
{
    return Localization::Get(key);
}

IAudio* MenuSZK::LoadAudio(std::string src)
{
    auto audio = new Audio();
    audio->LoadFromSource(src, false);

    return (IAudio*)audio;
}

IAudio* MenuSZK::Load3DAudio(std::string src)
{
    auto audio = new Audio();
    audio->LoadFromSource(src, true);

    return (IAudio*)audio;
}

void MenuSZK::UnloadAudio(IAudio* audio)
{
    auto realAudio = (Audio*)audio;
    realAudio->DestroyStream();

    delete realAudio;
}