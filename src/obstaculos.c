#include "obstaculos.h"
#include <stdlib.h>
#include <math.h>

#define PONTOS_PERDA_PIMENTA          150 // equivalente a perder a fusão mais barata (cereja)
#define PONTOS_PERDA_PODRE            120
#define PONTOS_PERDA_BOMBA_POR_FRUTA  100 // por fruta destruída; uma bomba bem posicionada pesa muito mais

#define PIMENTA_MULT_VELOCIDADE  5.0f  // multiplicador sobre a velocidade atual da fruta
#define PIMENTA_IMPULSO_LATERAL  350   // variação horizontal aleatória do lançamento
#define PIMENTA_IMPULSO_VERTICAL 500.0f // força do lançamento para cima
#define PIMENTA_LIMITE_Y         200.0f // mesma linha vermelha de main.c; fruta lançada não pode passar disso
#define PIMENTA_MARGEM_LIBERACAO 80.0f  // o quanto a fruta precisa cair abaixo da linha pra deixar de ser protegida/restrita

ObjetoEspecial listaObstaculos[MAX_OBSTACULOS_TELA];
int qtdObstaculos = 0;

static cpShape *shapesParaRemover[MAX_OBSTACULOS_TELA];
static int qtdRemoverShapes = 0;

static cpVect fusoesBombaPos[MAX_OBSTACULOS_TELA];
static int qtdBombasParaExplodir = 0;

static void perderPontos(int valor) {
    cont_pontos -= valor;
    if (cont_pontos < 0) cont_pontos = 0;
}


void inicializarObstaculos(cpSpace *espaco) {
    for (int i = 0; i < qtdObstaculos; i++) {
        if (!listaObstaculos[i].ativo) continue;

        cpSpaceRemoveShape(espaco, listaObstaculos[i].shape);
        cpShapeFree(listaObstaculos[i].shape);

        if (listaObstaculos[i].tipo != OBJ_BLOCO) {
            cpSpaceRemoveBody(espaco, listaObstaculos[i].body);
            cpBodyFree(listaObstaculos[i].body);
        }
        listaObstaculos[i].ativo = 0;
    }

    qtdObstaculos         = 0;
    qtdRemoverShapes      = 0;
    qtdBombasParaExplodir = 0;
}

static void agendarRemocaoObstaculo(cpShape *shape) {
    for (int i = 0; i < qtdRemoverShapes; i++)
        if (shapesParaRemover[i] == shape) return;
    shapesParaRemover[qtdRemoverShapes++] = shape;
}

static cpBool cbPimentaFruta(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    cpShape *shapePimenta, *shapeFruta;
    cpArbiterGetShapes(arb, &shapePimenta, &shapeFruta);
    if (cpShapeGetCollisionType(shapePimenta) != COLLISION_PIMENTA) {
        cpShape *temp = shapePimenta;
        shapePimenta = shapeFruta;
        shapeFruta = temp;
    }
    cpCollisionType tipo = cpShapeGetCollisionType(shapeFruta);
    if (tipo < NIVEIS_FRUTA) {
        Fruta *f = (Fruta*)cpShapeGetUserData(shapeFruta);
        if (f) {
            cpBody *bodyFruta = cpShapeGetBody(shapeFruta);
            cpVect  vel       = cpBodyGetVelocity(bodyFruta);
            vel = cpvmult(vel, PIMENTA_MULT_VELOCIDADE);
            vel = cpvadd(vel, cpv((rand() % (PIMENTA_IMPULSO_LATERAL * 2)) - PIMENTA_IMPULSO_LATERAL,
                                  -PIMENTA_IMPULSO_VERTICAL));
            cpBodySetVelocity(bodyFruta, vel);
            f->impulsionadaPorPimenta = 1;
            perderPontos(PONTOS_PERDA_PIMENTA);
        }
    }
    agendarRemocaoObstaculo(shapePimenta);
    return cpTrue;
}

static cpBool cbPodreFruta(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    cpShape *shapePodre, *shapeFruta;
    cpArbiterGetShapes(arb, &shapePodre, &shapeFruta);
    if (cpShapeGetCollisionType(shapePodre) != COLLISION_PODRE) {
        cpShape *temp = shapePodre;
        shapePodre = shapeFruta;
        shapeFruta = temp;
    }
    cpCollisionType tipo = cpShapeGetCollisionType(shapeFruta);
    if (tipo < NIVEIS_FRUTA) {
        Fruta *f = (Fruta*)cpShapeGetUserData(shapeFruta);
        if (f && !f->estaPodre) {
            f->estaPodre        = 1;
            f->cliquesRestantes = 10; 
            perderPontos(PONTOS_PERDA_PODRE);
        }
    }
    agendarRemocaoObstaculo(shapePodre);
    return cpTrue;
}

