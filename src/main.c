#include "raylib.h"
#include "menu.h"
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include "physics.h"
#include "fruits.h"

#define Largura 800
#define Altura  800
#define RGB(r, g, b) (Color){r, g, b, 255}

#ifdef _WIN32
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) { return _stat(path, buffer); }
#endif

int main(void) {
    InitWindow(Largura, Altura, "Watermelon Game");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    /* Texturas do menu  */
    Texture2D bg_menu            = LoadTexture("Resources/menu_background.png");
    Texture2D tex_play           = LoadTexture("Resources/button_play.png");
    Texture2D tex_exit           = LoadTexture("Resources/button_exit.png");
    Texture2D tex_settings       = LoadTexture("Resources/button_settings.png");
    Texture2D tex_music          = LoadTexture("Resources/button_music.png");
    Texture2D tex_play_hover     = LoadTexture("Resources/button_play_hover.png");
    Texture2D tex_exit_hover     = LoadTexture("Resources/button_exit_hover.png");
    Texture2D tex_settings_hover = LoadTexture("Resources/button_settings_hover.png");
    Texture2D tex_music_hover    = LoadTexture("Resources/button_music_hover.png");

    Botao btnPlay     = { {(Largura/2.0f - 145), 300, 290, 78}, tex_play,     tex_play_hover };
    Botao btnMusic    = { {(Largura/2.0f - 145), 398, 290, 78}, tex_music,    tex_music_hover };
    Botao btnSettings = { {(Largura/2.0f - 145), 496, 290, 78}, tex_settings, tex_settings_hover };
    Botao btnExit     = { {(Largura/2.0f - 145), 594, 290, 78}, tex_exit,     tex_exit_hover };

    /*  Física  */
    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);

    NodeFruta *head = NULL;
    g_head = &head;

    /*  Estado do jogo  */
    Estado_Jogo estado = EST_MENU;

    /* Fruta suspensa (aguardando o clique do jogador)  */
    int   tipo_atual = GetRandomValue(0, 3); /* nível da fruta atual         */
    int   tipo_prox  = GetRandomValue(0, 3); /* nível da próxima fruta       */
    float pos_x      = Largura / 2.0f;       /* posição horizontal da fruta  */
    int   pode_soltar = 1;                   /* impede spam de cliques       */

    while (!WindowShouldClose()) {

        /*  Input  */
        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) {
                    estado = EST_JOGO;
                    tipo_atual = GetRandomValue(0, 3);
                    tipo_prox  = GetRandomValue(0, 3);
                    pos_x      = Largura / 2.0f;
                    pode_soltar = 1;
                }
                if (foi_clicado(btnExit)) goto fechar;
                break;

            case EST_JOGO:
                if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;

                /* Fruta segue o mouse horizontalmente */
                pos_x = GetMousePosition().x;
                if (pos_x < 115) pos_x = 115;
                if (pos_x > 685) pos_x = 685;

                /* Clique solta a fruta e sorteia a próxima */
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pode_soltar) {
                    inserirFruta(espaco, pos_x, 160, tipo_atual, &head);
                    tipo_atual  = tipo_prox;
                    tipo_prox   = GetRandomValue(0, 3);
                    pode_soltar = 0; /* bloqueia até a fruta pousar */
                }

                /* Libera o próximo clique após a fruta cair um pouco */
                if (!pode_soltar) {
                    /* Verifica se a última fruta inserida já passou da linha de soltura */
                    if (head != NULL) {
                        cpVect pos = cpBodyGetPosition(head->fruta.body);
                        if (pos.y > 200) pode_soltar = 1;
                    }
                }

                cpSpaceStep(espaco, 1.0f / 60.0f);
                break;
        }

        /*  Desenho  */
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
                DrawRectangleLines(100, 150, 600, 600, DARKGRAY); /* placeholder da cesta */

                /* Fruta suspensa seguindo o mouse */
                float raio_atual = LISTA_FRUTAS[tipo_atual].raio;
                DrawCircle((int)pos_x, 160, (int)raio_atual, RED);
                DrawText(LISTA_FRUTAS[tipo_atual].nome,
                         (int)(pos_x - 10), 155, 10, WHITE);

                /* Linha guia vertical (pontilhada) */
                for (int y = 160 + (int)raio_atual; y < 750; y += 12)
                    DrawPixel((int)pos_x, y, GRAY);

                /* Prévia da próxima fruta (canto superior direito) */
                DrawText("Proxima:", 710, 160, 14, DARKGRAY);
                float raio_prox = LISTA_FRUTAS[tipo_prox].raio;
                DrawCircle(735, 200, (int)raio_prox, BLUE);
                DrawText(LISTA_FRUTAS[tipo_prox].nome,
                         710, 215, 10, DARKGRAY);

                /* Frutas no tabuleiro */
                NodeFruta *atual = head;
                while (atual != NULL) {
                    cpVect pos  = cpBodyGetPosition(atual->fruta.body);
                    float  raio = LISTA_FRUTAS[atual->fruta.nivel].raio;
                    DrawCircle((int)pos.x, (int)pos.y, (int)raio, RED);
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