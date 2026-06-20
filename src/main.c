#include "raylib.h"
#include "menu.h"
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include "physics.h"
#include "fruits.h" 
#include "obstaculos.h"
#include <string.h>
#include <math.h>

#define Largura 800
#define Altura  800

#undef LARGURA_JANELA
#undef ALTURA_JANELA

#define LARGURA_JANELA 1200
#define ALTURA_JANELA  900
#define RGB(r, g, b) (Color){r, g, b, 255}
#define DELAY_MELANCIA 3.0f
#define BONUS_TEMPO_JANELA   60.0f
#define BONUS_PONTOS_SEGUNDO 7 // recalibrado junto com LISTA_FRUTAS (~1.34x do valor antigo: 5)
#define TEMPO_LIMITE_PARTIDA 600.0f // 10 minutos: partida acaba quando tempoPartida atinge este valor
#define VELOCIDADE_ASSENTAMENTO_SQ 6400.0f // ~80px/s ao quadrado; mais tolerante a tremores residuais da física
#define TIMEOUT_ESPERA_QUEDA 1.0f // segundos; depois disso libera o clique mesmo sem "assentar" perfeitamente

#ifdef _WIN32
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) { return _stat(path, buffer); }
#endif

static const char *caminhoLeaderboard(void) {
    static char caminho[512] = {0};
    if (caminho[0] == '\0') {
        snprintf(caminho, sizeof(caminho), "%sleaderboard.txt", GetApplicationDirectory());
    }
    return caminho;
}

void salvarPlacar(const char *nome, int pontos, float tempo) {
    FILE *f = fopen(caminhoLeaderboard(), "a");
    if (f) {
        fprintf(f, "%s,%d,%.2f\n", nome, pontos, tempo);
        fclose(f);
    }
}

void mostrarLeaderboard(char nomes[][11], int pontos[], float tempos[], int *qtd, int capacidade) {
    FILE *f = fopen(caminhoLeaderboard(), "r");

    *qtd = 0;

    if (f != NULL) {
        char linha[64];
        while (fgets(linha, sizeof(linha), f) != NULL && *qtd < capacidade) {
            int camposNovo = sscanf(linha, "%10[^,],%d,%f",
                                     nomes[*qtd], &pontos[*qtd], &tempos[*qtd]);
            if (camposNovo == 3) {
                (*qtd)++;
                continue;
            }
            int camposAntigo = sscanf(linha, "%10[^,],%d", nomes[*qtd], &pontos[*qtd]);
            if (camposAntigo == 2) {
                tempos[*qtd] = -1.0f; 
                (*qtd)++;
            }
        }
        fclose(f);


        for (int i = 0; i < *qtd - 1; i++) {
            for (int j = i + 1; j < *qtd; j++) {
                int troca = 0;

                if (pontos[j] > pontos[i]) {
                    troca = 1;
                } else if (pontos[j] == pontos[i]) {
                    float tempoI = (tempos[i] >= 0.0f) ? tempos[i] : 1e9f;
                    float tempoJ = (tempos[j] >= 0.0f) ? tempos[j] : 1e9f;
                    if (tempoJ < tempoI) troca = 1;
                }

                if (troca) {
                    int tempPontos = pontos[i];
                    pontos[i] = pontos[j];
                    pontos[j] = tempPontos;

                    float tempTempo = tempos[i];
                    tempos[i] = tempos[j];
                    tempos[j] = tempTempo;

                    char tempNome[11];
                    strcpy(tempNome, nomes[i]);
                    strcpy(nomes[i], nomes[j]);
                    strcpy(nomes[j], tempNome);
                }
            }
        }
    }
}

