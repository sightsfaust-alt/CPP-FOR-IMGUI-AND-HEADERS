#include "Container.h"

#include "Render.h"
#include "Input.h"
#include "Debug.h"
#include "EventListener.h"

bool drawTagText = false;

// See Container.h: guards tree mutations vs concurrent traversal from the
// touch-event thread.
std::recursive_mutex g_containerTreeMutex;

Container::Container(std::string tag)
{
    this->tag = tag;

    onClick = new EventListener<>();
    onBeforeDraw = new EventListener<>();
    onAfterDraw = new EventListener<>();
}

Container::~Container()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);

    // Invalidate any Input caches pointing at this container BEFORE it is
    // freed, so the touch thread never dereferences a dangling pointer.
    Input::OnContainerDestroyed(this);

    if(parent)
    {
        parent->RemoveChild(this, false);
    }

    // Deleta todas as children antes de deletar este container
    for (auto child : children)
    {
        child->parent = nullptr;
        delete child;
    }
    children.clear();
}

void Container::ProcessPositionAndSize()
{
    CVector2D parentPosition(0, 0);

    auto screenResolution = Render::GetBaseResolution();

    CVector2D parentSize(screenResolution.width, screenResolution.height);

    if (parent) {
        parentPosition = ((Container*)parent)->cachedPosition;
        parentSize = parent->size;
    }

    // ajusta tamanho caso precise preencher
    if (fillHorizontal) {
        size.x = parentSize.x * fillHorizontalPercentage;
    }
    if (fillVertical) {
        size.y = parentSize.y;
    }

    CVector2D thisPosition(0, 0);

    // alinhamento horizontal
    if (horizontalAlign == HorizontalAlign::Left) {
        thisPosition.x = parentPosition.x;
    }
    if (horizontalAlign == HorizontalAlign::Middle) {
        thisPosition.x = parentPosition.x + (parentSize.x / 2.0f) - size.x / 2.0f;
    }
    if (horizontalAlign == HorizontalAlign::Right) {
        thisPosition.x = parentPosition.x + parentSize.x - size.x;
    }

    // alinhamento vertical
    if (verticalAlign == VerticalAlign::Top) {
        thisPosition.y = parentPosition.y;
    }
    if (verticalAlign == VerticalAlign::Middle) {
        thisPosition.y = parentPosition.y + (parentSize.y / 2.0f) - size.y / 2.0f;
    }
    if (verticalAlign == VerticalAlign::Bottom) {
        thisPosition.y = parentPosition.y + parentSize.y - size.y;
    }

    // offset local
    thisPosition.x += localOffset.x;
    thisPosition.y += localOffset.y;

    cachedPosition = thisPosition;
}

IContainer* Container::AddChild(IContainer* child)
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    child->parent = (IContainer*)this;
    children.push_back(std::move(child));
    return child;
}

void Container::RemoveChild(IContainer* child, bool destroy)
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        children.erase(it);   // remove do vetor
        child->parent = nullptr; // desfaz o vínculo
    }
    
    if(destroy)
    {
        delete ((Container*)child);
    }
}   

void Container::SetBackgroundImage(std::string src)
{
    backgroundImage = new Texture(src);
}

void Container::SetBackgroundImage(void* texture)
{
    backgroundImage = (Texture*)texture;
}

void Container::Update()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    ProcessPositionAndSize();

    if(!IsVisible()) return;

    isAnyPointerOver = false;
    for(auto& [id, t] : Input::Touches)
    {
        if(IsPointInside(t.currentPos))
        {
            isAnyPointerOver = true;

            if(canBeDragged)
            {
                if(t.isDown && (t.delta.x != 0 || t.delta.y != 0))
                {
                    if(touchThatBeganDrag == -1)
                    {
                        touchThatBeganDrag = id;
                        menuDebug->AddLine("begin drag", CRGBA(0, 255, 0));
                    }
                }
            }
        }
    }

    
    if(touchThatBeganDrag != -1)
    {
        auto touch = &Input::Touches[touchThatBeganDrag];

        if(!touch->isDown)
        {
            touchThatBeganDrag = -1;
            menuDebug->AddLine("stop drag", CRGBA(255, 0, 0));
        } else {

            auto delta = touch->delta;

            localOffset.x += delta.x;
            localOffset.y += delta.y;
        }
    }

    auto snapshot = children;  // copia o vetor
    for (auto& child : snapshot)
    {
        ((Container*)child)->Update();
    }
}

