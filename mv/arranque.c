#include "procesador.c" //no hacemos include de los demás módulos o librerias porque ya están incluidas en la concatenación de include...

void inicializarMemoria(short int tamCod, FILE *f) {
    printf("hola!!!\n");
    unsigned char byte;
    for (int i = 0; i < tamCod; i++){
        fread(&byte,1,1,f);
        memoria[i] = byte;
        printf("\n%2X\n",byte);
    }
}

void inicializarTablaSegm(short int tamCod){
    tablaSegm[0].base = 0;
    tablaSegm[0].tamaño = tamCod;
    tablaSegm[1].base = tamCod;
    tablaSegm[1].tamaño = CANT_CELDAS - tamCod;

    for (int i = 2; i < 8; i++){
        tablaSegm[i].base = -1;
        tablaSegm[i].tamaño = -1;
    }
}

void inicializarRegistros(){
    registros[26] = 0x00000000; // cs
    registros[27] = 0x00010000; // ds
    registros[0] = registros[26] ; // ip
}

int main(int argc, char *argv[]){
    // arg0 -> "vmx"
    // arg1 -> "filename.vmx"
    // arg2 -> "-d" (opcional)
    char *archivoBin = argv[1]; 
    printf("%s\n",archivoBin);
    FILE *f = fopen(archivoBin,"rb");
    if (f != NULL){
        char identif[5];
        char byteVersion;
        char byte1, byte2;

        int version;
        short int tamCod;

        fread(identif,1,5,f);

        fread(&byteVersion,1,1,f);
        version = byteVersion;
        
        // es necesario armar el numero de 2 bytes a mano
        // porque el compilador de C trabaja con Little-Endian
        // y el traductor trabaja con Big-Endian
        // entonces al leer directamente al número lo lee al revés
        fread(&byte1,1,1,f);
        fread(&byte2,1,1,f);
        tamCod = byte1;
        tamCod = tamCod << 8;
        tamCod |= byte2;

        printf("identif: %s \nVersion: %d \nTamaño del Código: %d bytes \n\n",identif,version,tamCod);

        if ((strcmp(identif,"VMX26") == 0) && version == 1){ // es necesario el (version == 1) ?
            printf("archivo válido\n\n");
            inicializarMemoria(tamCod,f); // pasamos el puntero a archivo apuntando al inicio del "code segment"
            fclose(f);
            inicializarTablaSegm(tamCod);
            inicializarPunteroFunciones();

            disassembler = (argc >= 3 && argv[2] == "-d");

            procesa();
            return 0;
        }
    }
    else
        printf("no se pudo abrir el binario, loco \n");
    return 1;
}