void resetarPartida(cpSpace *espaco, NodeFruta **head, int *tipo_atual, int *tipo_prox,
                     float *pos_x, int *pode_soltar, int *contadorCliques,
                     float *tempo_do_limite, int *tempoEsgotado, int *vitorias,
                     float *freezeMelancia, int *cont_pontos, float *tempoPartida,
                     int *placar_salvo) {
    while (*head != NULL) {
        NodeFruta *tmp = *head;
        *head = tmp->next;
        cpSpaceRemoveShape(espaco, tmp->fruta.shape);
        cpShapeFree(tmp->fruta.shape);
        cpSpaceRemoveBody(espaco, tmp->fruta.body);
        cpBodyFree(tmp->fruta.body);
        free(tmp);
    }

    inicializarObstaculos(espaco);
    *tipo_atual      = GetRandomValue(0, 3);
    *tipo_prox       = GetRandomValue(0, 3);
    *pos_x           = Largura / 2.0f;
    *pode_soltar     = 1;
    *contadorCliques = 0;
    *tempo_do_limite = 0.0f;
    *tempoEsgotado   = 0;
    *vitorias        = 0;
    *freezeMelancia  = 0.0f;
    *cont_pontos     = 0;
    *tempoPartida    = 0.0f;
    *placar_salvo    = 0;
}

