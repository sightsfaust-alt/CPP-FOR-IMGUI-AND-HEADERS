#include "Introduction.h"

#include "MenuSZK.h"
#include "Localization.h"

std::vector<std::string> g_introductionLines;

void Introduction::LoadLines()
{
    g_introductionLines.clear();

    // Language-aware: try introduction_<lang>.txt first (e.g. introduction_id.txt),
    // fall back to the legacy introduction.txt.
    std::string filePath = modData->GetFile("introduction_" + Localization::currentLanguage + ".txt");
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        filePath = modData->GetFile("introduction.txt");
        file.open(filePath);
    }
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
            g_introductionLines.push_back(line);
    }

    file.close();
}

void Introduction::ShowWindow()
{
    LoadLines();

    auto window = menuSZK->CreateWindow(400, 200, 800, GetTranslatedText("introduction"));

    window->AddText(GetTranslatedText("menu_author") + ": ~y~DaniloSZK");

    for(auto line : g_introductionLines)
    {
        window->AddText(line);  
    }

    //window->AddText(GetTranslatedText("data_author"));

    {
        auto button = window->AddButton("~r~" + GetTranslatedText("close"));
        button->onClick->Add([window]() {
            window->Close();
        });
    }
}