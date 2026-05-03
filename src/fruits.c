#include "fruits.h"
#include <stdlib.h>

#define RGB(r, g, b) (Color){r, g, b, 255}

const FrutaDef DEFS_FRUTA[NIVEIS_FRUTA] = {
    { 15.0f, RGB(255,  80,  80),   1 },  
    { 20.0f, RGB(220,  80, 120),   2 },  
    { 27.0f, RGB(170,  60, 200),   4 },  
    { 35.0f, RGB(255, 130,   0),   8 },  
    { 43.0f, RGB(255,  30,  30),  16 },  
    { 52.0f, RGB(180, 200,  50),  32 },  
    { 62.0f, RGB(230, 190,  30),  64 },  
    { 73.0f, RGB( 60, 200, 100), 128 },  
}
/*Lista de frutas na ordem:
Cereja, Morango, Uva, Laranja, Maçã, Pera, Abacaxi, Melancia
*/
