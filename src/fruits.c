#include "fruits.h"
#include <stdlib.h>

#define RGB(r, g, b) (Color){r, g, b, 255}

const Fruta_Def LISTA_FRUTAS[NIVEIS_FRUTA] = {
    {"Uva",      15.0f, RGB(170,  60, 200),   1 },
    {"Cereja",   20.0f, RGB(255,  80,  80),   2 },
    {"Morango",  27.0f, RGB(192,  64,  66),   4 },
    {"Maca",     35.0f, RGB(255,  30,  30),   8 },
    {"Pera",     43.0f, RGB(180, 200,  50),  16 },
    {"Laranja",  52.0f, RGB(255, 130,   0),  32 },
    {"Abacaxi",  62.0f, RGB(230, 190,  30),  64 },
    {"Melancia", 73.0f, RGB( 47, 173,  83), 128 },
};