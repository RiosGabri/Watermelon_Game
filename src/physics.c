#include <chipmunk/chipmunk.h>
#include "physics.h"
#include "fruits.h"

cpSpace* initEspaco() {
    cpSpace *espaco = cpSpaceNew(); //cria um espaco e guarda o endereço dele em um ponteiro
    cpSpaceSetGravity(espaco, cpv(0, 900)); //define a gravidade do espaco
    cpSpaceSetIterations(espaco, 20);
    return espaco;
}

void criarArea(cpSpace* espaco) {
    cpBody *corpoEstatico = cpSpaceGetStaticBody(espaco); //busca o corpo estatico (que nao é afetado pela fisica) dentro do espaco

    //definindo o chao:
    cpShape* chao = cpSegmentShapeNew(corpoEstatico, cpv(100, 750), cpv(700, 750), 2); //cria a area do chao com as cordenadas e a espessura da linha
    cpSpaceAddShape(espaco, chao);//insere o chao no espaco
    cpShapeSetFriction(chao, 0.5);
    cpShapeSetElasticity(chao, 0.1);
    cpShapeSetUserData(chao, NULL);
    cpShapeSetCollisionType(chao, 100);
    

    //definindo parede esquerda:
    
    cpShape* paredeEsquerda = cpSegmentShapeNew(corpoEstatico, cpv(100, 150), cpv(100, 750), 0);
    cpSpaceAddShape(espaco, paredeEsquerda);
    cpShapeSetFriction(paredeEsquerda, 0.8);
    cpShapeSetElasticity(paredeEsquerda, 0.1);
    cpShapeSetUserData(paredeEsquerda, NULL);
    cpShapeSetCollisionType(paredeEsquerda, 100);

    //definindo parede direita:
    cpShape* paredeDireita = cpSegmentShapeNew(corpoEstatico, cpv(700, 150), cpv(700, 750), 0);
    cpSpaceAddShape(espaco, paredeDireita);
    cpShapeSetFriction(paredeDireita, 0.8);
    cpShapeSetElasticity(paredeDireita, 0.1);
    cpShapeSetUserData(paredeDireita, NULL);
    cpShapeSetCollisionType(paredeDireita, 100);
}

cpShape* criarCorpoFruta(cpSpace* espaco, float x, float y, float raio, int tipo) {
    float restInercia = cpMomentForCircle(1.0f, 0, raio, cpvzero); //calcula a rotação do circulo quando este colide com algo
    cpBody* corpoFruta = cpBodyNew(1.0f, restInercia); //cria corpo
    cpSpaceAddBody(espaco, corpoFruta); //insere o corpo da fruta no espaço

    cpBodySetPosition(corpoFruta, cpv(x, y)); //posição da tela onde a fruta "starta"

    cpShape* formatoFruta = cpCircleShapeNew(corpoFruta, raio, cpvzero); //cria de fato o formato da fruta
    cpSpaceAddShape(espaco, formatoFruta);
    cpShapeSetFriction(formatoFruta, 0.5f); //atrito
    cpShapeSetElasticity(formatoFruta, 0.1f); //se ela pula/quica ao colidir

    cpShapeSetCollisionType(formatoFruta, tipo); //seta o formato da fruta para um tipo (como se tivesse colocando um id nela)
    cpBodySetVelocityUpdateFunc(corpoFruta, cpBodyUpdateVelocity); 
    cpSpaceSetCollisionSlop(espaco, 0.1f);
    cpShapeSetUserData(formatoFruta, NULL);
    return formatoFruta;
}

//fila das fusoes das frutas
FusaoPendente filaFusoes[MAX_FUSOES];
int           numFusoesPendentes = 0;

NodeFruta **g_head = NULL;

static void postStepFusao(cpSpace *espaco, void *key, void *data) {
    FusaoPendente *f = (FusaoPendente*)data;
    if (g_head) {
        removerFruta(espaco, f->formatoA, g_head);
        removerFruta(espaco, f->formatoB, g_head);
        if (f->nivelResultante < NIVEIS_FRUTA)
            inserirFruta(espaco, f->posicaoMedia.x, f->posicaoMedia.y, f->nivelResultante, g_head);
    }
    free(f);
}

//guardar e detectar fusao
cpBool callbackFusao (cpArbiter* arbiter, cpSpace* espaco, cpDataPointer userData) {
    //arbiter = informações sobre o que colidiu
    CP_ARBITER_GET_SHAPES(arbiter, formatoA, formatoB); //busca o que colidiu e insere os valores em A e B

    cpCollisionType tipoA = cpShapeGetCollisionType(formatoA);
    cpCollisionType tipoB = cpShapeGetCollisionType(formatoB);
    if (tipoA >= NIVEIS_FRUTA || tipoB >= NIVEIS_FRUTA) return cpTrue;

    Fruta* frutaA = (Fruta*)cpShapeGetUserData(formatoA);
    Fruta* frutaB = (Fruta*)cpShapeGetUserData(formatoB);
    if (!frutaA || !frutaB) return cpTrue;
    if (frutaA->fundindo || frutaB->fundindo) return cpTrue;

    frutaA->fundindo = 1;
    frutaB->fundindo = 1;

    FusaoPendente *f = (FusaoPendente*)malloc(sizeof(FusaoPendente));
    f->formatoA = formatoA;
    f->formatoB = formatoB;
    f->posicaoMedia = cpvlerp(
        cpBodyGetPosition(cpShapeGetBody(formatoA)),
        cpBodyGetPosition(cpShapeGetBody(formatoB)), 0.5f);
    f->nivelResultante = frutaA->nivel + 1;
    cpSpaceAddPostStepCallback(espaco, postStepFusao, formatoA, f);

    return cpFalse; //diz que nao vai ter uma resposta fisica e as frutas nao vao se empurrar quando gerar a nova fruta
}

//registra as fusões e chama a funcao callback para executar a funcao
void registrarFusoes(cpSpace* espaco) {
    for (int i = 0; i < NIVEIS_FRUTA - 1; i++) {
        cpCollisionHandler *h = cpSpaceAddCollisionHandler(espaco, i, i);
        h->beginFunc = callbackFusao;
    }
}