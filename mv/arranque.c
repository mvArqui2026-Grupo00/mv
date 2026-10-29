#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procesador.h"
#include "puntFunc.h"
#include "funcionesAuxiliares.h"
#include "estructuras.h"
#include "disassembler.h"

#define MAX_CAR_PARAM 50
#define MAX_PARAM 25

char parametros[MAX_PARAM][MAX_CAR_PARAM];
int version, disassembler, cantParam, offsetVectorParam;
unsigned short int tamCod, tamData, tamExtra, tamStack, tamConst, tamParam, offsetEntryPoint;

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


void inicializarCodeSegment(FILE *f) { // se debe pasar el puntero f referenciando al comienzo del código
    unsigned char byte;
    registros[4] = registros[26];
    calcularPunteroFisico(1);
    int base = registros[5] & 0xFFFF;
    for (int i = 0; i < tamCod; i++){
        fread(&byte,1,1,f);
        memoria[base+i] = byte;
    }
}

void inicializarConstSegment(FILE *f) { // se debe pasar el puntero f referenciando al comienzo de las constantes
    unsigned char byte;
    registros[4] = registros[30]; // uso el KS
    calcularPunteroFisico(1);
    int base = registros[5] & 0xFFFF;
    for (int i = 0; i < tamConst; i++){
        fread(&byte,1,1,f);
        memoria[base+i] = byte;
    }
}

void inicializarParamSegment(){
    int indiceCeldas = 0;
    int punterosLogicos[cantParam];
    
    for (int i=0; i < cantParam; i++){
        punterosLogicos[i] = indiceCeldas; // dado que los 2 primeros bytes siempre son 0, simplifico solo asignando offset
        strcpy(memoria + indiceCeldas,parametros[i]);
        indiceCeldas += strlen(parametros[i]);
    }
    offsetVectorParam = indiceCeldas;

    for (int i=0; i < cantParam; i++){
        escribirMemoriaArranque(indiceCeldas,punterosLogicos[i]);
        indiceCeldas += 4;
    }
    
    tamParam = indiceCeldas;
}

void inicializarSegm(){
    int i = 0, y = -1; // i para recorrer el vec, y para recorrer la tablaSegm
    unsigned int vec[6] = {tamParam,tamConst,tamCod,tamData,tamExtra,tamStack}; // este orden es crucial
    int aux;

    for (;i<6;i++){
        if (vec[i] > 0){
            y++;
            if (y == 0)
                tablaSegm[y].base = 0;
            else
                tablaSegm[y].base = tablaSegm[y-1].base + tablaSegm[y-1].tamaño;
            tablaSegm[y].tamaño = vec[i];
            aux = y;
            aux = aux << 16;

            if (i == 0)
                registros[31] = aux;
            else if(i == 1)
                registros[30] = aux;
            else if(i == 2)
                registros[26] = aux;
            else if(i == 3)
                registros[27] = aux;
            else if(i == 4)
                registros[28] = aux;
            else if(i == 5)
                registros[29] = aux;
            
        }
        else{
            if (i == 0)
                registros[31] = -1;
            else if(i == 1)
                registros[30] = -1;
            else if(i == 2)
                registros[26] = -1;
            else if(i == 3)
                registros[27] = -1;
            else if(i == 4)
                registros[28] = -1;
            else if(i == 5)
                registros[29] = -1;
        }
    }
    for (y++;y<CANT_SEGM;y++){
        tablaSegm[y].base = -1;
        tablaSegm[y].tamaño = -1;
    }

//    26 -> CS (Puntero al inicio del Code Segment)
//    27 -> DS (Puntero al inicio del Data Segment)
//    28 -> ES (Puntero al inicio del Extra Segment)
//    29 -> SS (Puntero al inicio del Stack Segment)
//    30 -> KS (Puntero al inicio del Const Segment)
//    31 -> PS (Puntero al inicio del Param Segment)
}

void inicializarVersion1(FILE *f){
    tamMemoria = CANT_CELDAS;
    inicializarMemoria();
    tamData = CANT_CELDAS - tamCod;
    tamExtra = tamStack = tamConst = tamParam = 0;
    inicializarSegm();
    inicializarCodeSegment(f);
    fclose(f);

    registros[0] = 0x00000000;
    inicializarPunteroFunciones(1);
}

