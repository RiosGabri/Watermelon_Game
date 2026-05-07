#ifdef _WIN32
#define _STAT_DEFINED
#define stat    _stat64i32
#define fstat   _fstat64i32
#endif

#include "raylib.h"

#define JANELA_W 600
#define JANELA_H 700
#define RGB(r, g, b) (Color){r, g, b, 255}

static void desenharBotao(Rectangle botao, const char *texto, Color cor, Color corHover)
{
    Vector2 mouse = GetMousePosition();
    int     hover = CheckCollisionPointRec(mouse, botao);

    Color corAtual = hover ? corHover : cor;

    DrawRectangleRec(botao, corAtual);
    DrawRectangleLinesEx(botao, 2, RGB(30, 100, 40));

    int largura = MeasureText(texto, 22);
    DrawText(texto,
             (int)(botao.x + botao.width  / 2 - largura / 2),
             (int)(botao.y + botao.height / 2 - 11),
             22, RGB(255, 255, 255));
}

int main(void)
{
    InitWindow(JANELA_W, JANELA_H, "Watermelon Game");
    SetTargetFPS(60);

    Rectangle btnPlay   = { JANELA_W / 2.0f - 100, 360, 200, 52 };
    Rectangle btnConf   = { JANELA_W / 2.0f - 100, 424, 200, 52 };
    Rectangle btnMusica = { JANELA_W / 2.0f - 100, 488, 200, 52 };
    Rectangle btnSair   = { JANELA_W / 2.0f - 100, 552, 200, 52 };

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RGB(245, 235, 210));

            /* Título */
            DrawText("Watermelon Game",
                     JANELA_W / 2 - MeasureText("Watermelon Game", 42) / 2,
                     40, 42, RGB(40, 140, 60));
            DrawText("Combine fruits to grow!",
                     JANELA_W / 2 - MeasureText("Combine fruits to grow!", 18) / 2,
                     96, 18, RGB(100, 100, 100));

            /* Botões — cor normal / cor hover */
            desenharBotao(btnPlay,   "PLAY",          RGB(60,  160, 70),  RGB(80,  200, 90));
            desenharBotao(btnConf,   "Configuracoes", RGB(60,  160, 70),  RGB(80,  200, 90));
            desenharBotao(btnMusica, "Musica",         RGB(60,  160, 70),  RGB(80,  200, 90));
            desenharBotao(btnSair,   "Sair",           RGB(170, 30,  30),  RGB(210, 60,  60));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}