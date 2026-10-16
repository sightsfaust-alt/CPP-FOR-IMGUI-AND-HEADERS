#pragma once

#include "pch.h"

#include "menuSZK/IWidget.h"
#include "menuSZK/IContainer.h"

#include "Texture.h"

class Widget : public IWidget {
public:
    IContainer* mainContainer;    

    Texture* imageTexture = nullptr;

    Widget();
    ~Widget();

    void Init(float width, std::string bg, std::string image);

    //

    void SetPosition(float x, float y) override;
    CVector2D GetSize() override;
    void Close() override;
};