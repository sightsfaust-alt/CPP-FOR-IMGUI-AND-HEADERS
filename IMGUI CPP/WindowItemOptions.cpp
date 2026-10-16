#include "WindowItemOptions.h"

#include "Container.h"
#include "Render.h"
#include "EventListener.h"

Texture* textureLeft = nullptr;
Texture* textureRight = nullptr;

WindowItemOptions::WindowItemOptions() : WindowItem()
{
    onOptionChange = new EventListener();

    if(!textureLeft)
        textureLeft = new Texture(modData->GetFileFromAssets("left.png"));

    if(!textureRight)
        textureRight = new Texture(modData->GetFileFromAssets("right.png"));

    container->highlightColor = CRGBA(0, 0, 0, 0);
}

WindowItemOptions::~WindowItemOptions()
{
    
}

void WindowItemOptions::UpdateVisuals()
{
    if(containerOption == nullptr) return;

    if(!options.empty())
    {
        containerOption->text = GetCurrentOptionText();
    } else if (pIntValue != nullptr)
    {
        containerOption->text = std::to_string(*pIntValue);
    } else if (pFloatValue != nullptr)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << *pFloatValue;
        containerOption->text = oss.str();
    } else {
        containerOption->text = "null";
    }
}

void WindowItemOptions::InitOptions()
{
    {
        auto container = new Container("option");
        container->fillHorizontal = true;
        container->fillVertical = true;
        container->horizontalAlign = HorizontalAlign::Middle;
        container->textHorizontalAlign = HorizontalAlign::Middle;
        container->textVerticalAlign = VerticalAlign::Middle;
        container->drawBackground = false;
        actionsContainer->AddChild(container);

        container->text = "VALUE";

        container->textFont.fontSize = 1.6f;
        container->textFont.align = eFontAlignment::ALIGN_CENTER;

        containerOption = container;
    }

    {
        auto button = new Container("left");
        button->size = CVector2D(90, 90);
        button->drawBackground = false;
        button->horizontalAlign = HorizontalAlign::Left;
        button->onClick->Add([this]() {
            ClickLeft();
        });
        actionsContainer->AddChild(button);

        container->onAfterDraw->Add([button]() {

            auto drawSize = button->size;
            drawSize.x *= 0.5f;
            drawSize.y *= 0.5f;

            auto drawPos = button->cachedPosition;
            drawPos.x += button->size.x / 2;
            drawPos.y += button->size.y / 2;

            drawPos.x -= drawSize.x / 2;
            drawPos.y -= drawSize.y / 2;
            
            if(textureLeft)
            {
                CRGBA color = button->isAnyPointerOver ? CRGBA(110, 110, 110) : COLOR_WHITE;

                Render::DrawSprite(textureLeft, drawPos, drawSize, color);
            }
        });
    }
   
    {
        auto button = new Container("right");
        button->size = CVector2D(90, 90);
        button->drawBackground = false;
        button->horizontalAlign = HorizontalAlign::Right;
        button->onClick->Add([this]() {
            ClickRight();
        });
        actionsContainer->AddChild(button);

        container->onAfterDraw->Add([button]() {

            auto drawSize = button->size;
            drawSize.x *= 0.5f;
            drawSize.y *= 0.5f;

            auto drawPos = button->cachedPosition;
            drawPos.x += button->size.x / 2;
            drawPos.y += button->size.y / 2;

            drawPos.x -= drawSize.x / 2;
            drawPos.y -= drawSize.y / 2;
            
            if(textureRight)
            {
                CRGBA color = button->isAnyPointerOver ? CRGBA(110, 110, 110) : COLOR_WHITE;

                Render::DrawSprite(textureRight, drawPos, drawSize, color);
            }
        });
    }

    UpdateVisuals();
}

const Option* WindowItemOptions::GetCurrentOption() const {
    if (options.empty() || currentOptionIndex < 0 || currentOptionIndex >= (int)options.size()) {
        return nullptr;
    }
    return &options[currentOptionIndex];
}

std::string WindowItemOptions::GetCurrentOptionText() const {
    auto opt = GetCurrentOption();
    return opt ? opt->text : "";
}

void WindowItemOptions::ClickLeft()
{
    if(!options.empty())
    {
        currentOptionIndex -= 1;
        if(currentOptionIndex < 0) currentOptionIndex = 0;
    } else if (pIntValue != nullptr)
    {
        *pIntValue -= 1;
        if (*pIntValue < min) *pIntValue = min;
    } else if (pFloatValue != nullptr)
    {
        *pFloatValue -= addBy;
    }

    UpdateVisuals();

    onOptionChange->Emit();
}

void WindowItemOptions::ClickRight()
{
    if(!options.empty())
    {
        currentOptionIndex += 1;
        if(currentOptionIndex > options.size() - 1) currentOptionIndex = options.size() - 1;
    } else if (pIntValue != nullptr)
    {
        *pIntValue += 1;
        if (*pIntValue > max) *pIntValue = max;
    } else if (pFloatValue != nullptr)
    {
        *pFloatValue += addBy;
    }

    UpdateVisuals();

    onOptionChange->Emit();
}

void WindowItemOptions::AddOption(std::string text, int value)
{
    options.push_back({ text, value });
    UpdateVisuals();
}

int WindowItemOptions::GetCurrentOptionValue()
{
    auto opt = GetCurrentOption();
    return opt ? opt->value : 0;
}

void WindowItemOptions::SetOptionIndex(int index)
{
    currentOptionIndex = index;
    
    onOptionChange->Emit();
}