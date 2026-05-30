#include "raylib.h"
#include "menu.h"
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include "physics.h"
#include "fruits.h"
#include "obstaculos.h"
#include <string.h>

#define Largura 800
#define Altura  800
#define RGB(r, g, b) (Color){r, g, b, 255}

#ifdef _WIN32
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) { return _stat(path, buffer); }
#endif

void salvarPlacar(const char *nome, int pontos) {
    FILE *f = fopen("leaderboard.txt", "a");
    if (f) {
        fprintf(f, "%s,%d\n", nome, pontos);
        fclose(f);
    }
}

void mostrarLeaderboard(char nomes[][11], int pontos[], int *qtd) {
    FILE *f = fopen("leaderboard.txt", "r");

    *qtd = 0;

    if(f != NULL) {
        while (
            fscanf(f, "%10[^,],%d\n",
            nomes[*qtd],
            &pontos[*qtd]) == 2
        ) {
            (*qtd)++;

            if (*qtd >= 100)
                break;
        }
        fclose(f);

        //pra deixar ordenado
        for (int i = 0; i < *qtd - 1; i++) {
            for (int j = i + 1; j < *qtd; j++) {
                if (pontos[j] > pontos[i]) {
                    int tempPontos = pontos[i];
                    pontos[i] = pontos[j];
                    pontos[j] = tempPontos;
                    char tempNome[11];
                    strcpy(tempNome, nomes[i]);
                    strcpy(nomes[i], nomes[j]);
                    strcpy(nomes[j], tempNome);
                }
            }
        }
    }
}

