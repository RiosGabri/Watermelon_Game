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

extern const FruitaDef DEFS_FRUTA[NIVEIS_FRUTA];
 
typedef struct {
    cpBody  *body;      
    cpShape *shape;     
    int      nivel;     /* Nível da fruta*/
    int      ativa;     /* 1 = existe, 0 = removida*/
    int      fundindo;  /* flag temporário de fusão*/
} Fruta;
 
#endif
