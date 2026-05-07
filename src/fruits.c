#include "fruits.h"
#include "chipmunk.h"
#include "physics.h"
#include <stdlib.h>

#define RGB(r, g, b) (Color){r, g, b, 255}

const Fruta_Def LISTA_FRUTAS[NIVEIS_FRUTA] = {
    {"Uva",  15.0f, RGB(170,  60,  200),   1 },  //tipo da uva = 0
    {"Cereja", 20.0f, RGB(255,  80, 80),   2 },  //tipo da cereja = 1
    {"Morango", 27.0f, RGB(192, 64, 66),   4 },  //tipo do morango = 2 ..
    {"Maçã",   35.0f, RGB(255, 30,   30),   8 },  
    {"Pera",    43.0f, RGB(180,  200,  50),  16 },  
    {"Laranja",   52.0f, RGB(255, 130, 0),  32 },  
    {"Abacaxi",  62.0f, RGB(230, 190,  30),  64 },  
    {"Melancia",73.0f, RGB(47, 173, 83), 128 },  
};

Fruta criarFruta(cpSpace *espaco, float x, float y, int tipo) {
    float raio = LISTA_FRUTAS[tipo].raio;
    cpShape* formatoFruta = criarCorpoFruta(espaco, 200, 300, raio, tipo); //criando o corpo da fruta de acrodo com o tipo dela
    cpBody* corpoFruta = cpShapeGetBody(formatoFruta);

    //criando a fruta
    Fruta fruta;
    fruta.shape = formatoFruta;
    fruta.body = corpoFruta;
    fruta.fundindo = 0; //nao esta fundindo
    fruta.nivel = tipo; //tipo da fruta
    return fruta;
}
//funcoes da lista encadeada da fruta:

NodeFruta* criarNodeFruta(Fruta fruta) {
    NodeFruta* novoNode = (NodeFruta*)malloc(sizeof(NodeFruta));
    novoNode->fruta = fruta;
    novoNode->next = NULL;
    return novoNode;
}

void inserirFruta(cpSpace* espaco, float x, float y, int tipo, NodeFruta** head) {
    Fruta novaFruta = criarFruta(espaco, x, y, tipo);
    NodeFruta* fruta = criarNodeFruta(novaFruta);
    cpShapeSetUserData(fruta->fruta.shape, &fruta->fruta); //linka o "objeto" fruta com o shape/formato dela

    //inserir a fruta no começo da lista
    fruta->next = *head; //novo node aponta para o head q foi recebido
    *head = fruta; //o novo node agora é o head
}

void removerFruta(cpSpace* espaco, cpShape *frutaRemover, NodeFruta** head) {
    if(*head == NULL) {
        return;
    }else {
        NodeFruta* aux = *head;
        NodeFruta* anterior = NULL;
        while(aux != NULL && aux->fruta.shape != frutaRemover) { //enquanto nao achar a fruta para remover
            anterior = aux;
            aux = aux->next;
        }
        if (aux==NULL) return; //se nao achar

        if (aux == *head)*head = aux->next; //dança de ponteiros para ajustar os ponteiros
        else anterior->next = aux->next;

        //remover do espaço da fisica
        cpSpaceRemoveShape(espaco, aux->fruta.shape);
        cpShapeFree(aux->fruta.shape);
        cpSpaceRemoveBody(espaco, aux->fruta.body);
        cpBodyFree(aux->fruta.body);

        free(aux);

    }
}