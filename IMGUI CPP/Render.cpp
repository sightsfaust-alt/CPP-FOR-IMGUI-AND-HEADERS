#include "Render.h"

#include "MenuSZK.h"

char s_buffer[0xFF];
unsigned short* gxt_text = new unsigned short[0xFF];

IFont Render::DefaultFont;
IFont Render::CurrentFont = Render::DefaultFont;

IResolution osResolution = {0, 0};
IResolution baseResolution = {2400, 1080};

IResolution Render::GetBaseResolution()
{
    return baseResolution;
}

float Render::MapWidthToOS(float value)
{
    float mappedW = (value / baseResolution.width) * osResolution.width;
    return mappedW;
}

float Render::MapHeightToOS(float value)
{
    float mappedH = (value / baseResolution.height) * osResolution.height;
    return mappedH;
}

float Render::MapWidthFromOS(float value)
{
    return (value / osResolution.width) * baseResolution.width;
}

float Render::MapHeightFromOS(float value)
{
    return (value / osResolution.height) * baseResolution.height;
}

void Render::CheckSize()
{
    int width = OS_ScreenGetWidth();
    int height = OS_ScreenGetHeight();

    if(width != osResolution.width || height != osResolution.height)
    {
        osResolution.width = width;
        osResolution.height = height;

        fileLog->Log("screen size changed!");
        fileLog->Log("width: " + std::to_string(osResolution.width));
        fileLog->Log("height: " + std::to_string(osResolution.height));
    }
}

void Render::DrawText_original(std::string text, CVector2D position)
{
    auto font = &CurrentFont;

    auto scale = font->fontSize;

    FontSetOrientation(font->align);
    FontSetColor(&font->color);
    FontSetBackground(false, false);
    FontSetWrapx(500.0f);
    FontSetScale(scale, scale);
    FontSetStyle(font->style);
    FontSetProportional(true);
    FontSetDropShadowPosition(1);
    FontSetDropColor(&COLOR_BLACK);

    sprintf(s_buffer, "%s", text.c_str());
    AsciiToGxtChar(s_buffer, gxt_text);
    FontPrintString(position.x, position.y, gxt_text);

    RenderFontBuffer();
}

void Render::DrawText(std::string text, CVector2D position)
{
    auto font = &CurrentFont;

    position.x = MapWidthToOS(position.x);
    position.y = MapHeightToOS(position.y);

    // quanto maior o magic number, mais pra baixo o texto fica
    float lineHeight = font->fontSize * 22.0f;
    

    position.y -= MapHeightToOS(lineHeight) / 2;

    float scaleX = font->fontSize * (float)osResolution.width / (float)baseResolution.width;
    float scaleY = font->fontSize * (float)osResolution.height / (float)baseResolution.height;

    FontSetOrientation(font->align);
    FontSetColor(&font->color);
    FontSetBackground(false, false);
    FontSetWrapx(3000.0f);
    FontSetScale(scaleX, scaleY);
    FontSetStyle(font->style);
    FontSetProportional(true);
    FontSetDropShadowPosition(1);
    FontSetDropColor(&COLOR_BLACK);

    sprintf(s_buffer, "%s", text.c_str());
    AsciiToGxtChar(s_buffer, gxt_text);
    FontPrintString(position.x, position.y, gxt_text);

    RenderFontBuffer();
}

void Render::ResetFont()
{
    CurrentFont = DefaultFont;
}

void Render::DrawRect_original(CRect rect, CRGBA color)
{
    CSprite2d_DrawRect(rect, color);
}

void Render::DrawRect(CVector2D position, CVector2D size, CRGBA color)
{
    float mappedX = MapWidthToOS(position.x);
    float mappedY = MapHeightToOS(position.y);

    float mappedW = MapWidthToOS(size.x);
    float mappedH = MapHeightToOS(size.y);

    CRect rect(mappedX, mappedY, mappedX + mappedW, mappedY + mappedH);
    DrawRect_original(rect, color);
}