static cpBool cbBombaAtiva(cpArbiter *arb, cpSpace *espaco, cpDataPointer data) {
    cpShape *shapeBomba, *shapeOutro;
    cpArbiterGetShapes(arb, &shapeBomba, &shapeOutro);
    if (cpShapeGetCollisionType(shapeBomba) != COLLISION_BOMBA) {
        cpShape *temp = shapeBomba;
        shapeBomba = shapeOutro;
        shapeOutro = temp;
    }
    cpVect pos = cpBodyGetPosition(cpShapeGetBody(shapeBomba));
    fusoesBombaPos[qtdBombasParaExplodir++] = pos;
    agendarRemocaoObstaculo(shapeBomba);
    return cpTrue;
}

void configurarCallbacksObstaculos(cpSpace *espaco) {
    for (int i = 0; i < NIVEIS_FRUTA; i++) {
        cpSpaceAddCollisionHandler(espaco, COLLISION_PIMENTA, i)->beginFunc = cbPimentaFruta;
        cpSpaceAddCollisionHandler(espaco, COLLISION_PODRE,   i)->beginFunc = cbPodreFruta;
        cpSpaceAddCollisionHandler(espaco, COLLISION_BOMBA,   i)->beginFunc = cbBombaAtiva;
    }
    cpSpaceAddCollisionHandler(espaco, COLLISION_BOMBA, 100)->beginFunc = cbBombaAtiva;
}

void testarSpawnEspecial(cpSpace *espaco, NodeFruta **head) {
    int dado = GetRandomValue(1, 100);
    if (dado <= 70) return; /* 70% chance de não spawnar nada */
    if (qtdObstaculos >= MAX_OBSTACULOS_TELA) return;

    TipoObjetoEspecial tipo;
    int   colType;
    float raio = 18.0f;

    if      (dado <= 80) { tipo = OBJ_BOMBA;   colType = COLLISION_BOMBA;   }
    else if (dado <= 90) { tipo = OBJ_PIMENTA; colType = COLLISION_PIMENTA; }
    else                 { tipo = OBJ_PODRE;   colType = COLLISION_PODRE;   }

    float spawnX = (float)GetRandomValue(125, 675);
    ObjetoEspecial *obj = &listaObstaculos[qtdObstaculos++];
    obj->tipo  = tipo;
    obj->raio  = raio;
    obj->ativo = 1;
    obj->body = cpSpaceAddBody(espaco,
                    cpBodyNew(1.0f, cpMomentForCircle(1.0f, 0, raio, cpvzero)));
    cpBodySetPosition(obj->body, cpv(spawnX, 140.0f));

    obj->shape = cpSpaceAddShape(espaco, cpCircleShapeNew(obj->body, raio, cpvzero));
    cpShapeSetFriction(obj->shape, 0.3f);
    cpShapeSetElasticity(obj->shape, 0.4f);
    cpShapeSetCollisionType(obj->shape, colType);
    cpShapeSetUserData(obj->shape, obj);
}


