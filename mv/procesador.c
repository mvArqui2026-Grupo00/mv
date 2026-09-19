// quitar las redundantes o innecesarias
#include "puntFunc.c"
int disassembler;

void paso(){
    registros[4] = registros[0];
    calcularPunteroFisico(1); // leemos de a byte
    leerDeMemoria();
    char instr = registros[6]; // lo que está en mbr
    short int direccionInstruccion = registros[5] & 0xFFFF; // obtengo el offset desde el cs para pasarselo al disassembler

    char opc;
    int op1;
    int op2;
    opc = instr & 0X1F; // opc
    if (instr == 0x0F) // operación de 0 operandos
        op1 = op2 = 0;
    else
        if (instr & 0x10){ // operación de 2 operandos
            op1 = instr & 0x30;
            op2 = instr & 0xC0;
        }
        else{
            op1 = instr & 0xC0;
            op2 = 0;
        }
    
    registros[0]++;
    registros[4] = registros[0];
    int n = op2;
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

    if (disassembler)
        mostrarAssembler(direccionInstruccion);
    funciones[opc];
}

void procesa(){
    // sigue mientras el IP apunte dentro del segmento de código (el 0 de la tabla)
    while ((registros[0] >> 16) == 0 && (registros[0] & 0xFFFF) < tablaSegm[0].tamaño)
        paso();
    printf("Gracias por compilar (: \nGlass Group SA -> subdivisión Grupo 0x00 -> departamento oficial encargado de la interpretación y ejecución Assembly\n");
}