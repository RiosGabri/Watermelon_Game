#ifdef _WIN32
#define _STAT_DEFINED
#define stat    _stat64i32
#define fstat   _fstat64i32
#endif

#include "raylib.h"
#include <stdbool.h>

// Tamanho do jogo (ainda não definido, provisório para testes)
#define Largura 800
#define Altura 800
#define RGB(r, g, b) (Color){r, g, b, 255}

// Fluxo de estados, coloquei para acompanhar o swich case
typedef enum jogo {
    EST_MENU,
    EST_JOGO,
    EST_CONFIG
} Estado_Jogo;

//struct do botão
typedef struct Botao {
    Rectangle area;

    Texture2D textura_normal;
    Texture2D textura_hover;

} Botao;

// Botões + Hover (ajuda o jogador a ver onde o mouse está no menu)
static void desenha_botao(Botao b)
{
    Vector2 mouse = GetMousePosition();

    bool hover = CheckCollisionPointRec(mouse, b.area);

    Texture2D textura = hover
        ? b.textura_hover
        : b.textura_normal;

    DrawTexturePro(
        textura,
        (Rectangle){0, 0, textura.width, textura.height},
        b.area,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

static bool foi_clicado(Botao b)
{
    return CheckCollisionPointRec(GetMousePosition(), b.area)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}


int main(void)
{
    // Inicializa a janela
    InitWindow(Largura, Altura, "Watermelon Game");
    Texture2D bg_menu = LoadTexture("Resources/menu_background.png");   //define o bg do menu
    SetExitKey(KEY_NULL);// Desabilita o ESC fechar o jogo automaticamente, optei por colocá-lo para levar ao menu (não definitivo)
    SetTargetFPS(60);

    Estado_Jogo estado = EST_MENU; //Início do jogo no menu

    //Espaço de cada botão
    Texture2D start_normal = LoadTexture("Resources/button_play.png");
    Texture2D start_hover  = LoadTexture("Resources/button_play.png"); //ainda será feita a textura com hover!

    Texture2D exit_normal = LoadTexture("Resources/button_exit.png");
    Texture2D exit_hover  = LoadTexture("Resources/button_exit.png"); 

    Texture2D settings_normal = LoadTexture("Resources/button_settings.png");
    Texture2D settings_hover = LoadTexture("Resources/button_settings.png");

    Texture2D music_normal = LoadTexture("Resources/button_music.png");
    Texture2D music_hover = LoadTexture("Resources/button_music.png");

    Botao play = {
    {
        Largura / 2.0f - start_normal.width / 2.0f,
        300,
        (float)start_normal.width,
        (float)start_normal.height
    },

    start_normal,
    start_hover
};

Botao settings = {
    {
        Largura / 2.0f - settings_normal.width / 2.0f,
        430,
        (float)settings_normal.width,
        (float)settings_normal.height
    },
    settings_normal,
    settings_hover
};

Botao music = {
    {
        Largura / 2.0f - music_normal.width / 2.0f,
        560,
        (float)music_normal.width,
        (float)music_normal.height
    },

    music_normal,
    music_hover
};

Botao exit = {
    {
        Largura / 2.0f - exit_normal.width / 2.0f,
        690,
        (float)exit_normal.width,
        (float)exit_normal.height
    },

    exit_normal,
    exit_hover
};

    // Loop principal
    while (!WindowShouldClose())
    {
        switch (estado)
        {
            case EST_MENU:
                if (foi_clicado(play)) estado = EST_JOGO; // Troca para a tela de gameplay
                if (foi_clicado(exit)) goto fechar; // Clicou em sair, fecha o jogo
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

            if (estado == EST_MENU)// // Entra aqui apenas se o jogador estiver na tela inicial
            {
                DrawTexture(bg_menu, 0, 0, WHITE);
                DrawText("Watermelon Game", Largura / 2 - MeasureText("Watermelon Game", 42) / 2, 40, 42, RGB(40, 140, 60));
                DrawText("Desenvolvido por Rios, Larissa e Heitor",(int)(Largura / 2.0f - MeasureText("Desenvolvido por Rios, Larissa e Heitor", 18) / 2.0f),96, 18, RGB(100, 100, 100));
                desenha_botao(play);
                desenha_botao(settings);
                desenha_botao(music);
                desenha_botao(exit);
            }
            else if (estado == EST_JOGO) //O jogo deve estar aqui
            {
                DrawText("  Teste  ", 180, 300, 30, DARKGRAY);
                DrawText("ESC para voltar", 235, 350, 15, GRAY);
            }

        EndDrawing();
    }
fechar:
    UnloadTexture(bg_menu);
    UnloadTexture(start_normal);
    UnloadTexture(start_hover);
    UnloadTexture(exit_normal);
    UnloadTexture(exit_hover);
    UnloadTexture(settings_normal);
    UnloadTexture(settings_hover);
    UnloadTexture(music_normal);
    UnloadTexture(music_hover);
    CloseWindow();
    return 0;
}