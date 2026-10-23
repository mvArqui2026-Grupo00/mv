#include "puntFunc.h"
#include "mnemonicos.h"

PuntFunc funciones[CANT_FUNC];

void inicializarPunteroFunciones(int version){
    // en esta primera parte implementamos 28 funciones
    funciones[0] = sys;
    funciones[1] = jmp;
    funciones[2] = jp;
    funciones[3] = jneg; // jn es una función built-in (del lenguaje)
    funciones[4] = jz;
    funciones[5] = jc;
    funciones[6] = jv;
    funciones[7] = jnp;
    funciones[8] = jnn;
    funciones[9] = jnz;
    funciones[10] = not; 

    // funciones agregadas en la 2da parte
    if (version == 1){
        funciones[11] = notDefined;
        funciones[12] = notDefined;
        funciones[13] = notDefined;
        funciones[14] = notDefined;
    }
    else{
        funciones[11] = push;
        funciones[12] = pop;
        funciones[13] = call;
        funciones[14] = ret;
    }

    //

    funciones[15] = stop;    
    funciones[16] = mov;
    funciones[17] = add;
    funciones[18] = sub;
    funciones[19] = mul;
    funciones[20] = divis; // div es una función de stdlib
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