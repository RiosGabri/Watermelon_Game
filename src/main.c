#ifdef _WIN32
#include <stdio.h>
#include <chipmunk/chipmunk.h>
#include "raylib.h"
#include "menu.h"
#include "fruits.h"
#include "physics.h"

#define Largura 800
#define Altura 800
#define RGB(r, g, b) (Color){r, g, b, 255}

int main(void) {
    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);
    NodeFruta *headFrutas = NULL;

    InitWindow(Largura, Altura, "Watermelon Game");
    Texture2D bg_menu = LoadTexture("Resources/menu_background.png");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Estado_Jogo estado = EST_MENU;

    Rectangle r_play  = { Largura / 2.0f - 100, 360, 200, 52 };
    Rectangle r_conf  = { Largura / 2.0f - 100, 424, 200, 52 };
    Rectangle r_audio = { Largura / 2.0f - 100, 488, 200, 52 };
    Rectangle r_sair  = { Largura / 2.0f - 100, 552, 200, 52 };

    while (!WindowShouldClose()) {

        if (estado == EST_MENU) {
            if (foi_clicado(r_play)) {
                inserirFruta(espaco, 400, 100, 0, &headFrutas);
                estado = EST_JOGO;
            }
            if (foi_clicado(r_sair)) break;
        }
        else if (estado == EST_JOGO) {
            if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;

            cpSpaceStep(espaco, 1.0f / 60.0f);
            processarFusoes(espaco, &headFrutas);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (estado == EST_MENU) {
                DrawTexture(bg_menu, 0, 0, WHITE);
                DrawText("Watermelon Game", Largura/2 - MeasureText("Watermelon Game", 42)/2, 40, 42, RGB(40, 140, 60));

                desenha_botao(r_play,  "JOGAR",  RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_conf,  "OPÇÕES", RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_audio, "MÚSICA", RGB(60, 160, 70), RGB(80, 200, 90));
                desenha_botao(r_sair,  "SAIR",   RGB(170, 30, 30), RGB(210, 60, 60));
            }
            else if (estado == EST_JOGO) {
                NodeFruta *atual = headFrutas;
                while (atual != NULL) {
                    cpVect pos = cpBodyGetPosition(atual->fruta.body);
                    DrawCircle(pos.x, pos.y, (atual->fruta.nivel + 1) * 10, ORANGE);
                    atual = atual->next;
                }
                DrawText("ESC para voltar", 10, 10, 20, DARKGRAY);
            }

        EndDrawing();
    }
    
    UnloadTexture(bg_menu);
    cpSpaceFree(espaco);
    CloseWindow();

    return 0;
}
#endif
