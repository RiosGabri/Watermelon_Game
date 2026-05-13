#include "raylib.h"
#include "menu.h"               
#include <chipmunk/chipmunk.h> 
#include <stdio.h>

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

    cpSpace *espaco = cpSpaceNew();
    cpSpaceSetGravity(espaco, cpv(0, 900));

    cpBody *bolaBody = cpSpaceAddBody(espaco, cpBodyNew(1.0, cpMomentForCircle(1.0, 0, 25, cpvzero)));
    cpBodySetPosition(bolaBody, cpv(Largura/2, 100));
    cpSpaceAddShape(espaco, cpCircleShapeNew(bolaBody, 25, cpvzero));

    Estado_Jogo estado = EST_MENU;

    while (!WindowShouldClose()) {
        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) {
                    estado = EST_JOGO;
                    cpBodySetPosition(bolaBody, cpv(Largura/2, 100));
                    cpBodySetVelocity(bolaBody, cpvzero);
                }
                if (foi_clicado(btnExit)) goto fechar;
                break;

            case EST_JOGO:
                if (IsKeyPressed(KEY_ESCAPE)) estado = EST_MENU;
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
                cpVect pos = cpBodyGetPosition(bolaBody);
                DrawCircleV((Vector2){ (float)pos.x, (float)pos.y }, 25, ORANGE);
                DrawRectangle(0, Altura - 50, Largura, 50, DARKGRAY);
            }
        EndDrawing();
    }

fechar:
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
