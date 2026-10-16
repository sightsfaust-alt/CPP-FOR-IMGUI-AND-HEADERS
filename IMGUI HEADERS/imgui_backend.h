#pragma once
// Custom minimal Dear ImGui GLES backend for MenuSZK64.
// Bundled directly in the plugin (no external libDearImGui.so).
//
// Design decisions (anti-black-screen):
//  - Lazy init on first render call (GL context guaranteed valid inside draw hook)
//  - FULL GL state save/restore around ImGui rendering (game state untouched)
//  - GLES2-compatible API subset (works on ES2 and ES3 contexts)
//  - No VAOs (ES2 compatible), VBOs with DYNAMIC_DRAW

#include "imgui.h"

namespace ImGuiBackend
{
    // Returns true when backend is initialized and ready.
    bool IsInitialized();

    // Human-readable status for on-screen diagnostics.
    const char* GetStatusString();

    // True when ImGui wants mouse input (touch over an ImGui window/widget).
    // Touch-thread safe (single-word read, may lag one frame).
    bool WantsMouseCapture();

    // Feed a touch event from AND_TouchEvent hook.
    // state: 2 = down, 3 = move, 1 = up (same as game). x/y in OS pixels.
    void ProcessTouch(int state, int touchId, float x, float y);

    // Begin a new ImGui frame. displayW/H in pixels (OS screen size).
    // Safe to call every frame; performs lazy GL init on first call.
    // Returns false if init failed (caller should skip ImGui UI).
    bool NewFrame(int displayW, int displayH, float deltaTime);

    // Render ImGui draw data with full GL state save/restore.
    void Render();

    // Optional cleanup (rarely needed on Android).
    void Shutdown();
}
