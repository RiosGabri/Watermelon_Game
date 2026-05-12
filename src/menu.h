#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum jogo {
    EST_MENU,
    EST_JOGO,
    EST_CONFIG
} Estado_Jogo;

void desenha_botao(Rectangle ret, const char *texto, Color cor_base, Color cor_hover);
int foi_clicado(Rectangle ret);

#endif
