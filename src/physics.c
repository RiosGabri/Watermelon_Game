#include "chipmunk.h"

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