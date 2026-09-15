#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"

//  FUNCIONES AUXILIARES

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

//  A DESARROLLAR
void leerDeMemoria(int operandoMemoria){}

void escribirEnMemoria(int operandoMemoria, int valor){}

int calcularPunteroLogico(int operandoMemoria){
    // return puntLogico
}

int calcularPunteroFisico(int puntLogico){
    //return puntFisico;
}



//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("flaco sos un pelotudo");    // mira vos
}


//--------------   SIN OPERANDOS  --------------------

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
}

//--------------    UN OPERANDO   --------------------

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
    int offset = registros[2]; // la idea es obtener el valor a desplazar
    // ahora esto está mal, el 2do operando tiene el valor a desplazar si es un inmediato, si es un registro o memoria hay que obtenerlo de una manera diferente
    registros[0] = registros[27] + offset; // ip = ds + offset
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

    CC();
}


//--------------   DOS OPERANDOS  --------------------

void CC(){    //  Modifica CC (registro 17)

}


void mov(){
    int valor2;
    valor2 = leerOperando(registros[3]);    //  Guarda valor de OP2 en valor2
    escribirOperando(registros[2], valor2); //  Escribe valor2 en OP1;

    CC();
}

void add(){
    int valor1 = leerOperando(registros[2]);//  Guarda valor de OP1 en valor1
    int valor2 = leerOperando(registros[3]);//  Guarda valor de OP2 en valor2
    valor1 += valor2;                       //  Guarda suma en valor1
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
}

void sub(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    valor1 -= valor2;                       //  Guarda resta en valor1;
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
}

void mul(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    valor1 *= valor2;                       //  Guarda multiplicacion en valor1;
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
}

void div(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resultado;
    int resto;
    //  !!!
    if (valor2 != 0){ //  Verifica si el contenido de OP2 es 0

        resultado = valor1 / valor2;            //  Realiza DIVISION ENTERA
        resto = valor1 % valor2;                //  Guarda el RESTO
        escribirOperando(registros[2], valor1); //  Guarda DIVISION ENTERA en OP1
        escribirOperando(registros[16], resto); //  Guarda RESTO en AC

        CC();
    }
    else
        stop(); //  Si el valor del operando 2 ES CERO, DETIENE EL PROGRAMA POR COMPLETO
}

void cmp(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int aux = valor1 - valor2;              //  Realiza DIFERENCIA entre valor1 y valor2

    CC(aux);                                //  Modifica CC con respecto a la diferencia anterior
}

void and(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    valor1 = valor1 & valor2;               //  Guarda AND LOGICO en valor1
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
}

void or(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    valor1 = valor1 | valor2;               //  Guarda OR LOGICO en valor1
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
}

void xor(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    valor1 = valor1 ^ valor2;               //  Guarda XOR LOGICO en valor1
    escribirOperando(registros[2], valor1); //  Escribe valor1 en OP1;

    CC();
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

    CC();
}

void shl(){
    CC();
}
void shr(){
    CC();
}
void sar(){
    CC();
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

//Carga un numero aleatorio entro 0 y el segundo operando al primer operando
void rnd(){
    int operandoDestino = registros[2]; // OP1: descriptor del operando A (destino)
    int limite = leerOperando(registros[3]); // OP2: descriptor del operando B
    int valor = rand() % (limite + 1); // número aleatorio entre 0 y limite (inclusive)
    escribirOperando(operandoDestino, valor);
}
