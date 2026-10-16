#pragma once

#include "pch.h"

#include "menuSZK/IWindow.h"
#include "menuSZK/IContainer.h"

class Window : public IWindow {
public:
    IContainer* mainContainer;
    IContainer* subtitleContainer;

    std::vector<IWindowItem*> items;

    int currentPage = 0;
    
    Window();
    ~Window();

    void Init(float x, float y, float width);

    void UpdateLayouts();

    void MovePage(int addBy);
    
    int GetMaxPages();

    // Number of items that participate in pagination.
    // When stickyBackButton is set, the last item is excluded (rendered
    // separately, always visible).
    int GetPaginatedItemCount() const;

    //

    IWindowItem* AddText(std::string text) override;
    IWindowItem* AddCheckbox(std::string text, bool* pBool) override;
    IWindowItemButton* AddButton(std::string text) override;
    IWindowItemOptions* AddOptions(std::string text) override;
    IWindowItemOptions* AddFloatRange(std::string text, float* pFloat) override;
    IWindowItemOptions* AddIntRange(std::string text, int* pInt, int min, int max) override;
    IWindowItem* AddCustomItem(std::string text, float height) override;

    void Close() override;

    // Dear ImGui rendering (replaces RW Container drawing)
    void ImGuiRender();
};