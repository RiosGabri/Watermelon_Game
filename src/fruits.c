#include "fruits.h"
#include <chipmunk/chipmunk.h>
#include "physics.h"
#include <stdlib.h>

const Fruta_Def LISTA_FRUTAS[NIVEIS_FRUTA] = {
    {"Uva",      15.0f,   100},
    {"Cereja",   20.0f,   150},
    {"Morango",  27.0f,   200},
    {"Maca",     35.0f,   250},
    {"Pera",     43.0f,   500},
    {"Laranja",  52.0f,   550},
    {"Abacaxi",  62.0f,   600},
    {"Melancia", 73.0f,   800}
};

Fruta criarFruta(cpSpace *espaco, float x, float y, int tipo) {
    Fruta fruta;
    float raio  = LISTA_FRUTAS[tipo].raio;
    float massa = 1.0f * (tipo + 1);

    fruta.body = cpSpaceAddBody(espaco,
                     cpBodyNew(massa, cpMomentForCircle(massa, 0, raio, cpvzero)));
    cpBodySetPosition(fruta.body, cpv(x, y));

    fruta.shape = cpSpaceAddShape(espaco, cpCircleShapeNew(fruta.body, raio, cpvzero));
    cpShapeSetFriction(fruta.shape, 0.4f);
    cpShapeSetElasticity(fruta.shape, 0.2f);
    cpShapeSetCollisionType(fruta.shape, tipo);

    fruta.nivel = tipo;
    fruta.fundindo = 0;
    fruta.estaPodre = 0;
    fruta.cliquesRestantes = 0;
    fruta.impulsionadaPorPimenta = 0;
    return fruta;
}

NodeFruta *criarNodeFruta(Fruta fruta) {
    NodeFruta *novo = (NodeFruta*)malloc(sizeof(NodeFruta));
    if (novo != NULL) {
        novo->fruta = fruta;
        novo->next  = NULL;
    }
    return novo;
}


void inserirFruta(cpSpace *espaco, float x, float y, int tipo, NodeFruta **head) {
    Fruta novaFruta = criarFruta(espaco, x, y, tipo);
    NodeFruta *novoNode  = criarNodeFruta(novaFruta);
    cpShapeSetUserData(novaFruta.shape, &(novoNode->fruta));

    novoNode->next = *head;
    *head = novoNode;
}

int removerFruta(cpSpace *espaco, cpShape *frutaRemover, NodeFruta **head) {
    if (frutaRemover == NULL || *head == NULL) return 0;
    NodeFruta *atual    = *head;
    NodeFruta *anterior = NULL;

    while (atual != NULL) {
        if (atual->fruta.shape == frutaRemover) {
            if (anterior == NULL) *head = atual->next;
            else anterior->next = atual->next;
            cpSpaceRemoveShape(espaco, atual->fruta.shape);
            cpShapeFree(atual->fruta.shape);
            cpSpaceRemoveBody(espaco, atual->fruta.body);
            cpBodyFree(atual->fruta.body);
            free(atual);
            return 1;
        }
        anterior = atual;
        atual    = atual->next;
    }
    return 0;
}

int cont_pontos = 0;

void processarFusoes(cpSpace *espaco, NodeFruta **head) {
    for (int i = 0; i < numFusoesPendentes; i++) {
        FusaoPendente *f = &filaFusoes[i];
        int removidoA = removerFruta(espaco, f->formatoA, head);
        int removidoB = removerFruta(espaco, f->formatoB, head);

        if (removidoA && removidoB && f->nivelResultante < NIVEIS_FRUTA) {
            inserirFruta(espaco, f->posicaoMedia.x, f->posicaoMedia.y, f->nivelResultante, head);
            cont_pontos += LISTA_FRUTAS[f->nivelResultante].pontos;
        }
    }
    numFusoesPendentes = 0; 
}

void atualizarFrutasPodres(NodeFruta *head) {
    NodeFruta *atual = head;
    while (atual != NULL) {
        if (atual->fruta.estaPodre) {
            atual->fruta.cliquesRestantes--;
            if (atual->fruta.cliquesRestantes <= 0) {
                atual->fruta.estaPodre        = 0;
                atual->fruta.cliquesRestantes = 0;
            }
        }
        atual = atual->next;
    }
}