void inicializarVersion2(FILE *f){
    inicializarMemoria();
    if (cantParam > 0)
        inicializarParamSegment();
    inicializarSegm();
    comprobarMemoriaInsuficiente();
    inicializarCodeSegment(f);
    inicializarConstSegment(f);
    fclose(f);

    registros[0] = registros[26];
    registros[0] += offsetEntryPoint;

    registros[7] = registros[29] + tablaSegm[devolverByte(registros[29],2)].tamaño; // sp inicializado en ss + SS.Tamaño (posición fuera del Stack Segment)
    if (cantParam > 0){
        int dirFisica = tablaSegm[devolverByte(registros[29],2)].base;
        dirFisica += tablaSegm[devolverByte(registros[29],2)].tamaño;
        dirFisica -= 4;

        escribirMemoriaArranque(dirFisica,offsetVectorParam); // *argv

        dirFisica-=4;
        escribirMemoriaArranque(dirFisica,cantParam); // argc

        dirFisica-=4;
        escribirMemoriaArranque(dirFisica,-1); // ret

        registros[7] -= 12;
    }

    inicializarPunteroFunciones(2);
}



int main(int argc, char *argv[]){
    // arg0 -> "vmx"
    // vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN]

    // para ejecutar correctamente la vmx, hay que pasar los argumentos en el orden ya establecido, se pueden omitir argumentos
    // pero no cambiar el orden
    // luego podríamos hacerlo más flexible esto, pero por ahora es así

    FILE *f;
    int indiceArg = 1; // para moverme por los argumentos de la ejecución de la mv
    char *paramVmx = ".vmx", *paramVmi = ".vmi", *paramMem = "m=", *paramDis = "-d", *paramParam = "-p";
    char archivoBin[100], *inicioExtension; // *archivoImagen -> lo pasé a funciones auxiliares, para poder usarlo como variable global en dicho módulo

    strcpy(archivoBin,"");
    strcpy(archivoImagen,"");

    char identif[6], identifLect[5], byteVersion;
    unsigned char byte1, byte2;

    printf("\n");

    inicioExtension = argv[indiceArg] + strlen(argv[indiceArg]) - 4; // bin = a.vmx\0 ; strlen -> 5; bin + 5 - 4 apunta a ".vmx\0" (strlen no considera \0)
    if (strcmp(inicioExtension,paramVmx) == 0){
        strcpy(archivoBin,argv[indiceArg]);
        indiceArg++;
    }
        
    hayImagen = 0;
    if (indiceArg < argc){

        if (((strlen(argv[indiceArg]) - 4)) >= 0){
            inicioExtension = argv[indiceArg] + strlen(argv[indiceArg]) - 4; // acá está el error
            if (strcmp(inicioExtension,paramVmi) == 0){
                strcpy(archivoImagen,argv[indiceArg]);
                indiceArg++;
                hayImagen = 1;
            }
        }
    }
    
    tamMemoria = CANT_CELDAS;
    if (indiceArg < argc && strncmp(argv[indiceArg],paramMem,2) == 0){
        char c;
        int i;
        for (i=0; i<strlen(argv[indiceArg]); i++){
            c = argv[indiceArg][i];
            if (c >= '0' && c <= '9') // me muevo hasta encontrar el primer dígito
                break;
        }
        int num = atoi(argv[indiceArg] + i);
        // si atoi no lee correctamente devuelve 0; tenerlo en cuenta
        // existe tmb strtol, pero me pareció más complicado

        tamMemoria = num;
        indiceArg++;
    }

    disassembler = 0;
    if (indiceArg < argc && strcmp(argv[indiceArg],paramDis) == 0){        
        disassembler = 1;
        indiceArg++;
    }
    if (archivoBin){ // si no se carga un archivoBin no tomamos ningunos de los parámetros
        tamParam = cantParam = 0;
        if (indiceArg < argc && strcmp(argv[indiceArg],paramParam) == 0){
            int indiceParam = 0;
            strcpy(parametros[0],archivoBin);

            indiceParam++;
            while (indiceArg + indiceParam < argc)
                strcpy(parametros[indiceParam],argv[indiceArg + indiceParam]);
                indiceParam++;

            cantParam = indiceParam;
        }
    }


    if (archivoBin){
        // esto es en caso de que haya un parametro .vmx
        f = fopen(archivoBin,"rb");
        if (f == NULL){
            return 1;
        }
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
                inicializarVersion1(f);

                if (disassembler)
                    mostrarAssembler(1,-1);

                procesa();
                return 0;
            }

            if (version == 2){

                tamData = leerTam(f);
                tamExtra = leerTam(f);
                tamStack = leerTam(f);
                tamConst = leerTam(f);
                offsetEntryPoint = leerTam(f);

                inicializarVersion2(f);
 
                if (disassembler)
                    mostrarAssembler(2,offsetEntryPoint);            
 
                procesa();
 
                return 0;
            }
        }
    }

    else { // no se pasó un .vmx como parámetro
        if (strlen(archivoImagen) == 0)
            return 1;
        else {
            f = fopen(archivoImagen,"rb");
            if (f == NULL)
                return 1;
            else{
                restaurarVmx();

                if (disassembler)
                    mostrarAssembler(2,offsetEntryPoint);

                procesa();
                return 0;
            }
        }
    }
}