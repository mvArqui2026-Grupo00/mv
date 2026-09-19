#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"
#include <stdio.h>

//  CONSTANTES: LIMITES INT
#define INT_MAX 0x7FFFFFFF
#define INT_MIN 0x80000000

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

void sysRead(){
    int direccionLogica = registros[13]; // EDX
    int modo = registros[10];            // EAX
    int cantidad = registros[12] & 0xFFFF;       // ECX (2 bytes bajos): cantidad de valores
    int tamano = (registros[12] >> 16) & 0xFFFF; // ECX (2 bytes altos): tamaño de cada valor

    for (int i = 0; i < cantidad; i++){
        int dirFisica = traducirDireccion(direccionLogica, tamano);
        if (dirFisica < 0) return; // fallo de segmento

        int valor = 0;
        char texto[33]; // solo para el binario: hasta 32 bits + '\0'

        printf("[%04X]: ", dirFisica);
        switch (modo){
            case 1: // decimal
                scanf("%d", &valor);
                break;
            case 2: // caracter
                scanf(" %c", (char*)&valor);
                break;
            case 4: // octal
                scanf("%o", &valor);
                break;
            case 8: // hexadecimal
                scanf("%x", &valor);
                break;
            case 16: // binario: se lee como texto y se arma el numero bit a bit
                scanf("%32s", texto);
                for (int k = 0; texto[k] != '\0'; k++)
                    valor = (valor << 1) | (texto[k] - '0');
                break;
        }

        registros[6] = valor; // MBR
        for (int b = tamano - 1; b >= 0; b--){ // de la ultima celda a la primera
            memoria[dirFisica + b] = valor & 0xFF; // guarda el byte menos significativo
            valor = valor >> 8;                    // pasa al siguiente byte
        }

        direccionLogica += tamano;
    }
}

