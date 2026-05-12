#include "menu.h"

#define RGB(r, g, b) (Color){r, g, b, 255}

void desenha_botao(Rectangle ret, const char *texto, Color cor_base, Color cor_hover) {
    Vector2 mouse = GetMousePosition();
    int hover = CheckCollisionPointRec(mouse, ret);
    Color cor = hover ? cor_hover : cor_base;

    DrawRectangleRec(ret, cor);
    DrawRectangleLinesEx(ret, 2, RGB(30, 100, 40));

    int tam_txt = MeasureText(texto, 22);
    DrawText(texto,
             (int)(ret.x + ret.width / 2 - tam_txt / 2),
             (int)(ret.y + ret.height / 2 - 11),
             22, WHITE);
}

int foi_clicado(Rectangle ret) {
    return CheckCollisionPointRec(GetMousePosition(), ret) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
