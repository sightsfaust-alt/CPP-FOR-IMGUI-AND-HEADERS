#include "WindowItemCheckbox.h"

#include "Container.h"
#include "Render.h"

Texture* textureBg = nullptr;
Texture* textureOn = nullptr;

WindowItemCheckbox::WindowItemCheckbox() : WindowItem()
{
    if(!textureBg)
        textureBg = new Texture(modData->GetFileFromAssets("checkbox_off.png"));

    if(!textureOn)
        textureOn = new Texture(modData->GetFileFromAssets("checkbox_on.png"));
}

WindowItemCheckbox::~WindowItemCheckbox()
{
    
}

void WindowItemCheckbox::InitCheckbox()
{
    {
        auto container = new Container("checkbox");
        container->size = CVector2D(height, height);
        container->horizontalAlign = HorizontalAlign::Right;
        container->drawBackground = true;
        container->onClick->Add([this]() {
            *pBoolValue = !(*pBoolValue);
        });
        actionsContainer->AddChild(container);

        container->onAfterDraw->Add([this, container]() {
            if(pBoolValue == NULL) return;
            
            auto drawSize = container->size;
            drawSize.x *= 0.8f;
            drawSize.y *= 0.8f;

            auto drawPos = container->cachedPosition;
            drawPos.x += container->size.x / 2;
            drawPos.y += container->size.y / 2;

            drawPos.x -= drawSize.x / 2;
            drawPos.y -= drawSize.y / 2;
            
            if(textureBg)
            {
                Render::DrawSprite(textureBg, drawPos, drawSize);
            }

            if(textureOn && *pBoolValue)
            {
                Render::DrawSprite(textureOn, drawPos, drawSize);
            }
        });
    }
}