void sysWrite(){
    int direccionLogica = registros[13]; // EDX
    int modo = registros[10];            // EAX (puede tener varios modos a la vez)
    int cantidad = registros[12] & 0xFFFF;       // ECX (2 bytes bajos): cantidad de valores
    int tamano = (registros[12] >> 16) & 0xFFFF; // ECX (2 bytes altos): tamaño de cada valor

    for (int i = 0; i < cantidad; i++){
        int dirFisica = traducirDireccion(direccionLogica, tamano);
        if (dirFisica < 0) 
          return; // fallo de segmento

        int valor = 0;
        for (int b = 0; b < tamano; b++) // arma el valor con los bytes de memoria (el primero es el mas significativo)
            valor = (valor << 8) | (unsigned char)memoria[dirFisica + b];
        registros[6] = valor; // MBR

        printf("[%04X]:", dirFisica);

        if (modo & 0x10){ // binario
            int empezo = 0; // pasa a 1 cuando aparece el primer bit en 1 (para no mostrar ceros a la izquierda)
            printf(" 0b");
            for (int b = 31; b >= 0; b--){
                int bit = (valor >> b) & 1;
                if (bit == 1) 
                  empezo = 1;
                  if (empezo) 
                    printf("%d", bit);
            }
            if (!empezo) 
              printf("0"); // el valor era 0
        }
        if (modo & 0x08) 
          printf(" 0x%X", valor); // hexadecimal

        if (modo & 0x04) 
          printf(" 0o%o", valor); // octal
          
        if (modo & 0x02){ // caracteres
            printf(" ");
            for (int b = 0; b < tamano; b++){
                char c = memoria[dirFisica + b];
                if (c >= 32 && c <= 126) printf("%c", c);
                else printf("."); // no imprimible
            }
        }
        if (modo & 0x01) 
          printf(" %d", valor); // decimal

        printf("\n");
        direccionLogica += tamano;  // avanza
    }
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

void setCC(int val1, int val2, int ans){    //  Modifica CC (registro 17)
    aux = 0;
    if( ans == 0 )
        aux = 0b0100;                   //  Z: Cero
    else
        if( ans < 0 )
            aux = 0b1000;               //  N: Negativo

    if((val1 > 0) && (val2 > 0))
        if( val1 > INT_MAX - val2)
            aux = aux | 0b0001;         //  V: Overflow
    else
        if((val1 < 0) && (val2 < 0))    //  V: Overflow
            if( val1 < INT_MIN - val2)
                aux = aux | 0b0001;

                                        // CARRY ???

    aux = aux << 28;    //  NZCV << 28  =  NZCV0000 00000000 00000000 00000000
    registros[17] = aux //  Guarda AUX en REGISTRO CC
}


void mov(){
    int valor2 = leerOperando(registros[3]);    //  Guarda valor de OP2 en valor2
    int valor1 = valor2;

    setCC(valor1, valor2, valor1);

    escribirOperando(registros[2], valor1); //  Escribe valor2 en OP1;
}

void add(){
    int valor1 = leerOperando(registros[2]);//  Guarda valor de OP1 en valor1
    int valor2 = leerOperando(registros[3]);//  Guarda valor de OP2 en valor2
    int suma = valor1 + valor2;

    setCC(valor1, valor2, suma);

    escribirOperando(registros[2], suma); //  Escribe 'suma' en OP1;
}

void sub(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resta = valor1 - valor2;

    setCC(valor1, valor2, resta);

    escribirOperando(registros[2], resta); //  Escribe 'resta' en OP1;
}

void mul(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int producto = valor1 * valor2;             //  Guarda multiplicacion en 'producto'

    setCC(valor1, valor2, producto);

    escribirOperando(registros[2], producto);   //  Escribe 'producto' en OP1;
}

void div(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int cociente;
    int resto;
    //  !!!
    if (valor2 != 0){                 //  Verifica si el contenido de OP2 es 0
        cociente = valor1 / valor2;   //  Realiza DIVISION ENTERA
        resto = valor1 % valor2;      //  Guarda el RESTO

        setCC(valor1, valor2, cociente);

        escribirOperando(registros[2], cociente);   //  Guarda DIVISION ENTERA en OP1
        escribirOperando(registros[16], resto);     //  Guarda RESTO en AC
    }
    else
        //  Si el valor del operando 2 ES CERO,
        //  DETIENE EL PROGRAMA POR COMPLETO
        stop();
}

void cmp(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int diferencia = valor1 - valor2;       //  Realiza DIFERENCIA entre valor1 y valor2

    setCC(valor1, valor2, diferencia);      //  Modifica CC con respecto a la diferencia anterior
}

void and(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int ans = valor1 & valor2;              //  Guarda AND LOGICO en 'ans'

    setCC(valor1, valor2, ans);

    escribirOperando(registros[2], ans);    //  Escribe 'ans' en OP1;
}

void or(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int ans = valor1 | valor2;              //  Guarda OR LOGICO en 'ans'

    setCC(valor1, valor2, ans);

    escribirOperando(registros[2], ans);    //  Escribe 'ans' en OP1;

}

void xor(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int ans = valor1 ^ valor2;              //  Guarda XOR LOGICO en 'ans'

    setCC(valor1, valor2, ans);

    escribirOperando(registros[2], ans);    //  Escribe 'ans' en OP1;
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
    int operando = registros[2]; // op1
    int valor = leerOperando(operando); // valor real de A
    int cantidad = leerOperando(registros[3]); // op2

    valor = valor << cantidad;

    escribirOperando(operando, valor);
    setCC(valor, 0, 0);
}

void shr(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando); // valor real de A (no el descriptor)
    int cantidad = leerOperando(registros[3]); // op2

    valor = valor >> cantidad;

    escribirOperando(operando, valor);
    setCC(valor, 0, 0);
}

void sar(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando); // valor real de A
    int cantidad = leerOperando(registros[3]); // op2

    valor = valor >> cantidad; // shift aritmético (con signo, preserva el bit de signo)

    escribirOperando(operando, valor);
    setCC(valor, 0, 0);
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
