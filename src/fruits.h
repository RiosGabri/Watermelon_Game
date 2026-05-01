#ifndef __FRUIT_H__ 
#define __FRUIT_H__

#include "chipmunk/chipmunk.h"
#include "raylib.h"

#define MAX_FRUITS     128
#define FRUIT_LEVELS   8


typedef struct Def_Fruta{
    float  raio;   
    Color Cor;       
    int   pontos;   
} FruitaDef;

extern const FruitDef FRUIT_DEFS[FRUIT_LEVELS];
 
/* Uma fruta no tabuleiro */
typedef struct {
    int x, y;       //Posição (coluna, linha) atual na tela
    int level;      // Nível da fruta (0-7)
    //int active; ->   Ver se existe no tabuleiro
    //int falling;  ->  Ver se estar caindo
    //int merging;  ->  Ver se pode fundir
} Fruit;
 
#endif