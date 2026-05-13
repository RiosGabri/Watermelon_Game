#ifndef __FRUITS_H__ 
#define __FRUITS_H__

#include "chipmunk/chipmunk.h"
//#include "raylib.h"

#define MAX_FRUITS     128
#define NIVEIS_FRUTA   8

//Nesse espaço deve possuir as dimensões do jogo (janela)

typedef struct Def_Fruta{
    char *nome;
    float  raio;
    int   pontos;

    //Color Cor;
    //textura
} Fruta_Def;

extern const Fruta_Def LISTA_FRUTAS[NIVEIS_FRUTA];
 
typedef struct {
    cpBody  *body;      /*Posição e velocidade do objeto rígido*/
    cpShape *shape;     /*Colisão*/  
    int      nivel;     /* Nível da fruta*/
    //int ativa removido pois essa parte é verificada atraves de nodeFruta
    int      fundindo;  /* flag temporário de fusão*/
} Fruta;

typedef struct NodeFruta { //struct node fruta para a lista encadeada
    Fruta fruta;
    struct NodeFruta *next;
}NodeFruta;
 
#endif

Fruta criarFruta(cpSpace *espaco, float x, float y, int tipo);
NodeFruta* criarNodeFruta(Fruta fruta);
void inserirFruta(cpSpace* espaco, float x, float y, int tipo, NodeFruta** head);
void processarFusoes(cpSpace* espaco, NodeFruta **head);