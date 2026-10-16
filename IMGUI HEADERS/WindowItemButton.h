#pragma once

#include "WindowItem.h"
#include "menuSZK/IWindowItemButton.h"
#include "Texture.h"

class WindowItemButton : public WindowItem, public IWindowItemButton {
public:
    WindowItemButton();
    ~WindowItemButton();
};