int main(void) {
    InitWindow(Largura, Altura, "Watermelon Game");
    Image icon = LoadImage("Resources/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    InitAudioDevice();
    SetWindowPosition(50, 50);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Texture2D bg_menu            = LoadTexture("Resources/menu_background.png");
    Texture2D tex_play           = LoadTexture("Resources/button_play.png");
    Texture2D tex_exit           = LoadTexture("Resources/button_exit.png");
    Texture2D tex_settings       = LoadTexture("Resources/button_settings.png");
    Texture2D tex_play_hover     = LoadTexture("Resources/button_play_hover.png");
    Texture2D tex_exit_hover     = LoadTexture("Resources/button_exit_hover.png");
    Texture2D tex_settings_hover = LoadTexture("Resources/button_settings_hover.png");
    Texture2D tex_music_hover    = LoadTexture("Resources/button_music_hover.png");
    Texture2D title_menu         = LoadTexture("Resources/title.png");

    Texture2D tex_frutas[NIVEIS_FRUTA];
    tex_frutas[0] = LoadTexture("Resources/uva.png");
    tex_frutas[1] = LoadTexture("Resources/cereja.png");
    tex_frutas[2] = LoadTexture("Resources/morango.png");
    tex_frutas[3] = LoadTexture("Resources/maca.png");
    tex_frutas[4] = LoadTexture("Resources/pera.png");
    tex_frutas[5] = LoadTexture("Resources/laranja.png");
    tex_frutas[6] = LoadTexture("Resources/abacaxi.png");
    tex_frutas[7] = LoadTexture("Resources/melancia.png");

    Texture2D tex_frutas_podres[NIVEIS_FRUTA];
    tex_frutas_podres[0] = LoadTexture("Resources/bad_uva.png");
    tex_frutas_podres[1] = LoadTexture("Resources/bad_cereja.png");
    tex_frutas_podres[2] = LoadTexture("Resources/bad_morango.png");
    tex_frutas_podres[3] = LoadTexture("Resources/bad_maca.png");
    tex_frutas_podres[4] = LoadTexture("Resources/bad_pera.png");
    tex_frutas_podres[5] = LoadTexture("Resources/bad_laranja.png");
    tex_frutas_podres[6] = LoadTexture("Resources/bad_abacaxi.png");
    tex_frutas_podres[7] = LoadTexture("Resources/bad_melancia.png");

    Texture2D tex_podre = LoadTexture("Resources/podre.png");
    Texture2D tex_bomba = LoadTexture("Resources/bomb.png");
    Texture2D tex_pimenta = LoadTexture("Resources/pimenta.png");
    Texture2D tex_bloco = LoadTexture("Resources/block.png");

    Botao btnPlay     = { {(Largura/2.0f - 145), 300, 290, 78}, tex_play,     tex_play_hover };
    Botao btnSettings = { {(Largura/2.0f - 145), 398, 290, 78}, tex_settings, tex_settings_hover };
    Botao btnExit     = { {(Largura/2.0f - 145), 496, 290, 78}, tex_exit,     tex_exit_hover };

    Music bossaMelon = LoadMusicStream("Resources/music/BossaMelon.mp3");
    Music violoncia = LoadMusicStream("Resources/music/Violoncia.mp3");
    Music frutinhas = LoadMusicStream("Resources/music/Frutinhas.mp3");
    SetMusicVolume(bossaMelon, 1.5f); //volume da musica
    SetMusicVolume(violoncia, 1.5f); //volume da musica
    SetMusicVolume(frutinhas, 1.5f); //volume da musica
    PlayMusicStream(bossaMelon);

    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);

    inicializarObstaculos();
    configurarCallbacksObstaculos(espaco);

    NodeFruta *head = NULL;
    g_head = &head;

    Estado_Jogo estado       = EST_MENU;
    int   tipo_atual         = GetRandomValue(0, 3);
    int   tipo_prox          = GetRandomValue(0, 3);
    float pos_x              = Largura / 2.0f;
    int   pode_soltar        = 1;
    int   contadorCliques    = 0;
    int musica_selecionada = 0;
    float volume_musica = 1.0f;
    float tempo_do_limite = 3.0f;
    int vitorias = 0;
    char player[11] = {0};
    int placar_salvo = 0;

    char leaderboard_nomes[10][11];    //matrizes pra printar o leaderboard
    int leaderboard_pontos[10];
    int total_scores = 0;
    Rectangle btnLeaderboard = {20, 760, 120, 30};

    while (!WindowShouldClose()) {
    Music *musicas[3] = {&bossaMelon, &violoncia, &frutinhas};
    UpdateMusicStream(*musicas[musica_selecionada]);
    UpdateMusicStream(bossaMelon); //atuailza as musicas a cada frame
        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) {
                    estado         = EST_NOME;
                    tipo_atual     = GetRandomValue(0, 3);
                    tipo_prox      = GetRandomValue(0, 3);
                    pos_x          = Largura / 2.0f;
                    pode_soltar    = 1;
                    contadorCliques = 0;
                    inicializarObstaculos();
                    StopMusicStream(bossaMelon);
                    Music *musicas[3] = {&bossaMelon, &violoncia, &frutinhas};
                    PlayMusicStream(*musicas[musica_selecionada]);
                }
                if (foi_clicado(btnExit)) goto fechar;
                if (foi_clicado(btnSettings)) estado = EST_CONFIGURACAO;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, btnLeaderboard)) {
                        mostrarLeaderboard(
                            leaderboard_nomes,
                            leaderboard_pontos,
                            &total_scores
                        );
                        estado = EST_LEADERBOARD;
                    }
                }
                
                break;
            case EST_CONFIGURACAO:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU; // se apertar esc, volta para menu
                    StopMusicStream(violoncia);
                    StopMusicStream(frutinhas);
                    PlayMusicStream(bossaMelon);
                }

                //escolher musica com setas esquerda/direita
                if (IsKeyPressed(KEY_LEFT)) {
                    musica_selecionada--;
                    if (musica_selecionada < 0) musica_selecionada = 2;
                }
                if (IsKeyPressed(KEY_RIGHT)) {
                    musica_selecionada++;
                    if (musica_selecionada > 2) musica_selecionada = 0;
                }

                // Ajustar volume com setas cima/baixo
                if (IsKeyPressed(KEY_UP)) {
                    volume_musica += 0.1f;
                    if (volume_musica > 1.0f) volume_musica = 1.0f;
                }
                if (IsKeyPressed(KEY_DOWN)) {
                    volume_musica -= 0.1f;
                    if (volume_musica < 0.0f) volume_musica = 0.0f;
                }

                // clicar nos botoes de setas e volume
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    printf("Mouse clicado em: (%.1f, %.1f)\n", mouse.x, mouse.y);
                    // Seta esquerda do select (< em x=230, y=330)
                    if (CheckCollisionPointRec(mouse, (Rectangle){225, 320, 40, 35})) {
                        musica_selecionada--;
                        printf("Clicado seta esquerda\n");
                        if (musica_selecionada < 0) musica_selecionada = 2;
                    }                                                               
                    // Seta direita do select (> em x=530, y=330)
                    if (CheckCollisionPointRec(mouse, (Rectangle){535, 320, 40, 35})) {
                        musica_selecionada++;
                        printf("Clicado seta direita\n");
                        if (musica_selecionada > 2) musica_selecionada = 0;
                    }

                    // Botão - do volume
                    if (CheckCollisionPointRec(mouse, (Rectangle){225, 400, 40, 35})) {
                        volume_musica -= 0.1f;
                        printf("clicado -\n");
                        if (volume_musica < 0.0f) volume_musica = 0.0f;
                    }
                    // Botão + do volume
                    if (CheckCollisionPointRec(mouse, (Rectangle){535, 400, 40, 35})) {
                        volume_musica += 0.1f;
                        printf("clicado +\n");
                        if (volume_musica > 1.0f) volume_musica = 1.0f;
                    }
                }

                // Aplica o volume nas músicas de jogo
                SetMusicVolume(violoncia,  volume_musica);
                SetMusicVolume(frutinhas,  volume_musica);
                SetMusicVolume(bossaMelon, volume_musica);
                break;

            case EST_PAUSE:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estado = EST_JOGO;
                    ResumeMusicStream(violoncia);
                    ResumeMusicStream(frutinhas);
                }
                // botao de retomar (clique)
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){300, 330, 200, 50})) {
                        estado = EST_JOGO;
                        ResumeMusicStream(violoncia);
                        ResumeMusicStream(frutinhas);
                    }
                    if (CheckCollisionPointRec(mouse, (Rectangle){300, 400, 200, 50})) {
                        estado = EST_MENU;
                        StopMusicStream(violoncia);
                        StopMusicStream(frutinhas);
                        PlayMusicStream(bossaMelon);
                    }
                }
                break;
            case EST_GAMEOVER:
            case EST_VITORIA:
                if (!placar_salvo) {
                    salvarPlacar(player, cont_pontos);
                    placar_salvo = 1;
                }

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                    // Resetar o jogo
                    while (head != NULL) {
                        NodeFruta *tmp = head;
                        head = head->next;
                        cpSpaceRemoveShape(espaco, tmp->fruta.shape);
                        cpShapeFree(tmp->fruta.shape);
                        cpSpaceRemoveBody(espaco, tmp->fruta.body);
                        cpBodyFree(tmp->fruta.body);
                        free(tmp);
                    }
                    inicializarObstaculos();
                    tipo_atual      = GetRandomValue(0, 3);
                    tipo_prox       = GetRandomValue(0, 3);
                    pos_x           = Largura / 2.0f;
                    pode_soltar     = 1;
                    contadorCliques = 0;
                    tempo_do_limite    = 0.0f;
                    vitorias = 0;
                    cont_pontos = 0;
                    placar_salvo = 0;
                    estado          = EST_MENU;
                    PlayMusicStream(bossaMelon);
                }
                break;
            
            case EST_NOME: 
                int input = GetCharPressed();

                while (input > 0) {
                    if (strlen(player) < 10 && input >= ' ') {
                        int len = strlen(player);
                        player[len] = (char)input;
                        player[len + 1] = '\0';
                    }

                    input = GetCharPressed();
                }

                int len = strlen(player);

                if (IsKeyPressed(KEY_BACKSPACE) && len > 0) {
                    player[len - 1] = '\0';
                }

                if (IsKeyPressed(KEY_ENTER) && strlen(player) > 0) {    //inicia o jogo
                    estado = EST_JOGO;
                    tipo_atual = GetRandomValue(0, 3);
                    tipo_prox  = GetRandomValue(0, 3);
                    pos_x = Largura / 2.0f;
                    pode_soltar = 1;
                    contadorCliques = 0;
                    cont_pontos = 0;
                    inicializarObstaculos();
                    StopMusicStream(bossaMelon);
                    PlayMusicStream(*musicas[musica_selecionada]);
                }

                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU;
                }

                break;

            case EST_LEADERBOARD:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU;
                }
                break;

            case EST_JOGO:
                if (IsKeyPressed(KEY_ESCAPE)) {

                    estado = EST_PAUSE;
                    PauseMusicStream(violoncia);
                    PauseMusicStream(frutinhas);
                }

                pos_x = GetMousePosition().x;
                if (pos_x < 115) pos_x = 115;
                if (pos_x > 685) pos_x = 685;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pode_soltar) {
                    inserirFruta(espaco, pos_x, 160, tipo_atual, &head);
                    tipo_atual  = tipo_prox;
                    tipo_prox   = GetRandomValue(0, 3);
                    pode_soltar = 0;
                    atualizarFrutasPodres(head);
                    contadorCliques++;

                    if (contadorCliques >= 10) {
                        spawnBlocoFixo(espaco, &head);
                        contadorCliques = 0;
                    } else {
                        testarSpawnEspecial(espaco, &head);
                    }
                }
                if (!pode_soltar && head != NULL) {
                    cpVect pos = cpBodyGetPosition(head->fruta.body);
                    if (pos.y > 200) pode_soltar = 1;
                }
                cpSpaceStep(espaco, 1.0f / 60.0f);
                atualizarELimparObstaculos(espaco, &head);
                processarFusoes(espaco, &head);

                //checar se as frutas chegaram no limite (game over)
                int fruta_no_limite = 0;
                NodeFruta *chk = head;
                while (chk != NULL) {
                    cpVect p = cpBodyGetPosition(chk->fruta.body);
                    if ((float)p.y < 200.0f) {
                        fruta_no_limite = 1;
                        break;
                    }
                    chk = chk->next;
                }
                if (fruta_no_limite) {
                    tempo_do_limite += GetFrameTime();
                    if (tempo_do_limite >= 3.0f) {
                        estado = EST_GAMEOVER;
                        StopMusicStream(violoncia);
                        StopMusicStream(frutinhas);
                    }
                } else {
                    tempo_do_limite = 0.0f;
                }

                //checagem se venceu
                if (!vitorias) {
                    NodeFruta *v = head;
                    while (v != NULL) {
                        if (v->fruta.nivel == NIVEIS_FRUTA - 1) { // melancia = ultimo nível
                            vitorias = 1;
                            estado = EST_VITORIA;
                            StopMusicStream(violoncia);
                            StopMusicStream(frutinhas);
                            break;
                        }
                        v = v->next;
                    }
                }

                break;
        }
        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (estado == EST_MENU) {
                DrawTexture(bg_menu, 0, 0, WHITE);
                float largura_titulo = 500;
                float altura_titulo = 160;
                DrawTexturePro(
                    title_menu,
                    (Rectangle){0, 0, (float)title_menu.width, (float)title_menu.height},
                    (Rectangle){(Largura/2.0f) - (largura_titulo/2.0f), 60, largura_titulo, altura_titulo},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE);

                desenha_botao(btnPlay);
                desenha_botao(btnSettings);
                desenha_botao(btnExit);

                DrawText("Leaderboard", 20, 760, 20, YELLOW);

            } else if (estado == EST_CONFIGURACAO) {
                const char *nomes_musicas[3] = {"BossaMelon", "Violoncia", "Frutinhas"};

                DrawTexture(bg_menu, 0, 0, WHITE);

                // Painel
                DrawRectangle(200, 180, 400, 340, (Color){0, 0, 0, 180});
                DrawRectangleLines(200, 180, 400, 340, WHITE);

                // Título
                DrawText("MUSICAS", 300, 200, 36, WHITE);
                DrawLine(220, 248, 580, 248, WHITE);

                // --- Select de música ---
                DrawText("Musica da partida:", 240, 290, 18, LIGHTGRAY);

                // Seta esquerda
                DrawText("<", 238, 322, 28, YELLOW);
                // Nome da música selecionada (centralizado)
                int tw = MeasureText(nomes_musicas[musica_selecionada], 22);
                DrawText(nomes_musicas[musica_selecionada], 400 - tw/2, 325, 22, WHITE);
                // Seta direita
                DrawText(">", 548, 322, 28, YELLOW);

                DrawLine(220, 368, 580, 368, (Color){255,255,255,60});

                // --- Controle de volume ---
                DrawText("Volume da musica:", 240, 382, 18, LIGHTGRAY);

                // Botão -
                DrawText("-", 238, 408, 32, YELLOW);
                // Barra de volume
                DrawRectangle(270, 415, 260, 14, (Color){255,255,255,40});
                DrawRectangle(270, 415, (int)(260 * volume_musica), 14, GREEN);
                DrawRectangleLines(270, 415, 260, 14, WHITE);
                
                // Botão +
                DrawText("+", 548, 406, 28, YELLOW);

                // Dica
                DrawText("< > para musica | ^ v para volume | ESC para voltar", 222, 490, 15, LIGHTGRAY);

            } else if (estado == EST_NOME) {
                DrawTexture(bg_menu, 0, 0, WHITE);
                DrawRectangle(200, 250, 400, 200, (Color){0, 0, 0, 180});
                DrawRectangleLines(200, 250, 400, 200, WHITE);
                DrawText("Insira seu nome:", 240, 275, 24, WHITE);
                DrawText(player, 240, 320, 28, YELLOW);
                DrawRectangleLines(235, 315, 320, 40, WHITE);
                DrawText("Pressione ENTER para confirmar", 240, 390, 16, LIGHTGRAY);
            } else if (estado == EST_PAUSE) {
            // Desenha o jogo por baixo (congelado)
            DrawTexture(bg_menu, 0, 0, WHITE);
            DrawRectangleLines(100, 150, 600, 600, DARKGRAY);

            NodeFruta *atual = head;
            while (atual != NULL) {
                cpVect pos     = cpBodyGetPosition(atual->fruta.body);
                float  raio    = LISTA_FRUTAS[atual->fruta.nivel].raio;
                float  angulo  = (float)cpBodyGetAngle(atual->fruta.body) * RAD2DEG;
                float  diam    = raio * 2;
                Texture2D tex  = atual->fruta.estaPodre ? tex_frutas_podres[atual->fruta.nivel]
                                                        : tex_frutas[atual->fruta.nivel];
                DrawTexturePro(tex,
                    (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                    (Rectangle){(float)pos.x, (float)pos.y, diam, diam},
                    (Vector2){raio, raio}, angulo, WHITE);
                atual = atual->next;
            }
            desenharObstaculos(tex_bomba, tex_podre, tex_pimenta, tex_bloco);

            // blur escuro
            DrawRectangle(0, 0, Largura, Altura, (Color){0, 0, 0, 150});

            // Painel central
            DrawRectangle(250, 260, 300, 230, (Color){0, 0, 0, 200});
            DrawRectangleLines(250, 260, 300, 230, WHITE);

            // Título
            int tw = MeasureText("PAUSADO", 36);
            DrawText("PAUSADO", 400 - tw/2, 280, 36, WHITE);
            DrawLine(270, 330, 530, 330, WHITE);

            // Botão Retomar
            bool hover_retomar = CheckCollisionPointRec(GetMousePosition(), (Rectangle){300, 345, 200, 50});
            DrawRectangle(300, 345, 200, 50,
                hover_retomar ? (Color){80, 200, 80, 255} : (Color){50, 150, 50, 255});
            int tw2 = MeasureText("Retomar", 22);
            DrawText("Retomar", 400 - tw2/2, 358, 22, WHITE);

            // Botão Sair
            bool hover_sair = CheckCollisionPointRec(GetMousePosition(), (Rectangle){300, 415, 200, 50});
            DrawRectangle(300, 415, 200, 50,
                hover_sair ? (Color){200, 60, 60, 255} : (Color){150, 40, 40, 255});
            int tw3 = MeasureText("Sair para o Menu", 18);
            DrawText("Sair para o Menu", 400 - tw3/2, 428, 18, WHITE);

            // Dica
            DrawText("ESC ou ENTER para retomar", 272, 475, 15, LIGHTGRAY);

            } else if (estado == EST_GAMEOVER) {
            // Fundo vermelho escuro
            ClearBackground((Color){40, 0, 0, 255});

            // Título
            int tw = MeasureText("GAME OVER", 72);
            DrawText("GAME OVER", 400 - tw/2, 220, 72, RED);

            // Linha decorativa
            DrawLine(150, 315, 650, 315, (Color){200, 50, 50, 255});

            // Mensagem
            int tw2 = MeasureText("Parabens! Você perdeu seu tempo!", 28);
            DrawText("Parabens! Você perdeu seu tempo!", 400 - tw2/2, 340, 28, LIGHTGRAY);

            // Instrução
            int tw3 = MeasureText("Pressione ENTER ou ESC para voltar ao menu", 18);
            DrawText("Pressione ENTER ou ESC para voltar ao menu", 400 - tw3/2, 520, 18, DARKGRAY);

            // Indicador de perigo piscando
            if ((int)(GetTime() * 2) % 2 == 0) {
                int tw4 = MeasureText("[ LIMITE ULTRAPASSADO ]", 22);
                DrawText("[ LIMITE ULTRAPASSADO ]", 400 - tw4/2, 430, 22, RED);
            }

        } else if (estado == EST_VITORIA) {
            // Fundo verde escuro
            ClearBackground((Color){0, 40, 0, 255});

            // Título
            int tw = MeasureText("VOCE VENCEU!", 68);
            DrawText("VOCE VENCEU!", 400 - tw/2, 210, 68, GREEN);

            // Linha decorativa
            DrawLine(150, 305, 650, 305, (Color){50, 200, 50, 255});

            // Mensagem
            int tw2 = MeasureText("Voce formou uma melancia!", 28);
            DrawText("Voce formou uma melancia!", 400 - tw2/2, 330, 28, LIGHTGRAY);

            // Instrução piscando
            if ((int)(GetTime() * 2) % 2 == 0) {
                int tw3 = MeasureText("[ MELANCIA FORMADA! ]", 22);
                DrawText("[ MELANCIA FORMADA! ]", 400 - tw3/2, 420, 22, GREEN);
            }

            // Instrução voltar
            int tw4 = MeasureText("Pressione ENTER ou ESC para voltar ao menu", 18);
            DrawText("Pressione ENTER ou ESC para voltar ao menu", 400 - tw4/2, 520, 18, DARKGRAY);

        } else if (estado == EST_LEADERBOARD) {

            DrawTexture(bg_menu, 0, 0, WHITE);

            DrawRectangle(150, 120, 500, 500, (Color){0,0,0,180});
            DrawRectangleLines(150, 120, 500, 500, WHITE);

            DrawText("LEADERBOARD", 260, 140, 35, YELLOW);

            DrawText("NOME", 220, 200, 24, WHITE);
            DrawText("PONTOS", 470, 200, 24, WHITE);

            for (int i = 0; i < total_scores && i < 10; i++) {
                DrawText(leaderboard_nomes[i], 220, 240 + i * 30, 20, WHITE);

                char scoreText[20];
                sprintf(scoreText, "%d", leaderboard_pontos[i]);

                DrawText(scoreText, 470, 240 + i * 30, 20, WHITE);
            }

            DrawText("Pressione ESC para voltar", 280, 580, 18, LIGHTGRAY);

            }else {
                ClearBackground(BLACK);
                DrawTexture(bg_menu, 0, 0, (Color){255, 255, 255, 150});    //vai ficar com o mesmo fundo do menu
                DrawRectangleLines(100, 150, 600, 600, DARKGRAY);
                if (tempo_do_limite > 0.0f && (int)(GetTime() * 4) % 2 == 0)
                    DrawLineEx((Vector2){100, 200}, (Vector2){700, 200}, 6.0f, RED);
                else
                    DrawLineEx((Vector2){100, 200}, (Vector2){700, 200}, 3.0f, (Color){255, 50, 50, 180});

                char texto_pontos[9];
                snprintf(texto_pontos, sizeof(texto_pontos), "%d", cont_pontos);
                DrawText(texto_pontos, 110, 160, 20, WHITE);

                float     raio_atual    = LISTA_FRUTAS[tipo_atual].raio;
                float     diam_atual    = raio_atual * 2;
                Texture2D tex_atual     = tex_frutas[tipo_atual];
                DrawTexturePro(tex_atual,
                    (Rectangle){0, 0, (float)tex_atual.width, (float)tex_atual.height},
                    (Rectangle){pos_x, 160, diam_atual, diam_atual},
                    (Vector2){raio_atual, raio_atual}, 0.0f, WHITE);

                for (int y = 160 + (int)raio_atual; y < 750; y += 12)
                    DrawPixel((int)pos_x, y, GRAY);

                DrawText("Proxima:", 710, 160, 14, DARKGRAY);
                float     raio_prox = LISTA_FRUTAS[tipo_prox].raio;
                float     diam_prox = raio_prox * 2;
                Texture2D tex_prox  = tex_frutas[tipo_prox];
                DrawTexturePro(tex_prox,
                    (Rectangle){0, 0, (float)tex_prox.width, (float)tex_prox.height},
                    (Rectangle){735, 200, diam_prox, diam_prox},
                    (Vector2){raio_prox, raio_prox}, 0.0f, WHITE);

                NodeFruta *atual = head;
                while (atual != NULL) {
                    cpVect pos  = cpBodyGetPosition(atual->fruta.body);
                    float  raio = LISTA_FRUTAS[atual->fruta.nivel].raio;
                    float  angulo = (float)cpBodyGetAngle(atual->fruta.body) * RAD2DEG;
                    float  diametro = raio * 2;
                    Texture2D tex;
                        if (atual->fruta.estaPodre) {
                            tex = tex_frutas_podres[atual->fruta.nivel];
                        } else {
                            tex = tex_frutas[atual->fruta.nivel];
                        }

                        DrawTexturePro(tex,
                            (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                            (Rectangle){(float)pos.x, (float)pos.y, diametro, diametro},
                            (Vector2){raio, raio}, angulo, WHITE);

                        if (atual->fruta.estaPodre) {
                            char txt[4];
                            snprintf(txt, sizeof(txt), "%d", atual->fruta.cliquesRestantes);
                            DrawText(txt, (int)pos.x - 4, (int)pos.y - 5, 10, DARKGREEN);
                        }
                    atual = atual->next;
                }
                desenharObstaculos(tex_bomba, tex_podre, tex_pimenta, tex_bloco);
            }

        EndDrawing();
    }

fechar:;
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
        if (!listaObstaculos[i].ativo) continue;
        cpSpaceRemoveShape(espaco, listaObstaculos[i].shape);
        cpShapeFree(listaObstaculos[i].shape);
        if (listaObstaculos[i].tipo != OBJ_BLOCO) {
            cpSpaceRemoveBody(espaco, listaObstaculos[i].body);
            cpBodyFree(listaObstaculos[i].body);
        }
    }

    UnloadTexture(bg_menu);
    UnloadTexture(title_menu);
    UnloadTexture(tex_play);
    UnloadTexture(tex_settings);
    UnloadTexture(tex_exit);
    UnloadTexture(tex_play_hover);
    UnloadTexture(tex_exit_hover);
    UnloadTexture(tex_settings_hover);
    UnloadTexture(tex_music_hover);
    for (int i = 0; i < NIVEIS_FRUTA; i++){
        UnloadTexture(tex_frutas[i]);
    }
    UnloadMusicStream(bossaMelon);
    UnloadMusicStream(violoncia);
    UnloadMusicStream(frutinhas);
    CloseAudioDevice();
    cpSpaceFree(espaco);
    CloseWindow();
    return 0;
}