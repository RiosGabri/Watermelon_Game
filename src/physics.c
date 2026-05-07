#include "chipmunk.h"
#include "physics.h"

#include "fruits.h"

cpSpace* initEspaco() {
    cpSpace *espaco = cpSpaceNew(); //cria um espaco e guarda o endereço dele em um ponteiro
    cpSpaceSetGravity(espaco, cpv(0, 900)); //define a gravidade do espaco
    return espaco;
}

void criarArea(cpSpace* espaco) {
    cpBody *corpoEstatico = cpSpaceGetStaticBody(espaco); //busca o corpo estatico (que nao é afetado pela fisica) dentro do espaco

    //definindo o chao:
    cpShape* chao = cpSegmentShapeNew(corpoEstatico, cpv(0,600), cpv(400,600), 0); //cria a area do chao com as cordenadas e a espessura da linha
    cpSpaceAddShape(espaco, chao); //insere o chao no espaco
    cpShapeSetFriction(chao, 0.5);

    //definindo parede esquerda:
    cpShape* paredeEsquerda = cpSegmentShapeNew(corpoEstatico, cpv(0,0), cpv(0, 600), 0);
    cpSpaceAddShape(espaco, paredeEsquerda);
    cpShapeSetFriction(paredeEsquerda, 0.8);

    //definindo parede direita:
    cpShape* paredeDireita = cpSegmentShapeNew(corpoEstatico, cpv(400,0), cpv(400, 600), 0);
    cpSpaceAddShape(espaco, paredeDireita);
    cpShapeSetFriction(paredeDireita, 0.8);
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
    return formatoFruta;
}

//fila das fusoes das frutas
FusaoPendente filaFusoes[MAX_FUSOES];
int           numFusoesPendentes = 0;

//guardar e detectar fusao
cpBool callbackFusao (cpArbiter* arbiter, cpSpace* espaco, cpDataPointer userData) {
    //arbiter = informações sobre o que colidiu
    CP_ARBITER_GET_SHAPES(arbiter, formatoA, formatoB); //busca o que colidiu e insere os valores em A e B

    for (int i = 0; i < numFusoesPendentes; i++) { //verificar se a colisao aconteceu mais de uma vez no mesmo frame
        if (filaFusoes[i].formatoA == formatoA || filaFusoes[i].formatoB == formatoB || filaFusoes[i].formatoA == formatoB || filaFusoes[i].formatoA == formatoB) {
            return cpFalse;
        }
    }

    //busca os corpos de acordo com formato correspondente para saber a posicao da fruta
    cpBody* corpoA = cpShapeGetBody(formatoA);
    cpBody* corpoB = cpShapeGetBody(formatoB);

    //busca a posicao dos corpos
    cpVect posA = cpBodyGetPosition(corpoA);
    cpVect posB = cpBodyGetPosition(corpoB);
    cpVect meio = cpvlerp(posA, posB, 0.5f); // calcula o ponto medio entre A e B

    //busca o userData de inserirFruta
    Fruta* frutaA = (Fruta*)cpShapeGetUserData(formatoA);
    Fruta* frutaB = (Fruta*)cpShapeGetUserData(formatoB);

    //marcando como fundindo:
    frutaA->fundindo = 1;
    frutaB->fundindo = 1;

    int nivelResultante = frutaA->nivel+1; //aumenta ela de nivel

    //inserir as informações na fila das fusoes pendentes
    filaFusoes[numFusoesPendentes].formatoA = formatoA;
    filaFusoes[numFusoesPendentes].formatoB = formatoB;
    filaFusoes[numFusoesPendentes].posicaoMedia = meio;
    filaFusoes[numFusoesPendentes].nivelResultante = nivelResultante;
    numFusoesPendentes++;
    return cpFalse; //diz que nao vai ter uma resposta fisica e as frutas nao vao se empurrar quando gerar a nova fruta
}

//registra as fusões e chama a funcao callback para executar a funcao
void registrarFusoes(cpSpace* espaco) {
    for (int i = 0; i < NIVEIS_FRUTA - 1; i++) {
        cpCollisionHandler *h = cpSpaceAddCollisionHandler(espaco, i, i);
        h->beginFunc = callbackFusao;
    }
}