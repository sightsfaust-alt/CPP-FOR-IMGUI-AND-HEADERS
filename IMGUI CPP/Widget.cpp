#include "Widget.h"

#include "Container.h"
#include "Render.h"
#include "Debug.h"
#include "MenuSZK.h"
#include "EventListener.h"

extern Container* rootContainer;

Widget::Widget()
{
    onClick = new EventListener<>();
}

Widget::~Widget()
{
    delete mainContainer;

    if(imageTexture)
    {
        delete imageTexture;
    }
}

void Widget::Init(float width, std::string bg, std::string image)
{
    if(image.length() > 0)
    {
        imageTexture = new Texture(image);
    }

    {
        auto container = new Container("widget");
        container->size = CVector2D(width, width);
        container->backgroundImage = new Texture(bg);
        container->hideWhenPauseGame = true;
        container->textHorizontalAlign = HorizontalAlign::Middle;
        container->textVerticalAlign = VerticalAlign::Middle;
        rootContainer->AddChild(container);

        container->textFont.align = eFontAlignment::ALIGN_CENTER;

        mainContainer = container;

        container->onAfterDraw->Add([this, container]() {

            if(!imageTexture) return;

            auto drawPos = container->cachedPosition;
            auto drawSize = container->size;

            CRGBA color = COLOR_WHITE;

            if(container->isAnyPointerOver)
            {
                color = CRGBA(0, 255, 0);
            } 

            Render::DrawSprite(imageTexture, drawPos, drawSize, color);
        });

        container->onClick->Add([this]() {
            menuDebug->AddLine("Clicked widget");
            fileLog->Log("[Widget] Container clicked, forwarding to Widget onClick");

            onClick->Emit();
        });
    }
}

void Widget::SetPosition(float x, float y)
{
    mainContainer->localOffset = CVector2D(x, y);
}

CVector2D Widget::GetSize()
{
    return mainContainer->size;
}

void Widget::Close()
{
    menuSZK->CloseWidget(this);
}