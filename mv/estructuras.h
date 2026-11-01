#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdint.h>
#define CANT_REGISTROS 32
#define CANT_CELDAS 16384 // (Cantidad predeterminada de bytes en mem ppal) 16 KiB -> 2^(10+4) -> 16384
#define CANT_SEGM 8

typedef struct {
    uint16_t base, tamaño; // 2 bytes cada elemento // ACTUALIZACIÓN 31/10 -> LOS HICE SIN SIGNO PARA ANALIZAR UN CASO DE FALLO DE TAMAÑOS DE SEGMENTO (SI EL TAMAÑO ES MAYOR O IGUAL A 0x8000 se vuelve negativo))
} Segmento;

void inicializarMemoria();

extern short int tamMemoria;
extern Segmento tablaSegm[CANT_SEGM];
extern unsigned char *memoria;
extern int registros[CANT_REGISTROS]; // 4 bytes cada registro

extern int hayImagen;
extern int pasoAPaso;

#endif
