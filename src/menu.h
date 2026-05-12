#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum jogo {
    EST_MENU,
    EST_JOGO,
    EST_CONFIG
} Estado_Jogo;

typedef struct Botao {
    Rectangle area;

    Texture2D normal;
    Texture2D hover;

} Botao;

void desenha_botao(Botao b);

#endif
