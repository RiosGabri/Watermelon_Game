#include "obstaculos.h"
#include <stdlib.h>
#include <math.h>

ObjetoEspecial listaObstaculos[MAX_OBSTACULOS_TELA];
int qtdObstaculos = 0;

static cpShape *shapesParaRemover[MAX_OBSTACULOS_TELA];
static int qtdRemoverShapes = 0;

static cpVect fusoesBombaPos[MAX_OBSTACULOS_TELA];
static int qtdBombasParaExplodir = 0;

void inicializarObstaculos(void) {
    qtdObstaculos = 0;
    qtdRemoverShapes = 0;
    qtdBombasParaExplodir = 0;
}

static void agendarRemocaoObstaculo(cpShape *shape) {
    for(int i = 0; i < qtdRemoverShapes; i++) {
        if(shapesParaRemover[i] == shape) return;
    }
    shapesParaRemover[qtdRemoverShapes++] = shape;
}

static cpBool cbPimentaFruta(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    CP_ARBITER_GET_SHAPES(arb, shapePimenta, shapeFruta);
    Fruta *f = (Fruta*)cpShapeGetUserData(shapeFruta);
    if (f) {
        cpBody *bodyFruta = cpShapeGetBody(shapeFruta);
        cpVect vel = cpBodyGetVelocity(bodyFruta);
        //Pimenta aumenta velocidade e faz fruta seguir para direção aleatório (ainda tô testando)
        vel = cpvmult(vel, 3.5f);
        vel = cpvadd(vel, cpv((rand() % 400) - 200, -300)); 
        cpBodySetVelocity(bodyFruta, vel);
    }
    agendarRemocaoObstaculo(shapePimenta);
    return cpTrue;
}

//Fruta Podre bate em Fruta (Corrompe e Some) -> ainda tô testando
static cpBool cbPodreFruta(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    CP_ARBITER_GET_SHAPES(arb, shapePodre, shapeFruta);
    Fruta *f = (Fruta*)cpShapeGetUserData(shapeFruta);
    if (f) {
        f->fundindo = 0; 
        f->nivel = f->nivel; 
        f->estaPodre = 1; 
    }
    agendarRemocaoObstaculo(shapePodre);
    return cpTrue;
}

static cpBool cbBombaAtiva(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    CP_ARBITER_GET_SHAPES(arb, shapeBomba, shapeOutro);
    cpBody *bodyBomba = cpShapeGetBody(shapeBomba);
    cpVect pos = cpBodyGetPosition(bodyBomba);
    fusoesBombaPos[qtdBombasParaExplodir++] = pos;
    agendarRemocaoObstaculo(shapeBomba);
    return cpTrue;
}

void configurarCallbacksObstaculos(cpSpace *espaco) {
    for (int i = 0; i < NIVEIS_FRUTA; i++) {
        cpCollisionHandler *h1 = cpSpaceAddCollisionHandler(espaco, COLLISION_PIMENTA, i);
        h1->beginFunc = cbPimentaFruta;
        
        cpCollisionHandler *h2 = cpSpaceAddCollisionHandler(espaco, COLLISION_PODRE, i);
        h2->beginFunc = cbPodreFruta;
        
        cpCollisionHandler *h3 = cpSpaceAddCollisionHandler(espaco, COLLISION_BOMBA, i);
        h3->beginFunc = cbBombaAtiva;
    }
    cpSpaceAddCollisionHandler(espaco, COLLISION_BOMBA, 100)->beginFunc = cbBombaAtiva;
}

void testarSpawnEspecial(cpSpace *espaco, NodeFruta **head) {
    int dado = GetRandomValue(1, 100);
    if (dado <= 70) return; 
    
    if (qtdObstaculos >= MAX_OBSTACULOS_TELA) return;
    
    TipoObjetoEspecial tipo;
    Color cor;
    int colType;
    float raio = 18.0f;
    if (dado > 70 && dado <= 80) {
        tipo = OBJ_BOMBA; cor = BLACK; colType = COLLISION_BOMBA;
    } else if (dado > 80 && dado <= 90) {
        tipo = OBJ_PIMENTA; cor = ORANGE; colType = COLLISION_PIMENTA;
    } else {
        tipo = OBJ_PODRE; cor = LIME; colType = COLLISION_PODRE;
    }
    
    float spawnX = (float)GetRandomValue(125, 675);
    float spawnY = 140.0f;
    
    ObjetoEspecial *obj = &listaObstaculos[qtdObstaculos++];
    obj->tipo = tipo;
    obj->raio = raio;
    obj->ativo = 1;
    
    obj->body = cpSpaceAddBody(espaco, cpBodyNew(1.0f, cpMomentForCircle(1.0f, 0, raio, cpvzero)));
    cpBodySetPosition(obj->body, cpv(spawnX, spawnY));
    
    obj->shape = cpSpaceAddShape(espaco, cpCircleShapeNew(obj->body, raio, cpvzero));
    cpShapeSetFriction(obj->shape, 0.3f);
    cpShapeSetElasticity(obj->shape, 0.4f);
    cpShapeSetCollisionType(obj->shape, colType);
    cpShapeSetUserData(obj->shape, obj);
}

