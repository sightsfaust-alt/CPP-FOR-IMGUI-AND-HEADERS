#pragma once

#include "pch.h"

#include "WindowItem.h"
#include "menuSZK/IWindowItemOptions.h"
#include "Texture.h"

struct Option {
    std::string text;
    int value;
};

class WindowItemOptions : public WindowItem, public IWindowItemOptions {
private:
    std::vector<Option> options;
    int currentOptionIndex = 0;

    IContainer* containerOption;
public:
    float* pFloatValue = nullptr;
    int* pIntValue = nullptr;

    float min = -1000.0f;
    float max = 1000.0f;

    WindowItemOptions();
    ~WindowItemOptions();

    void UpdateVisuals();

    // ImGui rendering accessors
    const std::vector<Option>& GetOptions() const { return options; }
    int GetCurrentOptionIndex() const { return currentOptionIndex; }

    void InitOptions();

    const Option* GetCurrentOption() const; 
    std::string GetCurrentOptionText() const;

    void ClickLeft();
    void ClickRight();

    //
    
    void AddOption(std::string text, int value) override;
    int GetCurrentOptionValue() override;
    void SetOptionIndex(int index) override;
};