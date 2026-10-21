#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "funcionesAuxiliares.h"
#include "estructuras.h"

char *archivoImagen; 

void imprimirHexa(unsigned int dato, int cantBytes){
    printf("0x");
    unsigned int aux;
    for (int i = (cantBytes-1); i >= 0; i--){
        aux = dato >> (i*8);
        aux &= 0xFF;
        printf("%02X ",aux & 0xFF);
    }
}

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

void mostrarMemoria(unsigned int inicio, unsigned int cantidad){
    printf("\nMemoria de celda [%d] a celda [%d](inclusive)\n",inicio,inicio+cantidad-1);
    for (int i=0; i<cantidad;i++){
        printf("Memoria[%d]: %02X\n",i+inicio,memoria[i+inicio]);
    }
    printf("\n");
}

void mostrarRegistros(){
    printf("Estado actual de los registros \n");
    for (int i = 0; i < CANT_REGISTROS; i++){
        printf("Registro[%d]: ",i);
        imprimirHexa(registros[i],4);
        printf("\n");
    }
    printf("\n");
}

void mostrarTablaSegm(){
    printf("Tabla de Segmentos\n");
    for (int i = 0; i < CANT_SEGM; i++){
        printf("Segmento[%d]: ",i);
        printf("Base: ");
        imprimirHexa(tablaSegm[i].base,2);
        printf("Tamanio: "); // windows no detecta la ñ ):
        imprimirHexa(tablaSegm[i].tamaño,2);
        printf("\n");
    }
    printf("\n");
}


//  FUNCIONES AUXILIARES

unsigned char devolverByte(unsigned int dato, int nroByte){ // el nro byte es el byte a devolver, siendo 0 el byte de menor significado, 1: el segundo de menor significado, ....
    dato = dato >> (8 * nroByte);
    dato &= 0xFF;
    unsigned char aux = dato;
    return aux;
}

void comprobarFalloSegmento(){
    int lar = registros[4];
    int posSegm = devolverByte(lar,2);
    int offset = lar & 0xFFFF;

    if (posSegm < 0 || posSegm >= CANT_SEGM){
        printf("\nError: Fallo de Segmento \n");
        exit(1);
    }
    if (offset >= tablaSegm[posSegm].tamaño){
        printf("\nError: Fallo de Segmento \n");
        exit(1);
    }
}

void comprobarMemoriaInsuficiente(){
    // originalmente iba a sumar los tamaños de todos los segmentos, pero dado que el stack segment es el último
    // y siempre se encuentra presente, puedo usar su base + su tamaño como reemplazo a la suma de tamaños de segmentos
    int posStack = (registros[29] >> 16) & 0xFFFF; 
    int sumaSegm = tablaSegm[posStack].base;
    sumaSegm += tablaSegm[posStack].tamaño;

    if (sumaSegm > tamMemoria){
        printf("Error: Memoria Insuficiente (%d > %d)\n",sumaSegm,tamMemoria);
        exit(1);
    }
}

void configurarStringBigEndian(char *cadena, unsigned int valor,int n){
    for (int i=0; i<n; i++)
        cadena[i] = (valor >> 8 * (n-1 -i)) & 0xFF;
}

int leerNumBigEndian(char *cadena, int n){
    unsigned int num = 0;
    for (int i=0; i<n; i++)
        num |= cadena[i] << 8 * (n-1 -i);
    return num;
}

void restaurarVmx(){
    FILE *f = fopen(archivoImagen,"wb");
    if (f == NULL){
        printf("Error al abrir archivo imagen\n");
        exit(1);
    }
    else{
        char id[6], version, stringTamMemoria[2];
        fread(id,6,1,f);
        fread(&version,1,1,f);
        fread(&stringTamMemoria,2,1,f);

        if (strcmp(id,"VMX26") == 0 && version == 1){
            tamMemoria = leerNumBigEndian(stringTamMemoria,2);

            // restauro registros
            char strReg[4];
            for (int i=0; i<CANT_REGISTROS;i++){
                fread(strReg,4,1,f);
                registros[i] = leerNumBigEndian(strReg,4);
            }

            // restauro tablaSegm
            char strSegm[4];
            for (int i=0; i<CANT_SEGM;i++){
                fread(strSegm,4,1,f);
                tablaSegm[i].base = leerNumBigEndian(strSegm,2);
                tablaSegm[i].tamaño = leerNumBigEndian(strSegm+2,2);
            }

            // restauro memoria
            char *charByte;
            for (int i=0; i<tamMemoria;i++){
                fread(charByte,1,1,f);
                memoria[i] = *charByte;
            }       

        }
        else{
            printf("Error, formato de archivo imagen válido\n");
            exit(1);
        }
        fclose(f);
    }
}

