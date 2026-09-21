#define CANT_REGISTROS 32

unsigned int registros[CANT_REGISTROS]; // 4 bytes cada registro

/*
    0 -> IP (Instruction Pointer)
    1 -> OPC (Operation Code)
    2 -> OP1 (Operando 1)
    3 -> OP2 (Operando 2)
    4 -> LAR (Logic Address Register)
    5 -> MAR (Memory Address Register)
    6 -> MBR (Memory Buffer Register)

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
*/