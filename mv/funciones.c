#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"

//void devolverOperandos(){


// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

// el dato lo "casteo" a unsigned int para hacer el shift lógico
char devolverByte(unsigned int dato, int nroByte){ // que nombre le ponemos a los parámetros
    // el nro byte es el byte a devolver, siendo 0 el byte de menor significado, 1: el segundo de menor significado, ....
    dato = dato >> 8 * nroByte;
    dato &= 0xFF;
}

// falta configurar el mbr, y un par de cosas más

int leerOperando(int operando, int cantBytes){
    int tipo = (devolverByte(operando,3));
    int datosOperando = operando & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            return registros[datosOperando & 0x1F];

        case 2: // inmediato
            return (datosOperando & 0xFFFF);

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(cantBytes);
            leerDeMemoria();
        default: // ninguno
            return 0; // está bien esto??
    }
}

void escribirOperando(int operando, int valor, int cantBytes){
    int tipo = devolverByte(operando,3);
    int datosOperando = operando & 0xFFFFFF;
    int aux;
    switch (tipo){
        case 1: // registro
            registros[datosOperando & 0x1F] = valor;
            break;

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(cantBytes);
            aux = leerOperando(valor,4);
            escribirMBR(aux);
            escribirEnMemoria();
            break;

        default: // ninguno o inmediato (no son destino válido)
            break;
    }
}

void leerDeMemoria(){
    int n; // cant bytes a leer
    n = devolverByte(registros[5],3);
    n = n << 8;
    n |= devolverByte(registros[5],2);

    int aux = 0; // por las dudas que el n sea 0, hay que verificarlo en el testing
    for(int i=0; i < n; i++){
        aux = aux << 8;
        aux = aux | memoria[registros[5] + i];
    }
    escribirMBR(aux);
}


void escribirEnMemoria(){
    int mbr;
    int n; // cant bytes a escribir
    mbr = registros[6];
    n = devolverByte(registros[5],3);
    n = n << 8;
    n |= devolverByte(registros[5],2);

    int aux = 0; // por las dudas que el n sea 0, hay que verificarlo en el testing
    for(int i=0; i < n; i++){
        memoria[registros[5]+i] = devolverByte(mbr,n-1 -i);
    }
}

void escribirMBR(int valor){ // configura el MBR
    registros[6] = valor;
}

void calcularPunteroLogico(int operandoMemoria){ // configura el LAR
    // lar -> reg[4]
    int nroReg = devolverByte(operandoMemoria,0);
    int puntLog = registros[nroReg];
    int aux = devolverByte(operandoMemoria,2);
    aux = aux << 1;
    aux = aux | devolverByte(operandoMemoria,1);
    puntLog += aux; // acá habría que verificar si no nos caímos del segmento   
    registros[4] = puntLog; // reg[4] -> LAR
}

void calcularPunteroFisico(int cantidadDeBytes){ // configura el MAR
    //return puntFisico;
    int puntLogico = registros[4]; // lar
    int nroSegm = devolverByte(puntLogico,2);
    int puntFisico = tablaSegm[nroSegm].base; // dirección base
    int aux = devolverByte(puntLogico,1);
    aux = aux << 1;
    aux = aux | devolverByte(puntLogico,0);
    puntFisico += aux; // + offset
    // acá habría que verificar si no nos caímos del segmento
    cantidadDeBytes = cantidadDeBytes << 16;
    puntFisico |= cantidadDeBytes; // cargamos en la parte alta del MAR la cant de bytes a leer/escribir
    registros[5] = puntFisico; // reg[5] -> MAR
}


//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("flaco sos un pelotudo");    // mira vos
}


//--------------    UN OPERANDO   --------------------

void sys(){
    int operando = registros[2]; // OP1: único operando (código de llamada al sistema)
    int numeroLlamada = leerOperando(operando,4);

    switch (numeroLlamada){
            case 1: // READ
            sysRead();
            break;
        case 2: // WRITE
            sysWrite();
            break;
        default:
            break;
    }
}

int negative(){
    if (registros[17] & 0x80000000) // CC band 1erBit
        return 1;
    else
        return 0;
}
int cero(){
    if (registros[17] & 0x40000000) // CC band 2doBit
        return 1;
    else
        return 0;
}
int carry(){
    if (registros[17] & 0x20000000) // CC band 3roBit
        return 1;
    else
        return 0;
}
int overflow(){
    if (registros[17] & 0x10000000) // CC band 4toBit
        return 1;
    else
        return 0;
}

void jmp(){
    int aux = leerOperando(registros[2],4); // leer op1, 4 bytes
    aux = aux << 8;
    aux |= 26; // registro CS
    calcularPunteroLogico(aux);    
    calcularPunteroFisico(4); // está bien esto?? no voy a leer bytes, solo calcular la posFisica
    registros[0] = registros[5]; // IP = MAR
    // acá sería necesario hacer una validación para ver que no nos caemos del code segment
    // o en su defecto, llamar a una función que realice esta asignación y validación...
}

void jp(){
    if(!negative() && !cero()){
        jmp();
    }
}   
void jn(){
    if (negative())
        jmp();
}
void jz(){
    if (cero())
        jmp();
}
void jc(){
    if (carry())
        jmp();
}
void jv(){
    if (overflow())
        jmp();
}
void jnp(){
    if (negative() || cero())
        jmp();
}
void jnn(){
    if (!negative())
        jmp;
}
void jnz(){
    if (!cero()){
        jmp();
    }
}

void not(){
    int aux;
    //aux = devolverValor(); // función a implementar...?
    aux = ~aux; // ~ -> operador not binario
    // *(punt)= aux;
}



//--------------   SIN OPERANDOS  --------------------

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
}




//--------------   DOS OPERANDOS  --------------------
void mov(){}
void add(){}
void sub(){}
void mul(){}
void div(){}
void cmp(){}
void and(){}
void or(){}
void xor(){}

void swap(){
    int descA = registros[2]; // OP1: descriptor original de A
    int descB = registros[3]; // OP2: descriptor original de B
    xor(); // XOR A, B  →  A = A ^ B
    registros[2] = descB;
    registros[3] = descA;
    xor(); // XOR B, A  →  B = B ^ A
    registros[2] = descA;
    registros[3] = descB;
    xor(); // XOR A, B  →  A = A ^ B
}

void shl(){}
void shr(){}
void sar(){}


//Carga los 2 bytes menos significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2)
void ldl(){     
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA);
    int valorB = leerOperando(operandoB);
    int resultado = (valorA & 0xFFFF0000) | (valorB & 0xFFFF); // conserva los 16 bits altos de A, reemplaza los 16 bits bajos con los bajos de B
    escribirOperando(operandoA, resultado);
}

//Carga los 2 bytes más significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2).
void ldh(){
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA);
    int valorB = leerOperando(operandoB);
    int resultado = (valorA & 0x0000FFFF) | ((valorB & 0xFFFF) << 16); // conserva los 16 bits bajos de A, reemplaza los 16 bits altos con los bajos de B

    escribirOperando(operandoA, resultado);
}

//Carga un numero aleatorio entro 0 y el segundo operando al primer operando
void rnd(){
    int operandoDestino = registros[2]; // OP1: descriptor del operando A (destino)
    int limite = leerOperando(registros[3]); // OP2: descriptor del operando B
    int valor = rand() % (limite + 1); // número aleatorio entre 0 y limite (inclusive)
    escribirOperando(operandoDestino, valor);
}
