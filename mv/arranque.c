#include "procesador.c" //no hacemos include de los demás módulos o librerias porque ya están incluidas en la concatenación de include...

void inicializarMemoria(short int tamCod, FILE *f) {
    char byte;
    for (int i = 0; i < tamCod; i++){
        fread(&byte,1,1,f);
        memoria[i] = byte;
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

int main(int argc, char *argv[]){ // el segundo parámetro es un supuesto mío de como sería el "-d", pero no estoy seguro que sea así ~Mauro
    // arg0 -> "mv"
    char *archivoBin = argv[1]; 
    FILE *f = fopen(archivoBin,"rb");
    char identif[5];
    char version;
    short int tamCod;
    // acá la buena práctica es poner sizeof(tipo), pero acá siempre queremos
    // "agarrar" la misma cantida de bytes, así que esto lo destaca
    fread(identif,1,5,f);
    fread(&version,1,1,f);
    fread(&tamCod,2,1,f);

    if (strcmp(identif,"VMX26") && version == '1'){ // es necesario el (version == 1) ?
        inicializarMemoria(tamCod,f); // pasamos el puntero a archivo apuntando al inicio del "code segment"
        fclose(f);
        inicializarTablaSegm(tamCod);
        inicializarPunteroFunciones();

        disassembler = (argc >= 3 && argv[2] == "-d");

        procesa();
        return 0;
    }

    return 1;
}