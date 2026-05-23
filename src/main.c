#include "raylib.h"
#include "menu.h"
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include "physics.h"
#include "fruits.h"
#include "obstaculos.h"

#define Largura 800
#define Altura  800
#define RGB(r, g, b) (Color){r, g, b, 255}

#ifdef _WIN32
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) { return _stat(path, buffer); }
#endif

int main(void) {
    InitWindow(Largura, Altura, "Watermelon Game");
    SetWindowPosition(50, 50);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Texture2D bg_menu            = LoadTexture("Resources/menu_background.png");
    Texture2D tex_play           = LoadTexture("Resources/button_play.png");
    Texture2D tex_exit           = LoadTexture("Resources/button_exit.png");
    Texture2D tex_settings       = LoadTexture("Resources/button_settings.png");
    Texture2D tex_music          = LoadTexture("Resources/button_music.png");
    Texture2D tex_play_hover     = LoadTexture("Resources/button_play_hover.png");
    Texture2D tex_exit_hover     = LoadTexture("Resources/button_exit_hover.png");
    Texture2D tex_settings_hover = LoadTexture("Resources/button_settings_hover.png");
    Texture2D tex_music_hover    = LoadTexture("Resources/button_music_hover.png");
    Texture2D title_menu          = LoadTexture("Resources/title.png");
    
    Texture2D tex_frutas[NIVEIS_FRUTA];
    tex_frutas[0] = LoadTexture("Resources/uva.png");
    tex_frutas[1] = LoadTexture("Resources/cereja.png");
    tex_frutas[2] = LoadTexture("Resources/morango.png");
    tex_frutas[3] = LoadTexture("Resources/maca.png");
    tex_frutas[4] = LoadTexture("Resources/pera.png");
    tex_frutas[5] = LoadTexture("Resources/laranja.png");
    tex_frutas[6] = LoadTexture("Resources/abacaxi.png");
    tex_frutas[7] = LoadTexture("Resources/melancia.png");

    Botao btnPlay     = { {(Largura/2.0f - 145), 300, 290, 78}, tex_play,     tex_play_hover };
    Botao btnMusic    = { {(Largura/2.0f - 145), 398, 290, 78}, tex_music,    tex_music_hover };
    Botao btnSettings = { {(Largura/2.0f - 145), 496, 290, 78}, tex_settings, tex_settings_hover };
    Botao btnExit     = { {(Largura/2.0f - 145), 594, 290, 78}, tex_exit,     tex_exit_hover };

    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);

    inicializarObstaculos();
    configurarCallbacksObstaculos(espaco);

    NodeFruta *head = NULL;
    g_head = &head;

    Estado_Jogo estado = EST_MENU;

    int   tipo_atual = GetRandomValue(0, 3); 
    int   tipo_prox  = GetRandomValue(0, 3); 
    float pos_x      = Largura / 2.0f;       
    int   pode_soltar = 1;                   
    int   contadorCliques = 0;

    while (!WindowShouldClose()) {

        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) {
                    estado = EST_JOGO;
                    tipo_atual = GetRandomValue(0, 3);
                    tipo_prox  = GetRandomValue(0, 3);
                    pos_x      = Largura / 2.0f;
                    pode_soltar = 1;
                    contadorCliques = 0;
                    inicializarObstaculos();
                }
                if (foi_clicado(btnExit)) goto fechar;
                if (foi_clicado(btnMusic)) estado = EST_MUSICA;
                if (foi_clicado(btnSettings)) estado = EST_CONFIGURACAO;
                break;
            case EST_MUSICA:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU; // se apertar esc, volta para menu
                }
                break;
            case EST_CONFIGURACAO:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU; // se apertar esc, volta para menu
                }
                break;
            case EST_JOGO:
                if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;

                pos_x = GetMousePosition().x;
                if (pos_x < 115) pos_x = 115;
                if (pos_x > 685) pos_x = 685;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pode_soltar) {
                    inserirFruta(espaco, pos_x, 160, tipo_atual, &head);
                    tipo_atual  = tipo_prox;
                    tipo_prox   = GetRandomValue(0, 3);
                    pode_soltar = 0; 

                    contadorCliques++;
                    if (contadorCliques >= 10) {
                        spawnBlocoFixo(espaco, &head); 
                        contadorCliques = 0;
                    } else {
                        testarSpawnEspecial(espaco, &head); 
                    }
                }

                if (!pode_soltar) {
                    if (head != NULL) {
                        cpVect pos = cpBodyGetPosition(head->fruta.body);
                        if (pos.y > 200) pode_soltar = 1;
                    }
                }

                cpSpaceStep(espaco, 1.0f / 60.0f);
                atualizarELimparObstaculos(espaco, &head); 
                processarFusoes(espaco, &head);           
                break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (estado == EST_MENU) {
                DrawTexture(bg_menu, 0, 0, WHITE);
                float largura_titulo = 500;
                float altura_titulo = 160;
                DrawTexturePro( //ajustar tamanho do titulo
                    title_menu,
                    (Rectangle){0, 0, (float)title_menu.width, (float)title_menu.height},
                    (Rectangle){(Largura/2.0f) - (largura_titulo/2.0f), 60, largura_titulo, altura_titulo},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE);
                desenha_botao(btnPlay);
                desenha_botao(btnMusic);
                desenha_botao(btnSettings);
                desenha_botao(btnExit);

            } else if (estado == EST_MUSICA) {
                DrawTexture(bg_menu, 0, 0, WHITE);

                // Painel central
                DrawRectangleRounded((Rectangle){200, 200, 400, 300}, 0.2f, 8, (Color){0, 0, 0, 160});
                DrawRectangleRoundedLines((Rectangle){200, 200, 400, 300}, 0.2f, 8, WHITE);

                // Título do painel
                DrawText("MUSICAS", 290, 230, 40, WHITE);

                // Linha separadora
                DrawLine(220, 285, 580, 285, WHITE);

                // Texto em breve
                DrawText("Em breve...", 295, 320, 28, YELLOW);

                // Instrução para voltar
                DrawText("Pressione ESC para voltar", 230, 450, 18, LIGHTGRAY);
            } else if (estado == EST_CONFIGURACAO) {
                DrawTexture(bg_menu, 0, 0, WHITE);

                // Painel central
                DrawRectangleRounded((Rectangle){200, 200, 400, 300}, 0.2f, 8, (Color){0, 0, 0, 160});
                DrawRectangleRoundedLines((Rectangle){200, 200, 400, 300}, 0.2f, 8, WHITE);

                // Título do painel
                DrawText("CONFIGURAÇAO", 250, 230, 40, WHITE);

                // Linha separadora
                DrawLine(220, 285, 580, 285, WHITE);

                // Texto em breve
                DrawText("Em breve...", 295, 320, 28, YELLOW);

                // Instrução para voltar
                DrawText("Pressione ESC para voltar", 230, 450, 18, LIGHTGRAY);
            }else {
                ClearBackground(RGB(245, 235, 210));
                DrawRectangleLines(100, 150, 600, 600, DARKGRAY); 

                float raio_atual = LISTA_FRUTAS[tipo_atual].raio;
                float diametro_atual = raio_atual * 2;
                Texture2D tex_atual = tex_frutas[tipo_atual];
                DrawTexturePro(
                    tex_atual,
                    (Rectangle){0, 0, (float)tex_atual.width, (float)tex_atual.height},
                    (Rectangle){pos_x, 160, diametro_atual, diametro_atual},
                    (Vector2){raio_atual, raio_atual},
                    0.0f,
                    WHITE
                );

                for (int y = 160 + (int)raio_atual; y < 750; y += 12)
                    DrawPixel((int)pos_x, y, GRAY);
                DrawText("Proxima:", 710, 160, 14, DARKGRAY);
                float raio_prox = LISTA_FRUTAS[tipo_prox].raio;
                Texture2D tex_prox = tex_frutas[tipo_prox];
                float diametro_prox = raio_prox * 2;
                DrawTexturePro(
                    tex_prox,
                    (Rectangle){0, 0, (float)tex_prox.width, (float)tex_prox.height},
                    (Rectangle){735, 200, diametro_prox, diametro_prox},
                    (Vector2){raio_prox, raio_prox},
                    0.0f,
                    WHITE
                );
                NodeFruta *atual = head;
                while (atual != NULL) {
                    cpVect pos  = cpBodyGetPosition(atual->fruta.body);
                    float  raio = LISTA_FRUTAS[atual->fruta.nivel].raio;
                    float  angulo = (float)cpBodyGetAngle(atual->fruta.body) * RAD2DEG; 
                    float  diametro = raio * 2;
                    Texture2D tex = tex_frutas[atual->fruta.nivel];
                    
                    DrawTexturePro(
                        tex,
                        (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                        (Rectangle){(float)pos.x, (float)pos.y, diametro, diametro},
                        (Vector2){raio, raio},
                        angulo,
                        WHITE
                    );
                    if (atual->fruta.estaPodre) {
                        DrawCircleLines((int)pos.x, (int)pos.y, (int)raio, DARKGREEN);
                    } 
                    atual = atual->next;
                }
                desenharObstaculos();
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
    for (int i = 0; i < qtdObstaculos; i++) {
        if (listaObstaculos[i].ativo) {
            cpSpaceRemoveShape(espaco, listaObstaculos[i].shape);
            cpShapeFree(listaObstaculos[i].shape);
            if (listaObstaculos[i].tipo != OBJ_BLOCO) {
                cpSpaceRemoveBody(espaco, listaObstaculos[i].body);
                cpBodyFree(listaObstaculos[i].body);
            }
        }
    }

    UnloadTexture(bg_menu);
    UnloadTexture(title_menu);
    UnloadTexture(tex_play);
    UnloadTexture(tex_settings);
    UnloadTexture(tex_music);
    UnloadTexture(tex_exit);
    UnloadTexture(tex_play_hover);
    UnloadTexture(tex_exit_hover);
    UnloadTexture(tex_settings_hover);
    UnloadTexture(tex_music_hover);
    for (int i = 0; i < NIVEIS_FRUTA; i++) {
        UnloadTexture(tex_frutas[i]);
    }
    cpSpaceFree(espaco);
    CloseWindow();
    return 0;
}