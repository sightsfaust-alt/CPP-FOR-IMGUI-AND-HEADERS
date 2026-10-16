#include "InputOLD.h"

#include "Input.h"

void InputOLD::Update()
{
    // Variável estática para guardar o estado anterior do toque
    static bool wasDown = false;

    // Pega o estado atual
    bool isDown = *CTouchInterface_m_bTouchDown;
    CVector2D pos = *m_vecCachedPos;

    int state = 0;

    if (isDown)
    {
        if (!wasDown)
            state = 2; // TOQUE INICIO
        else
            state = 3; // TOQUE MOVE
    }
    else
    {
        if (wasDown)
            state = 1; // TOQUE FIM
        else
            state = 0; // nenhum toque
    }

    wasDown = isDown;

    // Só processa se houver algum estado de toque
    if (state != 0)
    {
        int touchId = 0;            // sempre 0 para esse input antigo
        int x = (int)pos.x;
        int y = (int)pos.y;

        // Chama o sistema moderno de input
        Input::ProcessTouchEvent(state, touchId, x, y);
    }
}