void Render::DrawSprite(Texture* texture, CVector2D position, CVector2D size)
{
    DrawSprite(texture, position, size, COLOR_WHITE);
}

void Render::DrawSprite(Texture* texture, CVector2D position, CVector2D size, CRGBA color)
{
    if(texture == nullptr)
    {
        fileLog->Error("DrawSprite: texture is null");
        return;
    }

    auto sprite = &texture->sprite;

    float mappedX = MapWidthToOS(position.x);
    float mappedY = MapHeightToOS(position.y);

    float mappedW = MapWidthToOS(size.x);
    float mappedH = MapHeightToOS(size.y);

    // if (centered)
    // {
    //     mappedX -= mappedW * 0.5f;
    //     mappedY -= mappedH * 0.5f;
    // }
    
    if(sprite->m_pTexture == nullptr)
    {
        color.a = 100;

        DrawRect_original(CRect(
            mappedX,             // esquerda
            mappedY,             // topo
            mappedX + mappedW,   // direita
            mappedY + mappedH    // fundo
        ), color);
        return;
    } 

    CSprite2d_DrawSprite(
        sprite,
        CRect(
            mappedX,             // esquerda
            mappedY,             // topo
            mappedX + mappedW,   // direita
            mappedY + mappedH    // fundo
        ),
        color
    );
}

void Render::DrawTextureOnRadar(Texture* texture, CVector worldPosition, CRGBA color, float size)
{
    if (!texture || !texture->sprite.m_pTexture || !texture->sprite.m_pTexture->raster)
        return;

    bool paused = IsGamePaused();
    CVector2D pos(worldPosition.x, worldPosition.y);

    TransformRealWorldPointToRadarSpace(pos, pos);

    LimitRadarPoint(pos);

    TransformRadarPointToScreenSpace(pos, pos);

    if (paused)
        pos *= flMenuMapScaling; // 👈 escala do mapa no menu pausado

    CRect rect(pos.x - size * 0.5f, pos.y - size * 0.5f, pos.x + size * 0.5f, pos.y + size * 0.5f);

    // RwRenderStateSet(rwRENDERSTATETEXTURERASTER, texture->sprite.m_pTexture->raster);
    // RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, TRUE);
    // RwRenderStateSet(rwRENDERSTATESRCBLEND, rwBLENDSRCALPHA);
    // RwRenderStateSet(rwRENDERSTATEDESTBLEND, rwBLENDINVSRCALPHA);

    CSprite2d_DrawSprite(&texture->sprite, rect, color);
}

void Render::DrawTextureOnWorld(Texture* texture, CVector worldPosition, CRGBA color, CVector2D size){
    if(IsGamePaused())
        return;

    float actualScale = 1;

    if(HasScriptCommands())
    {
        auto playerActor = GET_PLAYER_ACTOR(0);
        CVector playerPosition;

        STORE_COORDS_FROM_ACTOR_WITH_OFFSET(playerActor, 0, 0, 0, &playerPosition.x, &playerPosition.y, &playerPosition.z);

        // Distância até o blip
        float distance = distanceBetweenPoints(playerPosition, worldPosition);

        const float maxDistance = 50.0f;
        if (distance > maxDistance)
            return;

        // Escala diminui com a distância (0 perto, 40 longe)
        float t = distance / maxDistance;          // 0 → perto, 1 → longe
        
        float scale = 1.0f - t;                    // 1.0 → perto, 0 → longe
        scale = std::clamp(scale, 0.2f, 1.0f);     // limita o tamanho mínimo

        actualScale = actualScale * scale;
    }

    auto actualSize = CVector2D(
        size.x * actualScale,
        size.y * actualScale
    );

    auto screenPos = menuSZK->ConvertWorldPositionToScreenPosition(worldPosition);
    screenPos.x -= actualSize.x / 2;
    screenPos.y -= actualSize.y / 2;

    DrawSprite(texture, screenPos, actualSize, color);
}