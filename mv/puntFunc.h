#ifndef PUNT_FUNC_H
#define PUNT_FUNC_H

#define CANT_FUNC 32
void inicializarPunteroFunciones();
typedef void (*PuntFunc)(); // plantilla de la cabecera de las funciones
extern PuntFunc funciones[CANT_FUNC];

#endif