#ifndef OBSTACULOS_H
#define OBSTACULOS_H

#include "chipmunk/chipmunk.h"
#include "raylib.h"
#include "fruits.h"

#define COLLISION_BLOCO   10
#define COLLISION_BOMBA   11
#define COLLISION_PIMENTA 12
#define COLLISION_PODRE   13

typedef enum {
    OBJ_BOMBA,
    OBJ_PIMENTA,
    OBJ_PODRE,
    OBJ_BLOCO
} TipoObjetoEspecial;

typedef struct {
    cpBody *body;
    cpShape *shape;
    TipoObjetoEspecial tipo;
    float raio;
    Rectangle areaBloco; 
    int ativo;
} ObjetoEspecial;

#define MAX_OBSTACULOS_TELA 64

extern ObjetoEspecial listaObstaculos[MAX_OBSTACULOS_TELA];
extern int qtdObstaculos;

void inicializarObstaculos(void);
void configurarCallbacksObstaculos(cpSpace *espaco);
void testarSpawnEspecial(cpSpace *espaco, NodeFruta **head);
void spawnBlocoFixo(cpSpace *espaco, NodeFruta **head);
void atualizarELimparObstaculos(cpSpace *espaco, NodeFruta **head);
void desenharObstaculos(void);

#endif