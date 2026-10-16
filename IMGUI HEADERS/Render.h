#pragma once

#include "pch.h"

#include "menuSZK/IFont.h"
#include "Texture.h"

struct IResolution
{
    int width;
    int height;
};

class Render {
public:
    static IFont DefaultFont;
    static IFont CurrentFont;
    
    static IResolution GetBaseResolution();

    static float MapWidthToOS(float value);
    static float MapHeightToOS(float value);
    static float MapWidthFromOS(float value);
    static float MapHeightFromOS(float value);
    
    static void CheckSize();

    static void DrawText_original(std::string text, CVector2D position);
    static void DrawText(std::string text, CVector2D position);
    
    static void ResetFont();
    
    static void DrawRect_original(CRect rect, CRGBA color);
    static void DrawRect(CVector2D position, CVector2D size, CRGBA color);

    static void DrawSprite(Texture* texture, CVector2D position, CVector2D size);
    static void DrawSprite(Texture* texture, CVector2D position, CVector2D size, CRGBA color);

    static void DrawTextureOnRadar(Texture* texture, CVector worldPosition, CRGBA color, float size);
    static void DrawTextureOnWorld(Texture* texture, CVector worldPosition, CRGBA color, CVector2D size);
};