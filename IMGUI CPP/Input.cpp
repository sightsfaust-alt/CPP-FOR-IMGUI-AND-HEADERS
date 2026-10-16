#include "Input.h"

#include "Debug.h"
#include "Render.h"

std::unordered_map<int, TouchInfo> Input::Touches;

Container* containerToCallOnClick = nullptr;

void Input::OnContainerDestroyed(Container* c)
{
    // Caller (~Container) holds g_containerTreeMutex.
    if (containerToCallOnClick == c)
        containerToCallOnClick = nullptr;

    for (auto& [id, t] : Touches)
    {
        if (t.overContainer == c)
            t.overContainer = nullptr;
    }
}

void Input::Update()
{
    
}

void Input::OnGameUpdate()
{
    Container* c = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
        c = containerToCallOnClick;
        containerToCallOnClick = nullptr;
    }
    // Emit outside the lock: user onClick handlers may queue window closes
    // etc. The pointer was detached above while holding the lock, and only
    // this (game) thread destroys containers, so it stays valid here.
    if(c)
    {
        fileLog->Log("[Input] Emitting onClick for container: " + c->tag);
        c->onClick->Emit();
    }
}

void Input::Draw()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    if(menuDebug->enabled)
    {
        for (auto& [id, t] : Touches)
        {
            // t.pos é a posição do toque (CVector2D)
            float size = 15.0f;

            // calcula o canto superior esquerdo para centralizar
            CVector2D pos(
                t.currentPos.x - size / 2.0f,
                t.currentPos.y - size / 2.0f
            );

            CRGBA color = t.isDown ? CRGBA(255, 255, 255) : CRGBA(100, 100, 100);

            // desenha o retângulo
            Render::DrawRect(
                pos,
                CVector2D(size, size),
                color // amarelo
            );
        }
    }
}

void Input::ResetFlags()
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    // Reset flags de cada frame
    for(auto& [id, t] : Touches)
    {
        t.beganThisFrame = false;
        t.releasedThisFrame = false;
        //t.delta = CVector2D(0,0);
    }

    // Opcional: remover toques finalizados
    for(auto it = Touches.begin(); it != Touches.end(); )
    {
        if(!it->second.isDown)
            it = Touches.erase(it);
        else
            ++it;
    }
}

bool Input::ProcessTouchEvent(int state, int touchId, int x, int y)
{
    // Serialize against the game thread's tree mutations (window/widget
    // close). GetContainerOverThatPoint locks internally too (recursive).
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);

    x = Render::MapWidthFromOS(x);
    y = Render::MapHeightFromOS(y);

    if(state == 2) // TOQUE INICIO
    {
        TouchInfo t;
        t.touchId = touchId;
        t.startPos = CVector2D((float)x, (float)y);
        t.currentPos = t.startPos;
        t.delta = CVector2D(0, 0);
        t.isDown = true;
        t.beganThisFrame = true;
        t.releasedThisFrame = false;

        Touches[touchId] = t;
    }
    else if(state == 3) // TOQUE MOVE
    {
        auto it = Touches.find(touchId);
        if(it != Touches.end())
        {
            TouchInfo& t = it->second;
            t.delta = CVector2D((float)x, (float)y) - t.currentPos;
            t.currentPos = CVector2D((float)x, (float)y);
        }
    }
    else if(state == 1) // TOQUE FIM
    {
        auto it = Touches.find(touchId);
        if(it != Touches.end())
        {
            TouchInfo& t = it->second;
            t.isDown = false;
            t.releasedThisFrame = true;
            t.delta = CVector2D(0,0);
        }
    }

    for (auto it = Touches.begin(); it != Touches.end(); )
    {
        auto id = it->first;

        if (id > 10) // ID bugado
        {
            fileLog->Error("Touch id glitched again...");

            for (auto& [id, t] : Touches)
            {
                fileLog->Error("Found id (" + std::to_string(id) + ")");
            }

            it = Touches.erase(it); // erase retorna o próximo iterador
        }
        else
        {
            ++it;
        }
    }

    bool result = true;

    for (auto& [id, t] : Touches)
    {
        auto containerOverPoint = Container::GetContainerOverThatPoint(t.currentPos);
        t.overContainer = containerOverPoint;

        if(containerOverPoint && touchId == id)
        {
            result = false;

            if(state == 2)
            {
                menuDebug->AddLine("down at: " + containerOverPoint->tag);
                fileLog->Log("[Input] Touch down on container: " + containerOverPoint->tag +
                             " at (" + std::to_string((int)t.currentPos.x) + "," +
                             std::to_string((int)t.currentPos.y) + ")");
            }
            if(state == 1)
            {
                containerToCallOnClick = containerOverPoint;
                fileLog->Log("[Input] Touch up on container: " + containerOverPoint->tag +
                             " — onClick queued");
            }
        }
        else if (touchId == id && state == 1)
        {
            // Fallback: touch-up missed the container (finger drifted), but
            // touch-DOWN was on a container — use the start position.
            // This makes taps on small widgets (like the toggle) reliable.
            auto containerAtStart = Container::GetContainerOverThatPoint(t.startPos);
            if (containerAtStart)
            {
                containerToCallOnClick = containerAtStart;
                fileLog->Log("[Input] Touch up missed, but touch-down was on: " +
                             containerAtStart->tag + " — onClick queued (startPos fallback)");
                result = false;
            }
        }
    }

    return result;
}

CVector2D Input::GetTouchPosition(int touchId)
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    auto it = Touches.find(touchId);
    if (it != Touches.end())
    {
        return it->second.currentPos;
    }

    // se não existir, retorna uma posição inválida
    return CVector2D(-1.0f, -1.0f);
}

int Input::GetTouchThatContainerIsOn(Container* container)
{
    std::lock_guard<std::recursive_mutex> lock(g_containerTreeMutex);
    for(auto& [id, t] : Touches)
    {
        if(t.overContainer == container)
        {
            return id;
        }
    }

    return -1;
}