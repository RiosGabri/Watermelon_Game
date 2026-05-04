#ifndef __MENU_H__
#define __MENU_H__
 
#include "raylib.h"

typedef enum menu{
    Menu_Aguardando,
    Menu_Jogar,
    Menu_Sair
} Estado_Menu;

typedef enum painel{
    Painel_Nenhum,
    Painel_Configuracoes,
    Painel_Musicas
} Painel_Ativo;

typedef struct configuracoes{
    float Volume_Musica;     
    int   Largura_Janela;
    int   Altura_Janela;
} Configuracoes;

#endif