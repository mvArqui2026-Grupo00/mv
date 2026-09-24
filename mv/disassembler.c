char mnemonicos[32][5] = {
    "SYS",
    "JMP",
    "JP",
    "JN",
    "JZ",
    "JC",
    "JV",
    "JNP",
    "JNN",
    "JNZ",
    "NOT",

    "NDef",
    "NDef",
    "NDef",
    "NDef",

    "STOP",
    "MOV",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "CMP",
    "AND",
    "OR",
    "XOR",
    "SWAP",
    "SHL",
    "SHR",
    "SAR",
    "LDL",
    "LDH",
    "RND"
};

char reg[32][5] = {
    "IP",
    "OPC",
    "OP1",
    "OP2",
    "LAR",
    "MAR",
    "MBR",

    "RgND",
    "RgND",
    "RgND",

    "EAX",
    "EBX",
    "ECX",
    "EDX",
    "EEX",
    "EFX",
    "AC",
    "CC",
    "RgND",
    "RgND",
    "RgND",
    "RgND",
    "RgND",
    "RgND",
    "RgND",
    "RgND",

    "CS",
    "DS",
    
    "RgND",
    "RgND",
    "RgND",
    "RgND",
};


void mostrarOperando(unsigned int tipo, int operando){
    switch (tipo){
        case 0:
            break;
        case 1:
            printf("%s",reg[operando]);
            break;
        case 2:
            printf("%d",operando);
            break;
        case 3:
            printf("[%s+%d]", reg[operando & 0x1F], (operando & 0xFFFF00) >> 8);
            break;  
    }
}


void mostrarAssembler(short int direccionInstruccion){
    int dirFisica = tablaSegm[registros[26] & 0x00FF0000].base + direccionInstruccion;
    printf("[%04X] ",direccionInstruccion);
    printf("%02X ",registros[1]);
    int op1 = registros[2] & 0x00FFFFFF;
    int op2 = registros[3] & 0x00FFFFFF;
    unsigned int tipo1 = registros[2] & 0xFF000000;
    tipo1 = tipo1 >> 24;
    unsigned int tipo2 = registros[3] & 0xFF000000;
    tipo2 = tipo2 >> 24;

    dirFisica++;
    for(int i=0; i < tipo1; i++){
        printf("%02X ",memoria[dirFisica]);
        dirFisica++;
    }
    for(int i=0; i < tipo2; i++){
        printf("%02X ",memoria[dirFisica]);
        dirFisica++;
    }
    printf("\t\t| ");
    printf("%s \t",mnemonicos[registros[1] & 0x1F] );

    mostrarOperando(tipo1,op1);
    if (tipo2){
        printf(",\t");
        mostrarOperando(tipo2,op2);
    }

    printf("\n");
}