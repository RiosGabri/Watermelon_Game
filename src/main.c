#ifdef _WIN32
#define _STAT_DEFINED
#define stat    _stat64i32
#define fstat   _fstat64i32
#endif

#include "raylib.h"

// Tamanho do jogo (ainda não definido, provisório para testes)
#define Largura 600
#define Altura 700
#define RGB(r, g, b) (Color){r, g, b, 255}

// Fluxo de estados, coloquei para acompanhar o swich case
typedef enum jogo {
    EST_MENU,
    EST_JOGO,
    EST_CONFIG
} Estado_Jogo;

// Botões + Hover (ajuda o jogador a ver onde o mouse está no menu)
static void desenha_botao(Rectangle ret, const char *texto, Color cor_base, Color cor_hover)
{
    Vector2 mouse = GetMousePosition(); //ve se o mouse está nos limites do botão (para o hover)
    int hover = CheckCollisionPointRec(mouse, ret);
    Color cor = hover ? cor_hover : cor_base;

    // Desenha o preenchimento e a borda
    DrawRectangleRec(ret, cor);
    DrawRectangleLinesEx(ret, 2, RGB(30, 100, 40));

    // Centralizar o texto no meio do botão
    int tam_txt = MeasureText(texto, 22);
    DrawText(texto,
             (int)(ret.x + ret.width / 2 - tam_txt / 2),
             (int)(ret.y + ret.height / 2 - 11),
             22, WHITE);
}

static int foi_clicado(Rectangle ret)
{
    return CheckCollisionPointRec(GetMousePosition(), ret) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); //Ve se foi botão esquerdo do mouse clicado
}
int main(void)
{
    // Inicializa a janela
    InitWindow(Largura, Altura, "Watermelon Game");
    SetExitKey(KEY_NULL);// Desabilita o ESC fechar o jogo automaticamente, optei por colocá-lo para levar ao menu (não definitivo)
    SetTargetFPS(60);

    Estado_Jogo estado = EST_MENU; //Início do jogo no menu

    //Espaço de cada botão
    Rectangle r_play  = { Largura / 2.0f - 100, 360, 200, 52 };
    Rectangle r_conf  = { Largura / 2.0f - 100, 424, 200, 52 };
    Rectangle r_audio = { Largura / 2.0f - 100, 488, 200, 52 };
    Rectangle r_sair  = { Largura / 2.0f - 100, 552, 200, 52 };

    // Loop principal
    while (!WindowShouldClose())
    {
        switch (estado)
        {
            case EST_MENU:
                if (foi_clicado(r_play)) estado = EST_JOGO; // Troca para a tela de gameplay
                if (foi_clicado(r_sair)) goto fechar; // Clicou em sair, fecha o jogo
                break;

                case EST_JOGO:
                if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU; //Depois implemento botão de pause dentro do jogo e resolvo esse botão
                break;

                //Redundância temporária, depois corrijo isso
            case EST_CONFIG:
                if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;
                break;
        }

        BeginDrawing(); // Renderização na tela (Provisório para testes)
        // Aplicação do mapa + frutas
            ClearBackground(RGB(245, 235, 210));

            if (estado == EST_MENU)// // Entra aqui apenas se o jogador estiver na tela inicial
            {
                DrawText("Watermelon Game", Largura / 2 - MeasureText("Watermelon Game", 42) / 2, 40, 42, RGB(40, 140, 60));
                DrawText("Desenvolvido por Rios, Larissa e Heitor",(int)(Largura / 2.0f - MeasureText("Desenvolvido por Rios, Larissa e Heitor", 18) / 2.0f),96, 18, RGB(100, 100, 100));
                desenha_botao(r_play,  "JOGAR",      RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_conf,  "OPÇÕES",     RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_audio, "MÚSICA",     RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_sair,  "SAIR",       RGB(170, 30, 30), RGB(210, 60, 60));
            }
            else if (estado == EST_JOGO) //O jogo deve estar aqui
            {
                DrawText("  Teste  ", 180, 300, 30, DARKGRAY);
                DrawText("ESC para voltar", 235, 350, 15, GRAY);
            }

        EndDrawing();
    }
fechar:
    CloseWindow();
    return 0;
}