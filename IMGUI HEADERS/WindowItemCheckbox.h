#pragma once

#include "WindowItem.h"
#include "Texture.h"

class WindowItemCheckbox : public WindowItem {
public:
    bool* pBoolValue = nullptr;

    WindowItemCheckbox();
    ~WindowItemCheckbox();

    void InitCheckbox();
};