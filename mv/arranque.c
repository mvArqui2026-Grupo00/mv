#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procesador.h"
#include "puntFunc.h"
#include "funcionesAuxiliares.h"
#include "estructuras.h"
#include "disassembler.h"

void inicializarVersion1(FILE *f,int tamCod){
    inicializarMemoria(1,tamCod,f); // pasamos el puntero a archivo apuntando al inicio del "code segment"
    fclose(f);
    inicializarTablaSegm(1,tamCod);
    inicializarRegistros(1);
    inicializarPunteroFunciones(1);
}

void inicializarVersion2(FILE *f,unsigned int tamCod, unsigned int tamData, unsigned int tamExtra, unsigned int tamStack, unsigned int tamConst, unsigned int tamParam, unsigned int offsetEntryPoint){
    inicializarTablaSegmVersion2(tamCod,tamData,tamExtra,tamStack,tamConst,tamParam);
    inicializarCodeSegment(tamCod,f);
    fclose(f);
    inicializarRegistros(2);
    registros[0] += offsetEntryPoint;
    inicializarPunteroFunciones(2);
}

void inicializarCodeSegment(short int tamCod, FILE *f) { // cambio de nombre???
    unsigned char byte;
    int base = registros[26];
    for (int i = 0; i < tamCod; i++){
        fread(&byte,1,1,f);
        memoria[base+i] = byte;
        //printf("\n%02X\n",byte);
    }
}

void inicializarTablaSegm(int version,short int tamCod){ // CAMBIAR
    if (version == 1){
        tablaSegm[0].base = 0;
        tablaSegm[0].tamaño = tamCod;
        tablaSegm[1].base = tamCod;
        tablaSegm[1].tamaño = CANT_CELDAS - tamCod;

        for (int i = 2; i < 8; i++){
            tablaSegm[i].base = -1;
            tablaSegm[i].tamaño = -1;
        }
    }
    else{

    }
}

void inicializarTablaSegmVersion2(unsigned int tamCod, unsigned int tamData, unsigned int tamExtra, unsigned int tamStack, unsigned int tamConst, unsigned int tamParam){
    int i = 0, y = -1; // i para recorrer el vec, y para recorrer la tablaSegm
    unsigned int vec[6] = {tamCod,tamData,tamExtra,tamStack,tamConst,tamParam};
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

void inicializarRegistros(int version){ // CAMBIAR
    if (version == 1){
        registros[26] = 0x00000000;
        registros[27] = 0x00000000;
        registros[0] = registros[26];
    }
    else{
        registros[0] = registros[26] ; // ip        
        registros[7] = registros[29] + tablaSegm[devolverByte(registros[29],2)].tamaño; // sp inicializado en ss + SS.Tamaño (posición fuera del Stack Segment)
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
    if (f != NULL){
        

        fread(identifLect,1,5,f);
        for (int i=0;i<5;i++)
            identif[i] = identifLect[i];
        identif[5] = '\0';

        fread(&byteVersion,1,1,f);
        version = byteVersion;
        
        fread(&byte1,1,1,f);
        fread(&byte2,1,1,f);
        tamCod = byte1;
        tamCod = tamCod << 8;
        tamCod |= byte2;

        if (version == 2){
            // DATA
            fread(&byte1,1,1,f);
            fread(&byte2,1,1,f);
            tamData = byte1;
            tamData = tamCod << 8;
            tamData |= byte2;

            // EXTRA
            fread(&byte1,1,1,f);
            fread(&byte2,1,1,f);
            tamExtra = byte1;
            tamExtra = tamCod << 8;
            tamExtra |= byte2;

            // STACK
            fread(&byte1,1,1,f);
            fread(&byte2,1,1,f);
            tamStack = byte1;
            tamStack = tamCod << 8;
            tamStack |= byte2;

            // CONST
            fread(&byte1,1,1,f);
            fread(&byte2,1,1,f);
            tamConst = byte1;
            tamConst = tamCod << 8;
            tamConst |= byte2;

            // Offset Entry Point
            fread(&byte1,1,1,f);
            fread(&byte2,1,1,f);
            offsetEntryPoint = byte1;
            offsetEntryPoint = tamCod << 8;
            offsetEntryPoint |= byte2;
        }


        if ((strcmp(identif,"VMX26") == 0) && (version == 1 || version == 2)){
            if (version == 1) {
                inicializarVersion1(f,tamCod);

                disassembler = ((argc >= 3) && (strcmp(argv[2],"-d") == 0));
                if (disassembler)
                    mostrarAssembler(tamCod);
            }
            else { // version == 2
                inicializarVersion2(f,tamCod,tamData,tamExtra,tamStack,tamConst,tamParam,offsetEntryPoint);
            }
            procesa();
            return 0;
        }
    }
    return 1;
}