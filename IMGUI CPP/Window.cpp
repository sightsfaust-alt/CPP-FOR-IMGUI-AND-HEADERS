#include "Window.h"

#include "Container.h"
#include "WindowItem.h"
#include "WindowItemCheckbox.h"
#include "WindowItemButton.h"
#include "WindowItemOptions.h"
#include "Debug.h"
#include "MenuSZK.h"

extern Container* rootContainer;

int maxItemsPerPage = 6;

Window::Window()
{

}

Window::~Window()
{
    menuDebug->AddLine("Destroy Window()");
    fileLog->Log("Deleting items...");

    delete mainContainer;

    for (auto item : items)
    {
        delete (WindowItem*)item;   // libera memória do objeto
    }
    items.clear();  // limpa o vetor
}

void Window::Init(float x, float y, float width)
{
    float subtitleHeight = 4.0f;

    {
        auto container = new Container("window");
        container->localOffset = CVector2D(x, y);
        container->size = CVector2D(width, 200);
        container->backgroundImage = new Texture(modData->GetFileFromAssets("bg.png"));
        container->canBeDragged = true;
        container->hideWhenPauseGame = true;
        container->text = title;
        container->textHorizontalAlign = HorizontalAlign::Middle;
        container->textVerticalAlign = VerticalAlign::Middle;
        rootContainer->AddChild(container);

        mainContainer = container;

        auto font = &container->textFont;
        font->style = eFontStyle::FONT_PRICEDOWN;
        font->align = eFontAlignment::ALIGN_CENTER;
        font->fontSize = 3.0f;
    }

    {
        auto container = new Container("subtitle");
        container->localOffset = CVector2D(0, 200);
        container->size = CVector2D(width, subtitleHeight);
        container->backgroundColor = CRGBA(0, 0, 0);
        container->text = "Subtitle goes here";
        container->textMarginLeft = 15.0f;
        container->textHorizontalAlign = HorizontalAlign::Left;
        container->textVerticalAlign = VerticalAlign::Middle;
        mainContainer->AddChild(container);

        subtitleContainer = container;

        auto font = &container->textFont;
        font->align = eFontAlignment::ALIGN_LEFT;
        font->color = CRGBA(67, 103, 150);
        font->fontSize = 1.6f;
    }

    float buttonSize = 90.0f;

    {
        auto button = new Container("left");
        button->size = CVector2D(buttonSize, buttonSize);
        button->drawBackground = true;
        button->localOffset = CVector2D(-buttonSize, 0);
        button->SetBackgroundImage(g_textureLeft);
        button->onClick->Add([this]() {

            audioChangePage->Play();
            MovePage(-1);
        });
        mainContainer->AddChild(button);
    }

    {
        auto button = new Container("right");
        button->size = CVector2D(buttonSize, buttonSize);
        button->drawBackground = true;
        button->horizontalAlign = HorizontalAlign::Right;
        button->localOffset = CVector2D(buttonSize, 0);
        button->SetBackgroundImage(g_textureRight);
        button->onClick->Add([this]() {

            audioChangePage->Play();
            MovePage(1);
        });
        mainContainer->AddChild(button);
    }
}

void Window::UpdateLayouts()
{
    float y = 200;

    y += subtitleContainer->size.y;

    int itemsPerPage = maxItemsPerPage;
    int totalItems = GetPaginatedItemCount();

     // total de páginas
    int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;

    // clamp da página atual
    if (currentPage >= totalPages) currentPage = totalPages - 1;
    if (currentPage < 0) currentPage = 0;

    // índice inicial e final da página
    int startIndex = currentPage * itemsPerPage;
    int endIndex = std::min(startIndex + itemsPerPage, totalItems);

    for (int i = 0; i < totalItems; i++)
    {
        WindowItem* item = (WindowItem*)items[i];

        if (i >= startIndex && i < endIndex)
        {
            // mostrar
            item->container->isVisible = true;
            item->container->size.x = mainContainer->size.x;
            item->container->size.y = item->height;
            item->container->localOffset.y = y;

            y += item->height;
        }
        else
        {
            // esconder
            item->container->isVisible = false;
        }
    }

    // Sticky back button (legacy path): always visible, positioned after content.
    if (stickyBackButton && !items.empty())
    {
        WindowItem* sticky = (WindowItem*)items.back();
        sticky->container->isVisible = true;
        sticky->container->size.x = mainContainer->size.x;
        sticky->container->size.y = sticky->height;
        sticky->container->localOffset.y = y;
    }
}

