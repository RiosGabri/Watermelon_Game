#include "menu.h"

bool foi_clicado(Botao b) {
    return CheckCollisionPointRec(GetMousePosition(), b.area) &&
           IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void desenha_botao(Botao b) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), b.area);
    Texture2D tex = hover ? b.textura_hover : b.textura;


    DrawTexturePro(
        tex,
        (Rectangle){0, 0, (float)tex.width, (float)tex.height},
        b.area,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}