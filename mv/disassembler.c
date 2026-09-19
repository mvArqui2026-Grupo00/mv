
void mostrarAssembler(short int direccionInstruccion){
    int dirFisica = tablaSegm[registros[26] & 0x00FF0000].base + direccionInstruccion;
    printf("[%4X] ",direccionInstruccion);
    printf("%2X ",registros[1]);
    int op1 = registros[2] & 0x00FFFFFF;
    int op1 = registros[3] & 0x00FFFFFF;
    for(int i=0; i < ){

    }
    for(){

    }

    printf("\n");

}