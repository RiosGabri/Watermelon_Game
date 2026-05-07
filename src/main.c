#include <stdio.h>
#include <chipmunk/chipmunk.h>
#include "fruits.h"
#include "physics.h"

int main(void){
  cpSpace *espaco = initEspaco();
  criarArea(espaco);
  registrarFusoes(espaco);

  NodeFruta *headFrutas = NULL;

  // teste: criar duas uvas na mesma posição
  inserirFruta(espaco, 200, 100, 0, &headFrutas);
  inserirFruta(espaco, 200, 200, 0, &headFrutas);

  // game loop básico (sem Raylib por enquanto)
  for (int frame = 0; frame < 300; frame++) {
    cpSpaceStep(espaco, 1.0f / 60.0f);
    processarFusoes(espaco, &headFrutas);

    // debug: imprimir posição de cada fruta
    NodeFruta *atual = headFrutas;
    while (atual != NULL) {
      cpVect pos = cpBodyGetPosition(atual->fruta.body);
      printf("nivel %d | x=%.1f y=%.1f\n",
             atual->fruta.nivel, pos.x, pos.y);
      atual = atual->next;
    }
  }

  cpSpaceFree(espaco);

  return 0;
}

//comando para compilar: gcc main.c (Get-ChildItem .\src\chipmunk\*.c -Recurse | % FullName) -Iinclude -o game.exe -lm
//comando para rodar: .\game.exe
//como não tem interface gráfica, o programa vai imprimir valores no terminal ;)