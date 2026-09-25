#include "stdio.h"
#include "disassembler.h"
#include "funcionesAuxiliares.h"


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

void pasoDisassembler(unsigned int * dirInstr){
    unsigned int direccionInstruccion = * dirInstr;
    int n1, n2, instr, opc, op1, op2;
    
    instr = memoria[direccionInstruccion];
    printf("[%04X] ",direccionInstruccion);
    printf("%02X ",instr);
    direccionInstruccion++;
    
    opc = instr & 0X1F; // opc
    if (opc == 0x0F) // operación de 0 operandos
        n1 = n2 = 0;
    else
        if (instr & 0x10){ // operación de 2 operandos
            n1 = (instr >> 4) & 0x03;
            n2 = (instr >> 6) & 0x03;
        }
        else{
            n1 = (instr >> 6) & 0x03;
            n2 = 0;
        }

    op2 = 0;
    for (int i=0;i<n2;i++){
        op2 = op2 << 8;
        op2 |= memoria[direccionInstruccion];
        direccionInstruccion++;
    }

    op1 = 0;
    for (int i=0;i<n1;i++){
        op1 = op1 << 8;
        op1 |= memoria[direccionInstruccion];
        direccionInstruccion++;
    }

    for(int i=0; i < n2; i++){
        printf("%02X ",devolverByte(op2,n2-1 -i));
    }
    for(int i=0; i < n1; i++){
        printf("%02X ",devolverByte(op1,n1-1 -i));
    }
    printf("\t\t| ");
    printf("%s \t",mnemonicos[opc & 0x1F]);

    mostrarOperando(n1,op1);
    if (n2){
        printf(",\t");
        mostrarOperando(n2,op2);
    }

    printf("\n");
    *dirInstr = direccionInstruccion;
}

void mostrarAssembler(unsigned int tamCod){
    unsigned int direccionInstruccion = 0; // actualmente el comienzo del cs está en 0, esto se podría cambiar de ser necesario
    
    printf("\n \t\t Disassembler \n");
    printf(" -------- -------- -------- --------\n\n");
    while(direccionInstruccion < tamCod){
        pasoDisassembler(&direccionInstruccion);
    }
    printf("\n -------- -------- -------- --------\n\n");
}