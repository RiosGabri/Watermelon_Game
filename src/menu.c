#include "menu.h"

bool foi_clicado(Botao b) {
    return CheckCollisionPointRec(GetMousePosition(), b.area) &&
           IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void desenha_botao(Botao b) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), b.area);
    Color cor = hover ? LIGHTGRAY : WHITE;

    DrawTexturePro(
        b.textura,
        (Rectangle){0, 0, (float)b.textura.width, (float)b.textura.height},
        b.area,
        (Vector2){0, 0},
        0.0f,
        cor
    );
}
