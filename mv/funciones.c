#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"

/*void devolverOperandos(int tipo1, int tipo2, int * op1,int * op2){

}*/ // implementamos o no?

// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

void notDefined(){
    printf("flaco sos un pelotudo");    // mira vos
}

int leerOperando(int descriptor){
    int tipo = (descriptor >> 24) & 0xFF;
    int datosOperando = descriptor & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            return registros[datosOperando & 0x1F];

        case 2: // inmediato
            return (int)(short int)(datosOperando & 0xFFFF); // sign-extend 16->32 bits

        case 3: // memoria
            return leerDeMemoria(datosOperando);

        default: // ninguno
            return 0;
    }
}

void escribirOperando(int descriptor, int valor){
    int tipo = (descriptor >> 24) & 0xFF;
    int datosOperando = descriptor & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            registros[datosOperando & 0x1F] = valor;
            break;

        case 3: // memoria
            escribirEnMemoria(datosOperando, valor);
            break;

        default: // ninguno o inmediato (no son destino válido)
            break;
    }
}

//Carga un numero aleatorio entro 0 y el segundo operando al primer operando
void rnd(){
    int operandoDestino = registros[2]; // OP1: descriptor del operando A (destino)
    int limite = leerOperando(registros[3]); // OP2: descriptor del operando B
    int valor = rand() % (limite + 1); // número aleatorio entre 0 y limite (inclusive)
    escribirOperando(operandoDestino, valor);
}

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

void sys(){
    int operando = registros[2]; // OP1: único operando (código de llamada al sistema)
    int numeroLlamada = leerOperando(operando);

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
