#ifndef __FRUITS_H__
#define __FRUITS_H__

#include "chipmunk/chipmunk.h"
//#include "raylib.h"

#define MAX_FRUITS     128
#define NIVEIS_FRUTA   8
#define LARGURA_JANELA 800
#define ALTURA_JANELA  800

typedef struct Def_Fruta {
    char  *nome;
    float  raio;
    int    pontos;
} Fruta_Def;

extern const Fruta_Def LISTA_FRUTAS[NIVEIS_FRUTA];

typedef struct {
    cpBody  *body;            /* Posição e velocidade do objeto rígido        */
    cpShape *shape;           /* Colisão                                      */
    int      nivel;           /* Nível da fruta                               */
    int      fundindo;        /* Flag temporário de fusão                     */
    int      estaPodre;       /* 1 = fusão bloqueada pela fruta podre         */
    int      cliquesRestantes;/* Cliques até a fruta se recuperar             */
} Fruta;

typedef struct NodeFruta {
    Fruta fruta;
    struct NodeFruta *next;
} NodeFruta;

void       removerFruta(cpSpace *espaco, cpShape *frutaRemover, NodeFruta **head);
Fruta      criarFruta(cpSpace *espaco, float x, float y, int tipo);
NodeFruta *criarNodeFruta(Fruta fruta);
void       inserirFruta(cpSpace *espaco, float x, float y, int tipo, NodeFruta **head);
void       processarFusoes(cpSpace *espaco, NodeFruta **head);
void       atualizarFrutasPodres(NodeFruta *head);

#endif 
