// quitar las redundantes o innecesarias
#include "puntFunc.c"
int disassembler;

void paso(){
//    printf("\n\nusted ha llegado a 'void paso()'\n\n\n");
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
    if (n==1 || n==3)
        leerDeMemoriaReg();
    else
        leerDeMemoria();
    op2 = op2 << 24;
    op2 |= registros[6];
    if (n==1 || n==3)
        op2 &= 0xFFFFFF1F;

    registros[0] += n;
    registros[4] = registros[0];
    
    n = op1;
//    printf("\nop1: %08X\n",n);
    calcularPunteroFisico(n);
//    printf("\nque hay en mar?: %X\n",registros[5]);
    if (n==1 || n==3)
        leerDeMemoriaReg();
    else
        leerDeMemoria();
//    printf("\nop1: %8X\n",op1);
    op1 = op1 << 24;
//    op1 = op1 & 0xFF000000;
//    printf("\nque hay en mbr?: %08X\n",registros[6]);
    op1 |= registros[6];
//    if (n==1 || n==3)
//        op1 &= 0xFFFFFF1F;
//    printf("\nop1: %08X\n",op1);
    registros[0] += n;


    registros[1] = opc;
    registros[2] = op1;
    registros[3] = op2;

    printf("\nOperación: ");
    imprimirHexa(opc,1);

    printf("\nOperando 1: ");
    imprimirHexa(op1,4);
    printf(" | %d | %c",op1 & 0xFFFFFF, op1 & 0xFF);

    printf("\nOperando 2: ");
    imprimirHexa(op2,4);
    printf(" | %d | %c",op2 & 0xFFFFFF, op2 & 0xFF);
    printf("\n");

//    printf("\n%d\n",disassembler);
    if (disassembler)
        mostrarAssembler(direccionInstruccion);

    funciones[opc]();
}

void procesa(){
//    printf("usted ha llegado a 'void procesa()'\n\n\n");
//    printf("tamaño del código según la tabla de segmentos: %d \n",tablaSegm[0].tamaño);
    /*
    for (int i=0;i<32;i++){
    printf("Reg[%d]: ",i);
    imprimirHexa(registros[i],4);
    printf("\n");
    }
    */

    /*
    printf("Segm1 -> Base: %d, Tamaño: %d \n",tablaSegm[0].base,tablaSegm[0].tamaño);
    printf("Segm2 -> Base: %d, Tamaño: %d \n",tablaSegm[1].base,tablaSegm[1].tamaño);
    */

    // sigue mientras el IP apunte dentro del segmento de código (el 0 de la tabla)
    while ( ( (registros[0] >> 16) == 0) && ( (registros[0] & 0xFFFF) < tablaSegm[0].tamaño)){
    //    printf("valor actual de ip:%4X",registros[0]);
        paso();
//        printf("apsdniansdina");
    }
//    printf("%c%c%c%c%c%c%c%c%c%cxxx \n",memoria[0],memoria[1],memoria[2],memoria[3],memoria[4],memoria[5],memoria[6],memoria[7],memoria[8],memoria[9]);
//    printf("valor actual de ip:%04X",registros[0]);
/*    for (int i=0;i<32;i++){
        printf("Reg[%d]: ",i);
        imprimirHexa(registros[i],4);
        printf("\n");
    }*/
    printf("\n");
    mostrarMemoria(tablaSegm[1].base,40);
    printf("\nGracias por compilar (: \nGlass Group SA -> subdivisión Grupo 0x00 -> departamento oficial encargado de la interpretación y ejecución Assembly\n");
}