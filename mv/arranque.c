#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procesador.h"
#include "puntFunc.h"
#include "funcionesAuxiliares.h"
#include "estructuras.h"
#include "disassembler.h"



unsigned short int leerTam(FILE *f){
    unsigned char byte1,byte2;
    unsigned short int aux;
    fread(&byte1,1,1,f);
    fread(&byte2,1,1,f);
    aux = byte1;
    aux = aux << 8;
    aux |= byte2;
    return aux;
}

void inicializarVersion1(FILE *f,int tamCod){
    inicializarMemoria(CANT_CELDAS);
    inicializarSegm(tamCod,CANT_CELDAS-tamCod,0,0,0,0);
    inicializarCodeSegment(tamCod,f); // pasamos el puntero a archivo apuntando al inicio del "code segment"
    fclose(f);

    registros[0] = 0x00000000;
    inicializarPunteroFunciones(1);
}

void inicializarVersion2(FILE *f,unsigned int tamCod, unsigned int tamData, unsigned int tamExtra, unsigned int tamStack, unsigned int tamConst, unsigned int tamParam, unsigned int tamMemoria, unsigned int offsetEntryPoint){
    inicializarMemoria(tamMemoria);
    inicializarSegm(tamCod,tamData,tamExtra,tamStack,tamConst,tamParam);
    inicializarCodeSegment(tamCod,f);
    fclose(f);

    registros[0] += offsetEntryPoint;
    registros[7] = registros[29] + tablaSegm[devolverByte(registros[29],2)].tamaño; // sp inicializado en ss + SS.Tamaño (posición fuera del Stack Segment)
    inicializarPunteroFunciones(2);
}

void inicializarCodeSegment(short int tamCod, FILE *f) { // se debe pasar el puntero f referenciando al comienzo del código
    unsigned char byte;
    registros[4] = registros[26];
    calcularPunteroFisico(1);
    int base = registros[5] & 0xFFFF;
    for (int i = 0; i < tamCod; i++){
        fread(&byte,1,1,f);
        memoria[base+i] = byte;
    }
}

void inicializarConstSegment(short int tamConst,FILE *f) { // se debe pasar el puntero f referenciando al comienzo de las constantes
    unsigned char byte;
    registros[4] = registros[30]; // uso el KS
    calcularPunteroFisico(1);
    int base = registros[5] & 0xFFFF;
    for (int i = 0; i < tamConst; i++){
        fread(&byte,1,1,f);
        memoria[base+i] = byte;
    }
}


void inicializarSegm(unsigned int tamCod, unsigned int tamData, unsigned int tamExtra, unsigned int tamStack, unsigned int tamConst, unsigned int tamParam){
    int i = 0, y = -1; // i para recorrer el vec, y para recorrer la tablaSegm
    unsigned int vec[6] = {tamParam,tamConst,tamCod,tamData,tamExtra,tamStack}; // este orden es crucial
    int aux;

    for (;i<6;i++){
        if (vec[i] > 0){
            y++;
            if (y == 0)
                tablaSegm[y].base = 0;
            else
                tablaSegm[y].base = tablaSegm[y-1].base;
            tablaSegm[y].tamaño = vec[i];
            aux = y;
            aux = aux << 16;
            registros[26+i] = aux;
        }
        else{
            registros[26+i] = -1;
        }
    }
    for (y++;y<CANT_SEGM;y++){
        tablaSegm[y].base = -1;
        tablaSegm[y].tamaño = -1;
    }

}








int main(int argc, char *argv[]){
    // arg0 -> "vmx"
    // arg1 -> "filename.vmx"
    // arg2 -> "-d" (opcional)

    FILE *f;
    char *archivoBin;
    char identif[6], identifLect[5], byteVersion;
    unsigned char byte1, byte2;
    int version, disassembler;
    unsigned short int tamCod,tamData,tamExtra,tamStack,tamConst,tamParam,offsetEntryPoint;

    printf("\n");
    archivoBin = argv[1]; 
    f = fopen(archivoBin,"rb");

    if (f == NULL)
        return 1;
    else {

        fread(identifLect,1,5,f);
        for (int i=0;i<5;i++)
            identif[i] = identifLect[i];
        identif[5] = '\0';

        if (strcmp(identif,"VMX26") != 0){
            return 1;
        }

        fread(&byteVersion,1,1,f);
        version = byteVersion;
        
        tamCod = leerTam(f);

        if (version == 1){
            inicializarVersion1(f,tamCod);

            disassembler = ((argc >= 3) && (strcmp(argv[2],"-d") == 0));
            if (disassembler)
                mostrarAssembler(tamCod,1);

            procesa();
            return 0;
        }

        if (version == 2){
            // obtener tamMemoria
            unsigned int tamMemoria;

            tamData = leerTam(f);
            tamExtra = leerTam(f);
            tamStack = leerTam(f);
            tamConst = leerTam(f);
            offsetEntryPoint = leerTam(f);

            inicializarVersion2(f,tamCod,tamData,tamExtra,tamStack,tamConst,tamParam, tamMemoria, offsetEntryPoint);

            disassembler = ((argc >= 3) && (strcmp(argv[2],"-d") == 0)); // cambiar condición
            if (disassembler)
                mostrarAssembler(tamCod,2);            
            
            procesa();
            return 0;
        }
    }
}