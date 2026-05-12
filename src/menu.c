#include "menu.h"

#define RGB(r, g, b) (Color){r, g, b, 255}

void desenha_botao(Botao b)
{
    Vector2 mouse = GetMousePosition();

    bool hover = CheckCollisionPointRec(mouse, b.area);

    Texture2D textura = hover
        ? b.hover
        : b.normal;

    DrawTexture(
        textura,
        (int)b.area.x,
        (int)b.area.y,
        WHITE
    );
}

bool foi_clicado(Botao b)
{
    return CheckCollisionPointRec(GetMousePosition(), b.area)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
