#ifndef __FRUITS_H__ 
#define __FRUITS_H__

#include "chipmunk/chipmunk.h"
#include "raylib.h"

#define MAX_FRUITS     128
#define NIVEIS_FRUTA   8

//Nesse espaço deve possuir as dimensões do jogo (janela)

typedef struct Def_Fruta{
    float  raio;   
    Color Cor;       
    int   pontos;   
} FruitaDef;

extern const FruitaDef DEFS_FRUTA[NIVEIS_FRUTA];
 
typedef struct {
    cpBody  *body;      /*Posição e velocidade do objeto rígido*/
    cpShape *shape;     /*Colisão*/  
    int      nivel;     /* Nível da fruta*/
    int      ativa;     /* 1 = existe, 0 = removida*/
    int      fundindo;  /* flag temporário de fusão*/
} Fruta;
 
#endif
