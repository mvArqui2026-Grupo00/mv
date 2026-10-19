#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdint.h>
#define CANT_REGISTROS 32
#define CANT_CELDAS 16384 // (Cantidad predeterminada de bytes en mem ppal) 16 KiB -> 2^(10+4) -> 16384
#define CANT_SEGM 8

typedef struct {
    int16_t base, tamaño; // 2 bytes cada elemento
} Segmento;

void inicializarMemoria();

extern int tamMemoria;
extern Segmento tablaSegm[CANT_SEGM];
extern unsigned char *memoria;
extern int registros[CANT_REGISTROS]; // 4 bytes cada registro

#endif