void spawnBlocoFixo(cpSpace *espaco, NodeFruta **head) {
    if (qtdObstaculos >= MAX_OBSTACULOS_TELA) return;
    
    float larg = (float)GetRandomValue(60, 110);
    float alt = (float)GetRandomValue(40, 70);
    float bx = (float)GetRandomValue(200, 600);
    float by = (float)GetRandomValue(350, 600);
    
    ObjetoEspecial *obj = &listaObstaculos[qtdObstaculos++];
    obj->tipo = OBJ_BLOCO;
    obj->ativo = 1;
    obj->areaBloco = (Rectangle){bx - larg/2.0f, by - alt/2.0f, larg, alt};
    
    obj->body = cpSpaceGetStaticBody(espaco);
    cpBB bb = cpBBNew(obj->areaBloco.x, obj->areaBloco.y, obj->areaBloco.x + larg, obj->areaBloco.y + alt);
    obj->shape = cpSpaceAddShape(espaco, cpBoxShapeNew2(obj->body, bb, 0.0f));    
    cpShapeSetFriction(obj->shape, 0.6f);
    cpShapeSetCollisionType(obj->shape, COLLISION_BLOCO);
    
    NodeFruta *atual = *head;
    while (atual != NULL) {
        cpVect fPos = cpBodyGetPosition(atual->fruta.body);
        if (CheckCollisionCircleRec((Vector2){(float)fPos.x, (float)fPos.y}, LISTA_FRUTAS[atual->fruta.nivel].raio, obj->areaBloco)) {
            cpVect dir = cpvsub(fPos, cpv(bx, by));
            if (dir.x == 0 && dir.y == 0) dir = cpv(0, -1);
            cpVect impulso = cpvmult(cpvnormalize(dir), 450.0f);
            cpBodyApplyImpulseAtWorldPoint(atual->fruta.body, impulso, fPos);
        }
        atual = atual->next;
    }
}

void atualizarELimparObstaculos(cpSpace *espaco, NodeFruta **head) {
    float raioExplosao = 130.0f;
    for (int b = 0; b < qtdBombasParaExplodir; b++) {
        cpVect bPos = fusoesBombaPos[b];
        NodeFruta *atual = *head;
        while (atual != NULL) {
            NodeFruta *proximo = atual->next;
            cpVect fPos = cpBodyGetPosition(atual->fruta.body);
            float dist = (float)cpvdist(bPos, fPos);
            
            if (dist <= raioExplosao) {
                removerFruta(espaco, atual->fruta.shape, head);
            }
            atual = proximo;
        }
    }
    qtdBombasParaExplodir = 0;
    for (int i = 0; i < qtdRemoverShapes; i++) {
        cpShape *sh = shapesParaRemover[i];
        if (!sh) continue;
        for (int j = 0; j < qtdObstaculos; j++) {
            if (listaObstaculos[j].shape == sh && listaObstaculos[j].ativo) {
                listaObstaculos[j].ativo = 0;
                cpSpaceRemoveShape(espaco, sh);
                cpBody *b = cpShapeGetBody(sh);
                cpSpaceRemoveBody(espaco, b);
                cpShapeFree(sh);
                cpBodyFree(b);
                break;
            }
        }
    }
    qtdRemoverShapes = 0;
}

void desenharObstaculos(void) {
    for (int i = 0; i < qtdObstaculos; i++) {
        if (!listaObstaculos[i].ativo) continue;
        if (listaObstaculos[i].tipo == OBJ_BLOCO) {
            DrawRectangleRec(listaObstaculos[i].areaBloco, DARKGRAY);
            DrawRectangleLines(
                (int)listaObstaculos[i].areaBloco.x, 
                (int)listaObstaculos[i].areaBloco.y, 
                (int)listaObstaculos[i].areaBloco.width, 
                (int)listaObstaculos[i].areaBloco.height, 
            BLACK
        );
        } else {
            cpVect pos = cpBodyGetPosition(listaObstaculos[i].body);
            Color c = BLACK;
            char *txt = "";
            if (listaObstaculos[i].tipo == OBJ_BOMBA) { c = BLACK; txt = "BOMB"; }
            else if (listaObstaculos[i].tipo == OBJ_PIMENTA) { c = RED; txt = "PIMT"; }
            else if (listaObstaculos[i].tipo == OBJ_PODRE) { c = DARKGREEN; txt = "PODR"; }

            DrawCircle((int)pos.x, (int)pos.y, (int)listaObstaculos[i].raio, c);
            DrawCircleLines((int)pos.x, (int)pos.y, (int)listaObstaculos[i].raio, WHITE);
            DrawText(txt, (int)pos.x - 11, (int)pos.y - 4, 9, WHITE);
        }
    }
}