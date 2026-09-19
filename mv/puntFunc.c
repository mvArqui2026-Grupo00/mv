#include "funciones.c"
#include "disassembler.c"
#define CANT_FUNC 32

typedef void (*PuntFunc)(); // plantilla de la cabecera de las funciones

PuntFunc funciones[CANT_FUNC];

void inicializarPunteroFunciones(){
    // en esta primera parte implementamos 28 funciones
    funciones[0] = sys;
    funciones[1] = jmp;
    funciones[2] = jp;
    funciones[3] = jn;
    funciones[4] = jz;
    funciones[5] = jc;
    funciones[6] = jv;
    funciones[7] = jnp;
    funciones[8] = jnn;
    funciones[9] = jnz;
    funciones[10] = not; 
    // funciones que no se desarrollan en esta parte
    funciones[11] = notDefined;
    funciones[12] = notDefined;
    funciones[13] = notDefined;
    funciones[14] = notDefined;
    //
    funciones[15] = stop;    
    funciones[16] = mov;
    funciones[17] = add;
    funciones[18] = sub;
    funciones[19] = mul;
    funciones[20] = div;
    funciones[21] = cmp;
    funciones[22] = and;
    funciones[23] = or;
    funciones[24] = xor;
    funciones[25] = swap;
    funciones[26] = shl;
    funciones[27] = shr;
    funciones[28] = sar;
    funciones[29] = ldl;
    funciones[30] = ldh;
    funciones[31] = rnd;
}