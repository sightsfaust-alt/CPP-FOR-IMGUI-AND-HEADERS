#include "WindowItem.h"

#include "Container.h"

extern Container* rootContainer;

WindowItem::WindowItem()
{
    container = new Container("itemContainer");
    container->backgroundColor = CRGBA(0, 0, 0, 120);
    container->highlightColor = CRGBA(255, 255, 255, 120);
}

WindowItem::~WindowItem()
{
    
}

void WindowItem::InitTitle(std::string title)
{
    auto mainContainer = container;

    {
        auto container = new Container("title");
        container->text = title;
        container->clickThroughMode = ClickThroughMode::ClickThroughThisOnly;
        container->drawBackground = false;
        container->fillHorizontal = true;
        container->fillVertical = true;
        container->textVerticalAlign = VerticalAlign::Middle;
        container->textHorizontalAlign = HorizontalAlign::Left;
        container->textMarginLeft = 20;

        auto font = &container->textFont;
        font->fontSize = 1.6f;

        mainContainer->AddChild(container);

        titleContainer = container;
    }
}

void WindowItem::InitActionsContainer(float horizontalFillPercentage)
{
    auto mainContainer = container;
    
    {
        auto container = new Container("actions");
        container->horizontalAlign = HorizontalAlign::Right;
        container->fillHorizontal = true;
        container->fillHorizontalPercentage = horizontalFillPercentage;
        container->fillVertical = true;
        container->drawBackground = false;

        mainContainer->AddChild(container);

        actionsContainer = container;
    }
}

void WindowItem::SetTitle(std::string title)
{
    if(!titleContainer) return;
    titleContainer->text = title;
}