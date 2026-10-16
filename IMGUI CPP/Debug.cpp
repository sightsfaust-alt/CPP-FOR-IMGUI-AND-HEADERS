#include "Debug.h"

#include "menuSZK/IFont.h"
#include "Render.h"

IFont font;

Debug::Debug()
{

}

void Debug::OnDraw()
{
    if(g_hasFirstUpdated)
    {
        if(!enabled) return;
    }

    float x = 20;
    float y = 400;

    font.align = eFontAlignment::ALIGN_LEFT;
    font.fontSize = 1.6f;
    font.color = COLOR_WHITE;

    for(auto line : lines)
    {
        font.color = line.color;

        Render::CurrentFont = font;
        Render::DrawText(line.text, CVector2D(x, y));

        y += 40;
    }
}

void Debug::AddLine(std::string text)
{
    AddLine(text, COLOR_WHITE);
}

void Debug::AddLine(std::string text, CRGBA color)
{
    IDebugLine line = {text, color};

    lines.push_back(line);

    fileLog->Debug(text);

    if (lines.size() >= 16) lines.erase(lines.begin());
}


void Debug::Clear()
{
    lines.clear();
}