void spawnBlocoFixo(cpSpace *espaco, NodeFruta **head) {
    if (qtdObstaculos >= MAX_OBSTACULOS_TELA) return;

    float larg = (float)GetRandomValue(60, 110);
    float alt  = (float)GetRandomValue(40, 70);
    float bx   = (float)GetRandomValue(200, 600);
    float by   = (float)GetRandomValue(350, 600);

    ObjetoEspecial *obj = &listaObstaculos[qtdObstaculos++];
    obj->tipo      = OBJ_BLOCO;
    obj->ativo     = 1;
    obj->areaBloco = (Rectangle){ bx - larg/2.0f, by - alt/2.0f, larg, alt };

    obj->body  = cpSpaceGetStaticBody(espaco);
    cpBB bb = cpBBNew(obj->areaBloco.x, obj->areaBloco.y,
                         obj->areaBloco.x + larg, obj->areaBloco.y + alt);
    obj->shape = cpSpaceAddShape(espaco, cpBoxShapeNew2(obj->body, bb, 0.0f));
    cpShapeSetFriction(obj->shape, 0.6f);
    cpShapeSetCollisionType(obj->shape, COLLISION_BLOCO);

    NodeFruta *atual = *head;
    while (atual != NULL) {
        cpVect fPos = cpBodyGetPosition(atual->fruta.body);
        if (CheckCollisionCircleRec(
                (Vector2){(float)fPos.x, (float)fPos.y},
                LISTA_FRUTAS[atual->fruta.nivel].raio,
                obj->areaBloco)) {
            cpVect dir    = cpvsub(fPos, cpv(bx, by));
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
            NodeFruta *prox = atual->next; 
            cpVect fPos = cpBodyGetPosition(atual->fruta.body);
            
            if ((float)cpvdist(bPos, fPos) <= raioExplosao) {
                removerFruta(espaco, atual->fruta.shape, head);
                perderPontos(PONTOS_PERDA_BOMBA_POR_FRUTA);
            }
            atual = prox;
        }
        for (int i = 0; i < qtdObstaculos; i++) {
            if (!listaObstaculos[i].ativo) continue;

            if (listaObstaculos[i].tipo == OBJ_BLOCO) {
                Vector2 centroBomba = { (float)bPos.x, (float)bPos.y };
                if (CheckCollisionCircleRec(centroBomba, raioExplosao, listaObstaculos[i].areaBloco)) {
                    agendarRemocaoObstaculo(listaObstaculos[i].shape);
                }
            } else {
                cpVect oPos = cpBodyGetPosition(listaObstaculos[i].body);
                if ((float)cpvdist(bPos, oPos) <= raioExplosao) {
                    agendarRemocaoObstaculo(listaObstaculos[i].shape);
                }
            }
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
                if (b != cpSpaceGetStaticBody(espaco)) {
                    cpSpaceRemoveBody(espaco, b);
                    cpBodyFree(b);
                }
                cpShapeFree(sh);
                break;
            }
        }
    }
    qtdRemoverShapes = 0;

    NodeFruta *atualPimenta = *head;
    while (atualPimenta != NULL) {
        if (atualPimenta->fruta.impulsionadaPorPimenta) {
            cpVect pos = cpBodyGetPosition(atualPimenta->fruta.body);
            if (pos.y < PIMENTA_LIMITE_Y) {
                cpVect vel = cpBodyGetVelocity(atualPimenta->fruta.body);
                if (vel.y < 0.0f) vel.y = 0.0f;
                cpBodySetVelocity(atualPimenta->fruta.body, vel);
                cpBodySetPosition(atualPimenta->fruta.body, cpv(pos.x, PIMENTA_LIMITE_Y));
            } else if (pos.y >= PIMENTA_LIMITE_Y + PIMENTA_MARGEM_LIBERACAO) {
                atualPimenta->fruta.impulsionadaPorPimenta = 0;
            }
        }
        atualPimenta = atualPimenta->next;
    }
}

void desenharObstaculos(Texture2D tex_bomba, Texture2D tex_podre, Texture2D tex_pimenta, Texture2D tex_bloco) {
    for (int i = 0; i < qtdObstaculos; i++) {
        if (!listaObstaculos[i].ativo) continue;

        if (listaObstaculos[i].tipo == OBJ_BLOCO) {
            Rectangle area = listaObstaculos[i].areaBloco;
            DrawTexturePro(tex_bloco,
                (Rectangle){0, 0, (float)tex_bloco.width, (float)tex_bloco.height},
                area,
                (Vector2){0, 0},
                0.0f,
                WHITE);

        } else {
            cpVect pos = cpBodyGetPosition(listaObstaculos[i].body);
            Color  cor;
            const char *txt;

            switch (listaObstaculos[i].tipo) {
                case OBJ_BOMBA:   cor = BLACK;     txt = "BOMB"; break;
                case OBJ_PIMENTA: cor = RED;       txt = "PIMT"; break;
                case OBJ_PODRE:   cor = DARKGREEN; txt = "PODR"; break;
                default:          cor = GRAY;      txt = "???";  break;
            }
            float raio = listaObstaculos[i].raio;
            float diametro = raio * 2;
            Texture2D tex;

            if (listaObstaculos[i].tipo == OBJ_BOMBA){
                tex = tex_bomba;
            } else if (listaObstaculos[i].tipo == OBJ_PODRE) {
                tex = tex_podre;
            } else {
                tex = tex_pimenta;
            }
            DrawTexturePro(tex,
                (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                (Rectangle){pos.x, pos.y, diametro, diametro},
                (Vector2){raio, raio},
                0.0f,
                WHITE
            );
        }
    }
}