#pragma once

#include "menuSZK/IWindowItem.h"
#include "menuSZK/IContainer.h"

class WindowItem : public IWindowItem {
public:
    IContainer* actionsContainer = NULL;

    void InitTitle(std::string title);
    void InitActionsContainer(float horizontalFillPercentage);

    WindowItem();
    virtual ~WindowItem();

    //

    void SetTitle(std::string title);
};