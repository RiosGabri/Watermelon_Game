#include <chipmunk/chipmunk.h>
#include "physics.h"
#include "fruits.h"
#include <stdlib.h>

FusaoPendente filaFusoes[MAX_FUSOES];
int numFusoesPendentes = 0;
NodeFruta **g_head = NULL;


cpSpace *initEspaco(void) {
    cpSpace *espaco = cpSpaceNew();
    cpSpaceSetGravity(espaco, cpv(0, 900));
    cpSpaceSetIterations(espaco, 20);
    return espaco;
}

void criarArea(cpSpace *espaco) {
    cpBody *corpoEstatico = cpSpaceGetStaticBody(espaco);

    cpShape *chao = cpSegmentShapeNew(corpoEstatico, cpv(100, 750), cpv(700, 750), 2);
    cpSpaceAddShape(espaco, chao);
    cpShapeSetFriction(chao, 0.5);
    cpShapeSetElasticity(chao, 0.1);
    cpShapeSetUserData(chao, NULL);
    cpShapeSetCollisionType(chao, 100);

    cpShape *paredeEsquerda = cpSegmentShapeNew(corpoEstatico, cpv(100, 150), cpv(100, 750), 2);
    cpSpaceAddShape(espaco, paredeEsquerda);
    cpShapeSetFriction(paredeEsquerda, 0.5);
    cpShapeSetElasticity(paredeEsquerda, 0.1);
    cpShapeSetUserData(paredeEsquerda, NULL);
    cpShapeSetCollisionType(paredeEsquerda, 101);

    cpShape *paredeDireita = cpSegmentShapeNew(corpoEstatico, cpv(700, 150), cpv(700, 750), 2);
    cpSpaceAddShape(espaco, paredeDireita);
    cpShapeSetFriction(paredeDireita, 0.5);
    cpShapeSetElasticity(paredeDireita, 0.1);
    cpShapeSetUserData(paredeDireita, NULL);
    cpShapeSetCollisionType(paredeDireita, 101);
}

static void postStepFusao(cpSpace *espaco, cpDataPointer key, cpDataPointer data) {
    FusaoPendente *f = (FusaoPendente*)key;
    if (numFusoesPendentes < MAX_FUSOES)
        filaFusoes[numFusoesPendentes++] = *f;
    free(f);
}

cpBool callbackFusao(cpArbiter *arbiter, cpSpace *espaco, cpDataPointer userData) {
    CP_ARBITER_GET_SHAPES(arbiter, formatoA, formatoB);

    cpVect posA = cpBodyGetPosition(cpShapeGetBody(formatoA));
    cpVect posB = cpBodyGetPosition(cpShapeGetBody(formatoB));
    if (posA.y > posB.y || (posA.y == posB.y && posA.x > posB.x))
        return cpTrue;

    cpCollisionType tipoA = cpShapeGetCollisionType(formatoA);
    cpCollisionType tipoB = cpShapeGetCollisionType(formatoB);
    if (tipoA >= NIVEIS_FRUTA || tipoB >= NIVEIS_FRUTA) return cpTrue;

    Fruta *frutaA = (Fruta*)cpShapeGetUserData(formatoA);
    Fruta *frutaB = (Fruta*)cpShapeGetUserData(formatoB);
    if (!frutaA || !frutaB)           return cpTrue;
    if (frutaA->fundindo || frutaB->fundindo) return cpTrue;
    if (frutaA->estaPodre || frutaB->estaPodre) return cpTrue;

    frutaA->fundindo = 1;
    frutaB->fundindo = 1;

    FusaoPendente *f = (FusaoPendente*)malloc(sizeof(FusaoPendente));
    f->formatoA = formatoA;
    f->formatoB = formatoB;
    f->posicaoMedia = cpvlerp(posA, posB, 0.5f);
    f->nivelResultante = (int)tipoA + 1;
    cpSpaceAddPostStepCallback(espaco, postStepFusao, f, NULL);
    return cpTrue;
}

void registrarFusoes(cpSpace *espaco) {
    for (int i = 0; i < NIVEIS_FRUTA; i++) {
        cpCollisionHandler *h = cpSpaceAddCollisionHandler(espaco, i, i);
        h->beginFunc = callbackFusao;
    }
}