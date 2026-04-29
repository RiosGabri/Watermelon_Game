#ifndef __FRUIT_H__ 
#define __FRUIT_H__

#include "screen.h"
//Até o momento que faço este commit, estes arquivos se encontram em fase de prototipação

#define MAX_FRUITS     128
#define FRUIT_LEVELS   8

//Caracteríticas de cada fruta
typedef struct {
    char        ch;       //Caractere visual
    int         radius;   //Raio para colisão
    screenColor fg;       //Cor do "texto"/fruta
    screenColor bg;       //Cor do fundo
    int         points;   //Pontuação de cada fruta
} FruitsDef;

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