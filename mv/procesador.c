// quitar las redundantes o innecesarias
#include "puntFunc.c"
int disassembler;

void testOperandos(int opc, int op1, int op2){
    printf("\nOperación: ");
    imprimirHexa(opc,1);

    printf("\nOperando 1: ");
    imprimirHexa(op1,4);
    printf(" | %d | %c",op1 & 0xFFFFFF, op1 & 0xFF);

    printf("\nOperando 2: ");
    imprimirHexa(op2,4);
    printf(" | %d | %c",op2 & 0xFFFFFF, op2 & 0xFF);
    printf("\n");
}

void paso(){
    registros[4] = registros[0];
    calcularPunteroFisico(1); // leemos de a byte
    leerDeMemoria();
    unsigned char instr = registros[6]; // lo que está en mbr
    short int direccionInstruccion = registros[5] & 0xFFFF; // obtengo el offset desde el cs para pasarselo al disassembler
    unsigned char opc;
    int op1;
    int op2;
    opc = instr & 0X1F; // opc
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


    int n = op2;
    calcularPunteroFisico(n);
    leerDeMemoria();
    op2 = op2 << 24;
    op2 |= registros[6];
    if (n==1 || n==3)
        op2 &= 0xFFFFFF1F;        
    registros[0] += n;
    registros[4] = registros[0];
    
    
    n = op1;
    calcularPunteroFisico(n);
    leerDeMemoria();
    op1 = op1 << 24;
    op1 |= registros[6];
    if (n==1 || n==3)
        op1 &= 0xFFFFFF1F;
    registros[0] += n;


    registros[1] = opc;
    registros[2] = op1;
    registros[3] = op2;

//    testOperandos(opc,op1,op2);

    if (disassembler)
        mostrarAssembler(direccionInstruccion);

    funciones[opc]();
}

void procesa(){
    // sigue mientras el IP apunte dentro del segmento de código (el 0 de la tabla) y no haya ocurrido un error
    while ( ( (registros[0] >> 16) == 0) && ( (registros[0] & 0xFFFF) < tablaSegm[0].tamaño)){
        paso();
    }
    
    printf("\n -------------------------------------------\n");
    printf("Gracias por compilar (: \nGlass Group SA -> subdivisión Grupo 0x00 -> departamento oficial encargado de la interpretación y ejecución Assembly\n\n");
}