void Window::MovePage(int addBy)
{
    currentPage += addBy;

    int maxPages = GetMaxPages();

    if (currentPage < 0)
        currentPage = 0;
    else if (currentPage >= maxPages)
        currentPage = maxPages - 1;

    UpdateLayouts();
}

int Window::GetMaxPages()
{
    int itemsPerPage = maxItemsPerPage;
    int totalItems = GetPaginatedItemCount();

    if (totalItems == 0) return 1; // pelo menos 1 página "vazia"

    return (totalItems + itemsPerPage - 1) / itemsPerPage;
}

// Items participating in pagination. When stickyBackButton is set, the last
// item (the "< Back" button) is excluded — it renders outside pagination,
// always visible at the bottom of the window.
int Window::GetPaginatedItemCount() const
{
    int n = (int)items.size();
    if (stickyBackButton && n > 0) n--;
    return n;
}

IWindowItem* Window::AddText(std::string text)
{
    auto item = new WindowItem();
    item->InitTitle(text);

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return item;
}

IWindowItem* Window::AddCheckbox(std::string text, bool* pBool)
{
    auto item = new WindowItemCheckbox();
    item->InitTitle(text);
    item->InitActionsContainer(0.5f);

    item->pBoolValue = pBool;
    item->InitCheckbox();

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return item;
}

IWindowItemButton* Window::AddButton(std::string text)
{
    auto item = new WindowItemButton();
    item->InitTitle(text);

    item->onClick->Add([]() {
        audioSelect->Play();
    });

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return (IWindowItemButton*)item;
}

IWindowItemOptions* Window::AddOptions(std::string text)
{
    auto item = new WindowItemOptions();
    item->InitTitle(text);
    item->InitActionsContainer(0.5f);

    item->InitOptions();

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    // item->AddOption("Opcao 1", 1);
    // item->AddOption("Opcao 2", 2);
    // item->AddOption("Opcao 3", 3);
    // item->AddOption("Especial 5", 5);

    return (IWindowItemOptions*)item;
}

IWindowItemOptions* Window::AddFloatRange(std::string text, float* pFloat)
{
    auto item = new WindowItemOptions();
    item->InitTitle(text);
    item->InitActionsContainer(0.5f);

    item->pFloatValue = pFloat;
    item->InitOptions();

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return (IWindowItemOptions*)item;
}

IWindowItemOptions* Window::AddIntRange(std::string text, int* pInt, int min, int max)
{
    auto item = new WindowItemOptions();
    item->InitTitle(text);
    item->InitActionsContainer(0.5f);

    item->pIntValue = pInt;
    item->min = min;
    item->max = max;
    item->InitOptions();

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return (IWindowItemOptions*)item;
}


IWindowItem* Window::AddCustomItem(std::string text, float height)
{
    auto item = new WindowItem();
    item->height = height;

    mainContainer->AddChild(item->container);

    items.push_back(item);
    UpdateLayouts();

    return item;
}

void Window::Close()
{
    menuSZK->CloseWindow(this);
}
// ------------------------------------------------------------------
// Dear ImGui rendering — replaces the RW Container-based drawing.
// Each Window becomes one ImGui window; items map to ImGui widgets.
// ------------------------------------------------------------------
#include "imgui/imgui.h"

namespace
{
// Strip GTA text color codes like ~r~ ~y~ ~g~ ~b~ ~w~ etc.
std::string StripGtaColors(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '~' && i + 2 < s.size() && s[i + 2] == '~')
        {
            i += 2; // skip ~x~
            continue;
        }
        out += s[i];
    }
    return out;
}

std::string ItemTitle(IWindowItem* item)
{
    auto wi = (WindowItem*)item;
    if (wi && wi->titleContainer)
        return StripGtaColors(wi->titleContainer->text);
    return "";
}
} // namespace