void Container::Draw()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    if(!IsVisible()) return;

    onBeforeDraw->Emit();

    if(backgroundImage)
    {
        Render::DrawSprite(backgroundImage, cachedPosition, size);
    } else {
        // fundo
        if(drawBackground)
        {
            Render::DrawRect(
                cachedPosition,
                size,
                backgroundColor
            );
        }
    }

    bool drawBorder = Input::GetTouchThatContainerIsOn(this) != -1;

    if(drawBorder && menuDebug->enabled)
    {
        float borderSize = 2.0f;
        CRGBA borderColor(255, 255, 0, 255); // amarelo

        float x = cachedPosition.x;
        float y = cachedPosition.y;
        float w = size.x;
        float h = size.y;
        float b = borderSize;

        // Borda superior
        Render::DrawRect(CVector2D(x, y), CVector2D(w, b), borderColor);

        // Borda inferior
        Render::DrawRect(CVector2D(x, y + h - b), CVector2D(w, b), borderColor);

        // Borda esquerda
        Render::DrawRect(CVector2D(x, y), CVector2D(b, h), borderColor);

        // Borda direita
        Render::DrawRect(CVector2D(x + w - b, y), CVector2D(b, h), borderColor);
    }

    if(text.length() > 0)
    {
        Render::CurrentFont = textFont;

        // calcula posição base
        CVector2D textPos = cachedPosition;


        // alinhamento horizontal
        if (textHorizontalAlign == HorizontalAlign::Middle)
            textPos.x += size.x / 2;
        else if (textHorizontalAlign == HorizontalAlign::Right)
            textPos.x += size.x;
        else if (textHorizontalAlign == HorizontalAlign::Left)
        {
            textPos.x += textMarginLeft;
        }

        // alinhamento vertical
        if (textVerticalAlign == VerticalAlign::Middle)
            textPos.y += size.y / 2;
        else if (textVerticalAlign == VerticalAlign::Bottom)
            textPos.y += size.y;

        // desenha o texto
        Render::CurrentFont = textFont;
        Render::DrawText(text, textPos);

    } else {
        
        // tag
        if(drawTagText)
        {
            Render::ResetFont();
            Render::CurrentFont.align = eFontAlignment::ALIGN_CENTER;
            Render::DrawText(tag, cachedPosition + CVector2D(size.x / 2, size.y / 2));
        }
    }

    onAfterDraw->Emit();

    if(highlightColor.a > 0 && isAnyPointerOver)
    {
        Render::DrawRect(cachedPosition, size, highlightColor);
    }

    auto snapshot = children;  // copia o vetor
    for (auto& child : snapshot)
    {
        ((Container*)child)->Draw();
    }
}

bool Container::IsPointInside(const CVector2D& point)
{
    return (
        point.x >= cachedPosition.x &&
        point.y >= cachedPosition.y &&
        point.x <= cachedPosition.x + size.x &&
        point.y <= cachedPosition.y + size.y
    );
}

bool Container::IsVisible()
{
    if(!isVisible) return false;

    if(hideWhenPauseGame && IsGamePaused())
    {
        return false;
    }

    return true;
}

std::vector<Container*> Container::GetAllChildrenRecursive()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    std::vector<Container*> result;

    for (auto child : children)
    {
        result.push_back((Container*)child);  // adiciona a child atual

        // adiciona todas as children recursivamente
        auto grandChildren = ((Container*)child)->GetAllChildrenRecursive();
        result.insert(result.end(), grandChildren.begin(), grandChildren.end());
    }

    return result;
}

extern Container* rootContainer;

Container* Container::GetContainerOverThatPoint(const CVector2D& point)
{
    // Whole traversal under the tree lock: the touch-event thread must never
    // observe a children vector while the game thread is mutating it
    // (window/widget close). Previously this raced and crashed with SIGSEGV
    // on a dangling child pointer (aml_crashlog-5.txt).
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);

    std::function<Container*(Container*, const CVector2D&, int)> findTop;
    findTop = [&](Container* c, const CVector2D& pt, int depth) -> Container*
    {
        // Defense in depth: null guard + depth limit against cycles.
        if (!c || depth > kMaxContainerFindDepth)
            return nullptr;

        // Se o container e seus filhos são totalmente clicáveis (ignoráveis), pula tudo
        if (c->clickThroughMode == ClickThroughMode::ClickThroughThisAndChildren)
            return nullptr;

        // Itera filhos de trás pra frente (topo -> fundo)
        for (auto it = c->children.rbegin(); it != c->children.rend(); ++it)
        {
            Container* child = (Container*)*it;
            if (!child)
                continue;
            if (!child->IsVisible())
                continue;

            Container* hitChild = findTop(child, pt, depth + 1);
            if (hitChild)
                return hitChild;

            // Se este container permite clique através dele, mas não dos filhos, ainda testa filhos acima
            if (child->clickThroughMode == ClickThroughMode::ClickThroughThisAndChildren)
                continue;

            CVector2D worldPos = child->cachedPosition;
            if (pt.x >= worldPos.x && pt.x <= worldPos.x + child->size.x &&
                pt.y >= worldPos.y && pt.y <= worldPos.y + child->size.y)
            {
                // Se for ClickThroughThisOnly, retorna só se nenhum filho foi encontrado
                if (child->clickThroughMode != ClickThroughMode::ClickThroughThisOnly)
                    return child;
            }
        }

        // Testa o próprio container
        CVector2D worldPos = c->cachedPosition;
        if (pt.x >= worldPos.x && pt.x <= worldPos.x + c->size.x &&
            pt.y >= worldPos.y && pt.y <= worldPos.y + c->size.y)
        {
            if (c->clickThroughMode == ClickThroughMode::CantClickThrough)
                return c;
        }

        return nullptr;
    };

    return findTop(rootContainer, point, 0);
}