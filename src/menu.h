#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include <stdbool.h>

typedef enum jogo {
    EST_MENU,
    EST_MUSICA,
    EST_JOGO
} Estado_Jogo;

typedef struct Botao {
    Rectangle area;
    Texture2D textura;
    Texture2D textura_hover;
} Botao;

bool foi_clicado(Botao b);
void desenha_botao(Botao b);

#endif