int main(void) {
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, "Watermelon Game");
    
    // Cria a "tela virtual" de 800x800 onde o jogo realmente vai acontecer
    RenderTexture2D telaVirtual = LoadRenderTexture(Largura, Altura);

    SetMouseOffset(-(LARGURA_JANELA - Largura) / 2, -(ALTURA_JANELA - Altura) / 2);

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
    SetMusicVolume(bossaMelon, 1.5f); 
    SetMusicVolume(violoncia, 1.5f); 
    SetMusicVolume(frutinhas, 1.5f); 
    PlayMusicStream(bossaMelon);

    cpSpace *espaco = initEspaco();
    criarArea(espaco);
    registrarFusoes(espaco);

    inicializarObstaculos(espaco);
    configurarCallbacksObstaculos(espaco);

    NodeFruta *head = NULL;
    g_head = &head;

    Estado_Jogo estado       = EST_MENU;
    int   tipo_atual         = GetRandomValue(0, 3);
    int   tipo_prox          = GetRandomValue(0, 3);
    float pos_x              = Largura / 2.0f;
    int   pode_soltar        = 1;
    cpShape *shapeUltimaSolta = NULL; 
    float tempoDesdeSolta     = 0.0f;
    int   contadorCliques    = 0;
    int musica_selecionada = 0;
    float volume_musica = 1.0f;
    float tempo_do_limite = 0.0f;
    int vitorias = 0;
    float freezeMelancia = 0.0f;
    float tempoPartida = 0.0f;
    int tempoEsgotado = 0; 
    char player[11] = {0};
    int placar_salvo = 0;

    char leaderboard_nomes[10][11];    
    int leaderboard_pontos[10];
    float leaderboard_tempos[10];
    int total_scores = 0;
    Rectangle btnLeaderboard = {20, 760, 120, 30};

    while (!WindowShouldClose()) {
    Music *musicas[3] = {&bossaMelon, &violoncia, &frutinhas};
    UpdateMusicStream(*musicas[musica_selecionada]);
    UpdateMusicStream(bossaMelon); 

        Vector2 mPos = GetMousePosition();
        bool mouseNaTela = (mPos.x >= 0 && mPos.x <= Largura && mPos.y >= 0 && mPos.y <= Altura);

        switch (estado) {
            case EST_MENU:
                if (foi_clicado(btnPlay)) {
                    estado         = EST_NOME;
                    tipo_atual     = GetRandomValue(0, 3);
                    tipo_prox      = GetRandomValue(0, 3);
                    pos_x          = Largura / 2.0f;
                    pode_soltar    = 1;
                    contadorCliques = 0;
                    inicializarObstaculos(espaco);
                    StopMusicStream(bossaMelon);
                    PlayMusicStream(*musicas[musica_selecionada]);
                }
                if (foi_clicado(btnExit)) goto fechar;
                if (foi_clicado(btnSettings)) estado = EST_CONFIGURACAO;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseNaTela) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, btnLeaderboard)) {
                        mostrarLeaderboard(
                            leaderboard_nomes,
                            leaderboard_pontos,
                            leaderboard_tempos,
                            &total_scores,
                            10
                        );
                        estado = EST_LEADERBOARD;
                    }
                }
                
                break;
            case EST_CONFIGURACAO:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EST_MENU; 
                    StopMusicStream(violoncia);
                    StopMusicStream(frutinhas);
                    PlayMusicStream(bossaMelon);
                }

                if (IsKeyPressed(KEY_LEFT)) {
                    musica_selecionada--;
                    if (musica_selecionada < 0) musica_selecionada = 2;
                }
                if (IsKeyPressed(KEY_RIGHT)) {
                    musica_selecionada++;
                    if (musica_selecionada > 2) musica_selecionada = 0;
                }

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
                        resetarPartida(espaco, &head, &tipo_atual, &tipo_prox, &pos_x,
                                       &pode_soltar, &contadorCliques, &tempo_do_limite,
                                       &tempoEsgotado, &vitorias, &freezeMelancia,
                                       &cont_pontos, &tempoPartida, &placar_salvo);
                        memset(player, 0, sizeof(player));

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
                    salvarPlacar(player, cont_pontos, tempoPartida);
                    placar_salvo = 1;
                }

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                    // Resetar o jogo
                    resetarPartida(espaco, &head, &tipo_atual, &tipo_prox, &pos_x,
                                   &pode_soltar, &contadorCliques, &tempo_do_limite,
                                   &tempoEsgotado, &vitorias, &freezeMelancia,
                                   &cont_pontos, &tempoPartida, &placar_salvo);
                    estado          = EST_MENU;
                    PlayMusicStream(bossaMelon);
                }
                break;
            
            case EST_NOME: 
                int input = GetCharPressed();

                while (input > 0) {
                    if (strlen(player) < 10 && input >= ' ' && input != ',') {
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

                if (IsKeyPressed(KEY_ENTER) && strlen(player) > 0) {
                    estado = EST_JOGO;
                    resetarPartida(espaco, &head, &tipo_atual, &tipo_prox, &pos_x,
                                   &pode_soltar, &contadorCliques, &tempo_do_limite,
                                   &tempoEsgotado, &vitorias, &freezeMelancia,
                                   &cont_pontos, &tempoPartida, &placar_salvo);
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

                if (!vitorias && tempoPartida < TEMPO_LIMITE_PARTIDA) tempoPartida += GetFrameTime();
                if (!vitorias && tempoPartida >= TEMPO_LIMITE_PARTIDA) {
                    tempoPartida = TEMPO_LIMITE_PARTIDA;
                    tempoEsgotado = 1;
                    estado = EST_GAMEOVER;
                    StopMusicStream(violoncia);
                    StopMusicStream(frutinhas);
                }

                pos_x = GetMousePosition().x;
                if (pos_x < 115) pos_x = 115;
                if (pos_x > 685) pos_x = 685;

                // Verificação 'mouseNaTela' adicionada para impedir disparos acidentais nas bordas pretas
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pode_soltar && freezeMelancia <= 0.0f && mouseNaTela) {
                    inserirFruta(espaco, pos_x, 160, tipo_atual, &head);
                    tipo_atual  = tipo_prox;
                    tipo_prox   = GetRandomValue(0, 3);
                    pode_soltar = 0;
                    shapeUltimaSolta = head->fruta.shape; // 'head' é exatamente a fruta que acabamos de inserir
                    tempoDesdeSolta  = 0.0f;
                    atualizarFrutasPodres(head);
                    contadorCliques++;

                    if (contadorCliques >= 10) {
                        spawnBlocoFixo(espaco, &head);
                        contadorCliques = 0;
                    } else {
                        testarSpawnEspecial(espaco, &head);
                    }
                }
                if (!pode_soltar) {
                    tempoDesdeSolta += GetFrameTime();
                    NodeFruta *frutaSolta = NULL;
                    NodeFruta *busca = head;
                    while (busca != NULL) {
                        if (busca->fruta.shape == shapeUltimaSolta) { frutaSolta = busca; break; }
                        busca = busca->next;
                    }

                    if (frutaSolta == NULL || tempoDesdeSolta >= TIMEOUT_ESPERA_QUEDA) {
                        pode_soltar = 1;
                    } else {
                        cpVect pos = cpBodyGetPosition(frutaSolta->fruta.body);
                        cpVect vel = cpBodyGetVelocity(frutaSolta->fruta.body);
                        float velQuadrada = (float)(vel.x * vel.x + vel.y * vel.y);
                        if (pos.y > 200 && velQuadrada < VELOCIDADE_ASSENTAMENTO_SQ) {
                            pode_soltar = 1;
                        }
                    }
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
                if (freezeMelancia <= 0.0f) {
                    if (fruta_no_limite) {
                        tempo_do_limite += GetFrameTime();
                        if (tempo_do_limite >= 3.0f) {
                            tempoEsgotado = 0;
                            estado = EST_GAMEOVER;
                            StopMusicStream(violoncia);
                            StopMusicStream(frutinhas);
                        }
                    } else {
                        tempo_do_limite = 0.0f;
                    }
                }

                if (!vitorias) {
                    NodeFruta *v = head;
                    while (v != NULL) {
                        if (v->fruta.nivel == NIVEIS_FRUTA - 1) { // melancia = ultimo nível
                            vitorias       = 1;
                            freezeMelancia = DELAY_MELANCIA;

                            int bonusVelocidade = (int)(fmaxf(0.0f, BONUS_TEMPO_JANELA - tempoPartida)
                                                         * BONUS_PONTOS_SEGUNDO);
                            cont_pontos += bonusVelocidade;
                            break;
                        }
                        v = v->next;
                    }
                }

                if (freezeMelancia > 0.0f) {
                    freezeMelancia -= GetFrameTime();
                    if (freezeMelancia <= 0.0f) {
                        freezeMelancia = 0.0f;
                        estado = EST_VITORIA;
                        StopMusicStream(violoncia);
                        StopMusicStream(frutinhas);
                    }
                }

                break;
        }

        BeginTextureMode(telaVirtual);
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
                DrawRectangleLinesEx((Rectangle){100, 150, 600, 600}, 5, RGB(255, 170, 98));

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

                DrawRectangle(0, 0, Largura, Altura, (Color){0, 0, 0, 150});

                DrawRectangle(250, 260, 300, 230, (Color){0, 0, 0, 200});
                DrawRectangleLines(250, 260, 300, 230, WHITE);

                int tw = MeasureText("PAUSADO", 36);
                DrawText("PAUSADO", 400 - tw/2, 280, 36, WHITE);
                DrawLine(270, 330, 530, 330, WHITE);

                bool hover_retomar = CheckCollisionPointRec(GetMousePosition(), (Rectangle){300, 345, 200, 50});
                DrawRectangle(300, 345, 200, 50,
                    hover_retomar ? (Color){80, 200, 80, 255} : (Color){50, 150, 50, 255});
                int tw2 = MeasureText("Retomar", 22);
                DrawText("Retomar", 400 - tw2/2, 358, 22, WHITE);

                bool hover_sair = CheckCollisionPointRec(GetMousePosition(), (Rectangle){300, 415, 200, 50});
                DrawRectangle(300, 415, 200, 50,
                    hover_sair ? (Color){200, 60, 60, 255} : (Color){150, 40, 40, 255});
                int tw3 = MeasureText("Sair para o Menu", 18);
                DrawText("Sair para o Menu", 400 - tw3/2, 428, 18, WHITE);

                DrawText("ESC ou ENTER para retomar", 272, 475, 15, LIGHTGRAY);

            } else if (estado == EST_GAMEOVER) {
                ClearBackground((Color){40, 0, 0, 255});

                int tw = MeasureText("GAME OVER", 72);
                DrawText("GAME OVER", 400 - tw/2, 220, 72, RED);

                DrawLine(150, 315, 650, 315, (Color){200, 50, 50, 255});

                int tw2 = MeasureText("Parabens! Você perdeu seu tempo!", 28);
                DrawText("Parabens! Você perdeu seu tempo!", 400 - tw2/2, 340, 28, LIGHTGRAY);

                int tw3 = MeasureText("Pressione ENTER ou ESC para voltar ao menu", 18);
                DrawText("Pressione ENTER ou ESC para voltar ao menu", 400 - tw3/2, 520, 18, DARKGRAY);

                const char *msgMotivo = tempoEsgotado ? "[ TEMPO ESGOTADO ]" : "[ LIMITE ULTRAPASSADO ]";
                if ((int)(GetTime() * 2) % 2 == 0) {
                    int tw4 = MeasureText(msgMotivo, 22);
                    DrawText(msgMotivo, 400 - tw4/2, 430, 22, RED);
                }

            } else if (estado == EST_VITORIA) {
                ClearBackground((Color){0, 40, 0, 255});

                int tw = MeasureText("VOCE VENCEU!", 68);
                DrawText("VOCE VENCEU!", 400 - tw/2, 210, 68, GREEN);

                DrawLine(150, 305, 650, 305, (Color){50, 200, 50, 255});

                int tw2 = MeasureText("Voce formou uma melancia!", 28);
                DrawText("Voce formou uma melancia!", 400 - tw2/2, 330, 28, LIGHTGRAY);

                if ((int)(GetTime() * 2) % 2 == 0) {
                    int tw3 = MeasureText("[ MELANCIA FORMADA! ]", 22);
                    DrawText("[ MELANCIA FORMADA! ]", 400 - tw3/2, 420, 22, GREEN);
                }

                int tw4 = MeasureText("Pressione ENTER ou ESC para voltar ao menu", 18);
                DrawText("Pressione ENTER ou ESC para voltar ao menu", 400 - tw4/2, 520, 18, DARKGRAY);

            } else if (estado == EST_LEADERBOARD) {
                DrawTexture(bg_menu, 0, 0, WHITE);

                // Painel Central Expandido e Alinhado
                DrawRectangle(100, 100, 600, 400, (Color){0, 0, 0, 180});
                DrawRectangleLines(100, 100, 600, 400, WHITE);

                DrawText("[ LEADERBOARD ]", 290, 120, 26, YELLOW);

                int colX_Pos = 140;
                int colX_Nome = 220;
                int colX_ScoreRight = 500; 
                int colX_TempoRight = 660; 

                DrawText("POS", colX_Pos, 160, 20, WHITE);
                DrawText("NOME", colX_Nome, 160, 20, WHITE);
                DrawText("SCORE", 420, 160, 20, WHITE); 
                DrawText("TEMPO", 590, 160, 20, WHITE); 

                DrawLine(120, 190, 680, 190, WHITE);

                for (int i = 0; i < total_scores && i < 10; i++) {
                    int yPos = 210 + i * 28;
                    
                    char posTxt[10];
                    snprintf(posTxt, sizeof(posTxt), "%02dº", i + 1);
                    DrawText(posTxt, colX_Pos, yPos, 20, WHITE);

                    DrawText(leaderboard_nomes[i], colX_Nome, yPos, 20, WHITE);

                    char scoreText[20];
                    snprintf(scoreText, sizeof(scoreText), "%d", leaderboard_pontos[i]);
                    int scoreWidth = MeasureText(scoreText, 20);
                    DrawText(scoreText, colX_ScoreRight - scoreWidth, yPos, 20, WHITE);

                    char tempoText[20];
                    if (leaderboard_tempos[i] >= 0.0f) {
                        int mm = (int)leaderboard_tempos[i] / 60;
                        int ss = (int)leaderboard_tempos[i] % 60;
                        snprintf(tempoText, sizeof(tempoText), "%02d:%02d", mm, ss);
                    } else {
                        strcpy(tempoText, "--:--");
                    }
                    int tempoWidth = MeasureText(tempoText, 20);
                    DrawText(tempoText, colX_TempoRight - tempoWidth, yPos, 20, WHITE);
                }

                int twFoot = MeasureText("[ESC] Voltar Menu", 18);
                DrawText("[ESC] Voltar Menu", 400 - twFoot/2, 520, 18, LIGHTGRAY);

            } else {
                ClearBackground(BLACK);
                DrawTexture(bg_menu, 0, 0, (Color){255, 255, 255, 150});    
                DrawRectangleLinesEx((Rectangle){100, 150, 600, 600}, 5, RGB(255, 170, 98));
                if (tempo_do_limite > 0.0f && (int)(GetTime() * 4) % 2 == 0)
                    DrawLineEx((Vector2){100, 200}, (Vector2){700, 200}, 6.0f, RED);
                else
                    DrawLineEx((Vector2){100, 200}, (Vector2){700, 200}, 3.0f, (Color){255, 50, 50, 180});

                char texto_pontos[9];
                snprintf(texto_pontos, sizeof(texto_pontos), "%d", cont_pontos);
                DrawText(texto_pontos, 110, 160, 20, WHITE);

                float tempoRestante = TEMPO_LIMITE_PARTIDA - tempoPartida;
                if (tempoRestante < 0.0f) tempoRestante = 0.0f;
                int mmAtual = (int)tempoRestante / 60;
                int ssAtual = (int)tempoRestante % 60;
                char texto_tempo[8];
                snprintf(texto_tempo, sizeof(texto_tempo), "%02d:%02d", mmAtual, ssAtual);
                DrawText(texto_tempo, 110, 182, 16, LIGHTGRAY);

                float     raio_atual    = LISTA_FRUTAS[tipo_atual].raio;
                float     diam_atual    = raio_atual * 2;
                Texture2D tex_atual     = tex_frutas[tipo_atual];
                DrawTexturePro(tex_atual,
                    (Rectangle){0, 0, (float)tex_atual.width, (float)tex_atual.height},
                    (Rectangle){pos_x, 160, diam_atual, diam_atual},
                    (Vector2){raio_atual, raio_atual}, 0.0f, WHITE);

                for (int y = 160 + (int)raio_atual; y < 750; y += 12)
                    DrawPixel((int)pos_x, y, GRAY);

                DrawText("Proxima:", 710, 160, 15, WHITE);
                float     raio_prox = LISTA_FRUTAS[tipo_prox].raio;
                float     diam_prox = raio_prox * 2;
                Texture2D tex_prox  = tex_frutas[tipo_prox];
                DrawTexturePro(tex_prox,
                    (Rectangle){0, 0, (float)tex_prox.width, (float)tex_prox.height},
                    (Rectangle){735, 220, diam_prox, diam_prox},
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

                if (freezeMelancia > 0.0f) {
                    DrawRectangle(0, 0, Largura, Altura, (Color){0, 0, 0, 130});

                    const char *msg = "MELANCIA FORMADA!";
                    int twMsg = MeasureText(msg, 38);
                    DrawText(msg, 400 - twMsg/2, 320, 38, YELLOW);

                    int segsRestantes = (int)ceilf(freezeMelancia);
                    char txtCount[8];
                    snprintf(txtCount, sizeof(txtCount), "%d", segsRestantes);
                    int twCount = MeasureText(txtCount, 60);
                    DrawText(txtCount, 400 - twCount/2, 380, 60, WHITE);
                }
            }

        EndTextureMode();
        BeginDrawing();
            ClearBackground(BLACK); // Borda preta

            DrawTexturePro(
                telaVirtual.texture,
                // Eixo Y precisa ser invertido para texturas virtuais no Raylib
                (Rectangle){ 0.0f, 0.0f, (float)telaVirtual.texture.width, -(float)telaVirtual.texture.height },
                // Calcula automaticamente o centro para o ecrã real
                (Rectangle){ (LARGURA_JANELA - Largura) / 2.0f, (ALTURA_JANELA - Altura) / 2.0f, (float)Largura, (float)Altura },
                (Vector2){ 0, 0 }, 
                0.0f, 
                WHITE
            );
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

    UnloadRenderTexture(telaVirtual); // Limpa a memória da tela virtual

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