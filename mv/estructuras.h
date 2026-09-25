#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdint.h>
#define CANT_REGISTROS 32
#define CANT_CELDAS 16384 // 16 KiB -> 2^(10+4) -> 16384
#define CANT_SEGM 8

typedef struct {
    int16_t base, tamaño; // 2 bytes cada elemento
} Segmento;

extern Segmento tablaSegm[CANT_SEGM];
extern unsigned char memoria[CANT_CELDAS];
extern int registros[CANT_REGISTROS]; // 4 bytes cada registro

#endif