void Window::ImGuiRender()
{
    std::string winTitle = StripGtaColors(title);
    if (winTitle.empty()) winTitle = "Menu";

    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(480, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(winTitle.c_str(), &open))
    {
        ImGui::End();
        if (!open) Close();
        return;
    }
    if (!open) { ImGui::End(); Close(); return; }

    // TASK2 (fixed): Title-bar "X" button — top-right, touch-friendly.
    // Uses defensive positioning: falls back to simple layout if content
    // region is unavailable. Uses onXButton if set, else Close().
    {
        float xBtnW = 64.0f;
        float xBtnH = 40.0f; // explicit height for touch target
        // Try right-align; clamp to valid range to avoid off-screen placement.
        float availW = ImGui::GetContentRegionAvail().x;
        float curX = ImGui::GetCursorPosX();
        float targetX = curX + availW - xBtnW;
        // Defensive: if availW looks bogus (<=0 or huge), just use SameLine flow.
        if (availW > xBtnW && availW < 5000.0f && targetX > curX)
        {
            ImGui::SetCursorPosX(targetX);
        }
        // Red X for visibility
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.35f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("X##close", ImVec2(xBtnW, xBtnH)))
        {
            if (onXButton) onXButton();
            else Close();
        }
        ImGui::PopStyleColor(3);
    }

    if (!subTitle.empty())
    {
        ImGui::TextDisabled("%s", StripGtaColors(subTitle).c_str());
        ImGui::Separator();
    }

    // Classic pagination with </> buttons.
    // When stickyBackButton is set, the last item ("< Back") is excluded
    // from pagination and rendered separately at the bottom (always visible).
    int itemsPerPage = maxItemsPerPage;
    int paginatedCount = GetPaginatedItemCount();
    int start = 0;
    int end = paginatedCount;

    {
        int maxPages = GetMaxPages();
        if (maxPages > 1)
        {
            if (ImGui::Button("<")) MovePage(-1);
            ImGui::SameLine();
            ImGui::Text(GetTranslatedText("page_indicator").c_str(), currentPage + 1, maxPages);
            ImGui::SameLine();
            if (ImGui::Button(">")) MovePage(1);
            ImGui::Separator();
        }
        start = currentPage * itemsPerPage;
        end = paginatedCount < start + itemsPerPage ? paginatedCount : start + itemsPerPage;
    }

    // Shared item renderer (used for paginated items and the sticky button).
    auto renderItem = [](IWindowItem* item, int index, const char* idSuffix)
    {
        std::string label = ItemTitle(item);
        // Unique ID per item to avoid ImGui ID collisions
        std::string id = label + "##item" + std::to_string(index) + idSuffix;

        if (auto btn = dynamic_cast<WindowItemButton*>(item))
        {
            if (ImGui::Button(id.c_str(), ImVec2(-1, 0)))
            {
                if (btn->onClick) btn->onClick->Emit();
            }
        }
        else if (auto chk = dynamic_cast<WindowItemCheckbox*>(item))
        {
            if (chk->pBoolValue)
                ImGui::Checkbox(id.c_str(), chk->pBoolValue);
            else
                ImGui::TextDisabled("%s", label.c_str());
        }
        else if (auto opt = dynamic_cast<WindowItemOptions*>(item))
        {
            const auto& options = opt->GetOptions();
            if (!options.empty())
            {
                // Discrete options -> combo box
                int cur = opt->GetCurrentOptionIndex();
                if (cur < 0) cur = 0;
                if (cur >= (int)options.size()) cur = (int)options.size() - 1;
                std::string preview = options[cur].text;
                if (ImGui::BeginCombo(id.c_str(), preview.c_str()))
                {
                    for (int n = 0; n < (int)options.size(); n++)
                    {
                        bool selected = (n == cur);
                        if (ImGui::Selectable(options[n].text.c_str(), selected))
                            opt->SetOptionIndex(n);
                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            else if (opt->pIntValue)
            {
                ImGui::SliderInt(id.c_str(), opt->pIntValue, (int)opt->min, (int)opt->max);
            }
            else if (opt->pFloatValue)
            {
                ImGui::SliderFloat(id.c_str(), opt->pFloatValue, opt->min, opt->max, "%.2f");
            }
            else
            {
                ImGui::TextDisabled("%s", label.c_str());
            }
        }
        else
        {
            // Plain text item
            ImGui::TextWrapped("%s", label.c_str());
        }
    };

    for (int i = start; i < end; ++i)
    {
        renderItem(items[i], i, "");
    }

    // Sticky back button: always visible at the bottom, outside pagination.
    if (stickyBackButton && !items.empty())
    {
        ImGui::Separator();
        renderItem(items.back(), (int)items.size() - 1, "_sticky");
    }

    ImGui::End();
}
