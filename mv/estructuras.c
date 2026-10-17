#include "estructuras.h"

/*if (tamMemoria > 0) // esto no funciona
    unsigned char memoria[tamMemoria];;
else
    unsigned char memoria[CANT_CELDAS];*/
Segmento tablaSegm[CANT_SEGM];
unsigned char memoria[CANT_CELDAS]; // CAMBIAR ??
int registros[CANT_REGISTROS]; // 4 bytes cada registro

/*  Tabla de Segmentos
    0 - 5 -> variables
    6 -7 -> -1 (no se usan nunca)

    CS y SS si o si están
    el resto pueden no estar

    el orden es
    Param
    Constant
    Code

    Data
    Extra
    Stack

*/


/*  REGISTROS
    0 -> IP (Instruction Pointer)
    1 -> OPC (Operation Code)
    2 -> OP1 (Operando 1)
    3 -> OP2 (Operando 2)
    4 -> LAR (Logic Address Register)
    5 -> MAR (Memory Address Register)
    6 -> MBR (Memory Buffer Register)

    7 -> SP (Stack Pointer)
    8 -> BP (Base Pointer)

    10 -> EAX (General Purpose Registers)
    11 -> EBX (General Purpose Registers)
    12 -> ECX (General Purpose Registers)
    13 -> EDX (General Purpose Registers)
    14 -> EEX (General Purpose Registers)
    15 -> EFX (General Purpose Registers)
    16 -> AC (Accumulator)
    17 -> CC (Condition Code)
    
    26 -> CS (Puntero al inicio del Code Segment)
    27 -> DS (Puntero al inicio del Data Segment)
    28 -> ES (Puntero al inicio del Extra Segment)
    29 -> SS (Puntero al inicio del Stack Segment)
    30 -> KS (Puntero al inicio del Const Segment)
    31 -> PS (Puntero al inicio del Param Segment)
*/