void setearVmi(){
    FILE *f = fopen(archivoImagen,"wb");
    if (f == NULL){
        printf("Error al crear archivo imagen\n");
        exit(1);
    }
    else{
        char id[6] = "VMX26", version = 1, stringTamMemoria[2];
        fwrite(id,strlen(id),1,f);
        fwrite(&version,1,1,f);

        configurarStringBigEndian(stringTamMemoria, tamMemoria,2);
        fwrite(stringTamMemoria,2,1,f);


        // copio registros
        char strReg[4];
        for (int i=0; i<CANT_REGISTROS;i++){
            configurarStringBigEndian(strReg,registros[i],4);
            fwrite(strReg,4,1,f);
        }

        // copio segmentos
        char strSegm[4];
        for (int i=0; i<CANT_SEGM;i++){
            configurarStringBigEndian(strSegm,tablaSegm[i].base,2);
            configurarStringBigEndian(strSegm+2,tablaSegm[i].tamaño,2);
            fwrite(strSegm,4,1,f);
        }

        // copio memoria
        char *charByte;
        for (int i=0; i<tamMemoria;i++){
            charByte = memoria + i;
            fwrite(charByte,1,1,f);
        }
        fclose(f);
    }
}

void escribirMemoriaArranque(int dirFisica,int valor){ // función a usar en el módulo de arranque
    for (int i=0;i<4;i++){
        memoria[dirFisica+i] = devolverByte(valor, 3-i);
    }
}

int leerOperando(int operando){
    int tipo = (devolverByte(operando,3));
    int datosOperando = operando & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            return registros[datosOperando & 0x1F];

        case 2: // inmediato
            int16_t aux16 = datosOperando & 0xFFFF;
            int32_t aux = aux16; // esto para convertir correctamente inmediatos negativos
            return (aux);

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(4);
            leerDeMemoria();
            return registros[6];
        default: // ninguno
            return 0;
    }
}

void escribirOperando(int operando, int valor){
    int tipo = devolverByte(operando,3);
    int datosOperando = operando & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            registros[datosOperando & 0x1F] = valor;
            break;

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(4);
            escribirMBR(valor);
            escribirEnMemoria();
            break;

        default: // ninguno o inmediato (no son destino válido)
            break;
    }
}

void leerDeMemoria(){
    int n; // cant bytes a leer
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;
    unsigned int aux = 0;

    for(int i=0; i < n; i++){
        aux = aux << 8;
        aux = aux | memoria[posFisica + i];
    }
    escribirMBR(aux);
}

void escribirEnMemoria(){
    int mbr;
    int n; // cant bytes a escribir
    mbr = registros[6];
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;

    for(int i=0; i < n; i++){
        memoria[posFisica+i] = devolverByte(mbr,n-1 -i);
    }
}

void escribirMBR(int valor){ // configura el MBR
    registros[6] = valor;
}

void calcularPunteroLogico(int operandoMemoria){ // configura el LAR
    int nroReg = (devolverByte(operandoMemoria,0) & 0x1F );
    int puntLog = registros[nroReg];
    int aux = devolverByte(operandoMemoria,2);
    aux = aux << 8;
    aux = aux | devolverByte(operandoMemoria,1);
    puntLog += aux;
    registros[4] = puntLog;
    comprobarFalloSegmento();
}

void calcularPunteroFisico(int cantidadDeBytes){ // configura el MAR
    int puntLogico = registros[4];
    int nroSegm = devolverByte(puntLogico,2);
    int puntFisico = tablaSegm[nroSegm].base; // dirección base

    int aux = devolverByte(puntLogico,1);
    aux = aux << 8;
    aux = aux | devolverByte(puntLogico,0);

    puntFisico += aux; // + offset

    cantidadDeBytes = cantidadDeBytes << 16;
    puntFisico |= cantidadDeBytes; // cargamos en la parte alta del MAR la cant de bytes a leer/escribir

    registros[5] = puntFisico; // reg[5] -> MAR
}