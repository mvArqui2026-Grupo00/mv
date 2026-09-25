#include <stdio.h>
#include <stdint.h>
#include "procesador.h"
#include "puntFunc.c"
#include "mnemonicos.h"
#include "funcionesAuxiliares.h"
#include "estructuras.h"
#include "disassembler.h"

void paso(){
    unsigned char instr, opc;
    int16_t direccionInstruccion;
    int op1, op2, n;

    registros[4] = registros[0];
    calcularPunteroFisico(1); // leemos de a byte
    leerDeMemoria();
    instr = registros[6]; // lo que está en mbr
    direccionInstruccion = registros[5] & 0xFFFF; // obtengo el offset desde el cs para pasarselo al disassembler
    opc = instr & 0X1F;

    if (opc == 0x0F) // operación de 0 operandos
        op1 = op2 = 0;
    else
        if (instr & 0x10){ // operación de 2 operandos
            op1 = (instr >> 4) & 0x03;
            op2 = (instr >> 6) & 0x03;
        }
        else{
            op1 = (instr >> 6) & 0x03;
            op2 = 0;
        }
    
    registros[0]++;
    registros[4] = registros[0];


    n = op2;
    calcularPunteroFisico(n);
    leerDeMemoria();
    op2 = op2 << 24;
    op2 |= registros[6];
    registros[0] += n;
    registros[4] = registros[0];
    
    
    n = op1;
    calcularPunteroFisico(n);
    leerDeMemoria();
    op1 = op1 << 24;
    op1 |= registros[6];
    registros[0] += n;


    registros[1] = opc;
    registros[2] = op1;
    registros[3] = op2;

    funciones[opc]();
}

void procesa(){
    // sigue mientras el IP apunte dentro del segmento de código (el 0 de la tabla) y no haya ocurrido un error
    while ( ( (registros[0] >> 16) == 0) && ( (registros[0] & 0xFFFF) < tablaSegm[0].tamaño)){
        paso();
    }
    
    printf("\n -------------------------------------------\n");
    printf("\nദി(˵ •̀ ᴗ - ˵ ) ✧ \n\n");
    printf(":: Gracias por compilar (: \n:: GLASS Group S.L.R - Departamento de Codificacion Assembler\n\n");
    //subdivisión Grupo 0x00 -> departamento oficial encargado de la interpretación y ejecución Assembly
}