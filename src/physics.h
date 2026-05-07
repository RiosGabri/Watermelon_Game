#include "chipmunk.h"

cpSpace* initEspaco();
void criarArea(cpSpace* espaco);
cpShape* criarCorpoFruta(cpSpace* espaco, float x, float y, float raio, int tipo);

typedef struct { //struct para guardar as fusoes que vao ser formadas (que estao na fila)
    cpShape *formatoA;
    cpShape *formatoB;
    cpVect   posicaoMedia;
    int      nivelResultante;
} FusaoPendente;

#define MAX_FUSOES 32

extern FusaoPendente filaFusoes[];
extern int           numFusoesPendentes;

cpBool callbackFusao (cpArbiter* arbiter, cpSpace* espaco, cpDataPointer userData);
void registrarFusoes(cpSpace* espaco);