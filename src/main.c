#include "raylib.h"
#include "menu.h"               
#include <chipmunk/chipmunk.h> 
#include <stdio.h>
#include "physics.h"
#include "fruits.h"

#define Largura 800
#define Altura 800
#define RGB(r, g, b) (Color){r, g, b, 255}

#ifdef _WIN32
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) { return _stat(path, buffer); }
#endif

int main(void) {
    InitWindow(Largura, Altura, "Watermelon Game");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Texture2D bg_menu = LoadTexture("Resources/menu_background.png");
    Texture2D tex_play = LoadTexture("Resources/button_play.png");
    Texture2D tex_exit = LoadTexture("Resources/button_exit.png");
    Texture2D tex_settings = LoadTexture("Resources/button_settings.png"); 
    Texture2D tex_music = LoadTexture("Resources/button_music.png");
    Texture2D tex_play_hover     = LoadTexture("Resources/button_play_hover.png");
    Texture2D tex_exit_hover     = LoadTexture("Resources/button_exit_hover.png");
    Texture2D tex_settings_hover = LoadTexture("Resources/button_settings_hover.png");
    Texture2D tex_music_hover    = LoadTexture("Resources/button_music_hover.png"); 

    Botao btnPlay     = { {(Largura/2.0f - 145), 300, 290, 78}, tex_play,     tex_play_hover };
    Botao btnMusic    = { {(Largura/2.0f - 145), 398, 290, 78}, tex_music,    tex_music_hover };
    Botao btnSettings = { {(Largura/2.0f - 145), 496, 290, 78}, tex_settings, tex_settings_hover };
    Botao btnExit     = { {(Largura/2.0f - 145), 594, 290, 78}, tex_exit,     tex_exit_hover };

    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);

    NodeFruta *head = NULL;
    g_head = &head;

    Estado_Jogo estado = EST_MENU;

    while (!WindowShouldClose()) {
        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) estado = EST_JOGO;
                if (foi_clicado(btnExit)) goto fechar;
                break;

            case EST_JOGO:
            if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){   //imput do jogo para spawnar fruta: lmb
                Vector2 mouse = GetMousePosition();
                float x = mouse.x;
                if (x < 115) x = 115;   //só spawna dentro da "cesta"
                if (x > 685) x = 685;
                inserirFruta(espaco, x, 160, 0, &head);
            }
            cpSpaceStep(espaco, 1.0f / 60.0f);
            break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            if (estado == EST_MENU) {
                DrawTexture(bg_menu, 0, 0, WHITE);
                    desenha_botao(btnPlay);
                    desenha_botao(btnMusic);
                    desenha_botao(btnSettings);
                    desenha_botao(btnExit);
            } else {
                ClearBackground(RGB(245, 235, 210));
                DrawRectangleLines(100, 150, 600, 600, DARKGRAY); //placeholder da "cesta"

                NodeFruta *atual = head;
                while (atual != NULL) {
                    cpVect pos = cpBodyGetPosition(atual->fruta.body);
                    float raio = LISTA_FRUTAS[atual->fruta.nivel].raio;
                    DrawCircle((int)pos.x, (int)pos.y, raio, RED);
                    DrawText(LISTA_FRUTAS[atual->fruta.nivel].nome,
                             (int)pos.x - 10, (int)pos.y - 5, 10, WHITE);
                    atual = atual->next;
                }

            }
        EndDrawing();
    }

fechar:
    while (head != NULL) {
        NodeFruta *tmp = head;
        head = head->next;
        cpSpaceRemoveShape(espaco, tmp->fruta.shape);
        cpShapeFree(tmp->fruta.shape);
        cpSpaceRemoveBody(espaco, tmp->fruta.body);
        cpBodyFree(tmp->fruta.body);
        free(tmp);
    }

    UnloadTexture(bg_menu);
    UnloadTexture(tex_play);
    UnloadTexture(tex_settings);
    UnloadTexture(tex_music);
    UnloadTexture(tex_exit);
    UnloadTexture(tex_play_hover);
    UnloadTexture(tex_exit_hover);
    UnloadTexture(tex_settings_hover);
    UnloadTexture(tex_music_hover);
    cpSpaceFree(espaco);
    CloseWindow();
    return 0;
}
