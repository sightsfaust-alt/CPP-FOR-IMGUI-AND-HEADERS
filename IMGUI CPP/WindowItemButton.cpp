#include "WindowItemButton.h"

#include "EventListener.h"

WindowItemButton::WindowItemButton() : WindowItem()
{
    onClick = new EventListener<>();

    container->onClick->Add([this]() {
        onClick->Emit();
    });
}

WindowItemButton::~WindowItemButton() = default;