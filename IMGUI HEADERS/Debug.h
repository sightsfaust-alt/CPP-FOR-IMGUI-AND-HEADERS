#pragma once

#include "pch.h"

#include "menuSZK/IDebug.h"

class Debug : public IDebug {
public:
    Debug();

    void OnDraw();

    void AddLine(std::string text);
    void AddLine(std::string text, CRGBA color);
    void Clear();
};

inline Debug* localDebug;

#define menuDebug localDebug