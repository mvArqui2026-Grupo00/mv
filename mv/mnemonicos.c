#include <stdio.h>
#include <stdlib.h>
#include <time.h> // para mnemónico rnd
#include <stdint.h>
#include "mnemonicos.h"
#include "funcionesAuxiliares.h"
#include "estructuras.h"


// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("Error: Mnemonico no Definido");
    exit(1);
}


//--------------   SIN OPERANDOS  --------------------

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
}

void ret(){
    escribirOperando(registros[2], registros[0]);   //  OP1 = IP
    pop();                                          //  POP IP    (IP ahora contiene la siguiente instruccion en la subrut. invocada)
}

//--------------    UN OPERANDO   --------------------

void push(){
    registros[7] -= 4;                      //  1 - Decrementar SP: SP-4

    if(registros[7] <= registros[29]){       //  2 - SP < SS => STACK OverFLOW
        printf("\n:: [!] STACK OVERFLOW\n\n");
        exit(1);
    }

    int valor = leerOperando( registros[2] ); //  3 - Obtener valor del operando (Ya devuelve 4 bytes con signo propagado)

    registros[4] = registros[7];///-------------------------
    calcularPunteroFisico(4);   ///-------------------------
    registros[6] = valor;       ///-------------------------

    escribirEnMemoria();        ///--------------------------
    //escribirOperando( registros[7], valor );  //  4 - En la memoria, almaceno el VALOR donde este apuntando el SP
}

void pop(){
    //int valor = leerOperando(registros[7]);   //  1 - Obtengo el valor que esta en Memoria apuntado por SP (Ya truncado)
    int valor;
    int posStack = (registros[29] >> 16) & 0xFFFF;
    int tamStack = tablaSegm[posStack].tamaño;
    int aux = registros[29] + tamStack; //  Direccion Stack Segment + Cantidad de bytes en SS

    if( registros[7] >= aux){                 //  2 - Si SP apunta a una direccion fuera del segmento de PILA => Stack UNDERflow
        printf("\n:: [!] STACK UNDERFLOW.\n\n");
        exit(1);
    }

    registros[4] = registros[7];///-------------------------
    calcularPunteroFisico(4);   ///-------------------------
    leerDeMemoria(4);           ///-------------------------
    valor = registros[6];       ///-------------------------

    escribirOperando(registros[2], valor);   //  3 - Asignar resultado al primer operando

    registros[7] += 4;                       //  4 - Incrementar SP: SP + 4
}

void call(){                                        //  CALL subrutina  (OP1 = subrutina)
    int op1aux = leerOperando(registros[2]);            //  aux = subrutina
    escribirOperando(registros[2], registros[0]);   //  OP1 = IP
    push();                                         //  PUSH IP
    escribirOperando(registros[2], op1aux);         //  OP1 = subrutina
    jmp();                                          //  JMP subrutina
}

//modos del Sys
void sysRead(){
    int direccionLogica = registros[13]; // EDX
    int modo = registros[10];            // EAX
    int cantidad = registros[12] & 0xFFFF;       // ECX (2 bytes bajos): cantidad de valores
    int tamano = (registros[12] >> 16) & 0xFFFF; // ECX (2 bytes altos): tamaño de cada valor

    int dirFisica;
    int valor;

    for (int i = 0; i < cantidad; i++){
        registros[4] = direccionLogica; // config lar
        comprobarFalloSegmento();
        calcularPunteroFisico(tamano); // config mar
        dirFisica = registros[5] & 0xFFFF;

        valor = 0;
        unsigned char texto[33]; // solo para el binario: hasta 32 bits + '\0'

        printf("[%04X]: ", dirFisica);

        switch (modo) {
            case 1: // decimal
                scanf("%d", &valor);
                break;
            case 2: // caracter
                unsigned char aux;
                scanf(" %c", &aux);
                valor = aux;
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
        escribirMBR(valor); // config mbr
        escribirEnMemoria();
        direccionLogica += tamano;
    }
}

void sysWrite(){
    int direccionLogica = registros[13]; // EDX
    int modo = registros[10];            // EAX (puede tener varios modos a la vez)
    int cantidad = registros[12] & 0xFFFF;       // ECX (2 bytes bajos): cantidad de valores
    int tamano = (registros[12] >> 16) & 0xFFFF; // ECX (2 bytes altos): tamaño de cada valor

    int dirFisica;
    int valor;

    for (int i = 0; i < cantidad; i++){
        registros[4] = direccionLogica; // config lar
        comprobarFalloSegmento();
        calcularPunteroFisico(tamano); // config mar
        leerDeMemoria();
        dirFisica = registros[5] & 0xFFFF;
        valor = registros[6];

        printf("[%04X]: ", dirFisica);

        if (modo & 0x10){ // binario
            int empezo = 0; // pasa a 1 cuando aparece el primer bit en 1 (para no mostrar ceros a la izquierda)
            printf("0b");
            for (int b = 31; b >= 0; b--){
                int bit = (valor >> b) & 1;
                if ( (!empezo) && (bit == 1))
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
            if (valor >= 32 && valor <= 126)
                printf(" %c", valor);
            else
                printf("."); // no imprimible
            //}
        }
        if (modo & 0x01)
            printf(" %d", valor); // decimal

        printf("\n");
        direccionLogica += tamano;
    }
}

void clearScreen(){
    printf("\033[2J\033[H");
    // \033 : es el caracter ESC
    // [ : es el aviso de que viene una secuencia de control con parámetros.
    // 2J : es el parametro de la orden, que indica que borre toda la pantalla
    // \033[H : Otra vez escape y que lleve el cursor a la posicion inicial
}

void breakPoint(){
    if (hayImagen){
        setearVmi();

        char linea[10];
        fgets(linea, sizeof(linea), stdin);

        switch(linea[0]){
            case 'g': pasoAPaso = 0;
                break;
            case 'q':
                exit(0);
            case '\n': pasoAPaso = 1;
                break;
        }
    }
}

void stringWrite(){
    int direccionLogica = registros[13]; // EDX: dirección lógica del string
    int caracter;
    int dirFisica;

    registros[4] = direccionLogica;      // LAR
    comprobarFalloSegmento();
    calcularPunteroFisico(1);            // MAR, 1 byte (tamaño de caracter)
    leerDeMemoria();
    caracter = registros[6];             // MBR

    dirFisica = registros[5] & 0xFFFF;
 //   printf("[%04X]: ", dirFisica);
 //   estos no van en los string write y read

    while (caracter != 0){               // hasta encontrar '\0'
        printf("%c", caracter);

        // leo el siguiente byte
        direccionLogica++;
        registros[4] = direccionLogica;
        comprobarFalloSegmento();
        calcularPunteroFisico(1);
        leerDeMemoria();
        caracter = registros[6];
    }
    printf("\n");
}

void stringRead(){
    int direccionLogica = registros[13]; // EDX donde guardar el string
    int limite = registros[12];          // ECX maximo de caracteres (-1 = sin límite)
    int contador = 0;
    int caracter;
    int dirFisica;

    registros[4] = direccionLogica;   // LAR
    calcularPunteroFisico(1);         // MAR, 1 byte
    dirFisica = registros[5] & 0xFFFF;
 //   printf("[%04X]: ", dirFisica);
 //   estos no van en los string write y read
    while ((limite == -1 || contador < limite) && (caracter = getchar()) != '\n' && caracter != EOF){ // corta con Enter o fin de entrada

        registros[4] = direccionLogica;   // LAR
        comprobarFalloSegmento();
        calcularPunteroFisico(1);         // MAR, 1 byte
        escribirMBR(caracter);            // MBR
        escribirEnMemoria();

        direccionLogica++;
        contador++;
    }

    // se escribe siempre el '\0' final
    registros[4] = direccionLogica;
    comprobarFalloSegmento();
    calcularPunteroFisico(1);
    escribirMBR(0);
    escribirEnMemoria();
    printf("\n");
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
        case 3:
            stringRead();
            break;
        case 4:
            stringWrite();
            break;
        case 0:
            clearScreen();
            break;
        case 15:
            breakPoint();
            break;
        default:
            break;
    }
}

// funciones auxiliares para Jumps Condicionales
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
    registros[0] &= 0xFFFF0000;                     //  IP = IP & FFFF0000
    registros[0] += leerOperando(registros[2]);     //  IP = IP + OP1

    registros[4] = registros[0];                    //  LAR = IP
    comprobarFalloSegmento();
}

void jp(){
    if(!negative() && !cero()){
        jmp();
    }
}
void jneg(){
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
        jmp();
}
void jnz(){
    if (!cero()){
        jmp();
    }
}

void not(){
    int aux = leerOperando(registros[2]);
    aux = ~aux;
    escribirOperando(registros[2], aux);
    setCC(aux,0,0);
}


//--------------   DOS OPERANDOS  --------------------
// función auxiliar que maneja la configuración del CC

void setCC(int resultado, int hayCarry, int hayOverflow){    //  Modifica CC (registro 17)
    int aux = 0;
    if (resultado == 0)
        aux = 0b0100;                   //  Z: Cero
    else
        if (resultado < 0)
            aux = 0b1000;               //  N: Negativo

    aux |= 0b0010 * hayCarry;
    aux |= 0b0001 * hayOverflow;

    aux = aux << 28;    //  NZCV << 28  =  0b NZCV0000 00000000 00000000 00000000
    registros[17] = aux; //  Guarda AUX en REGISTRO CC
}


void mov(){
    int valor = leerOperando(registros[3]);    //  Guarda valor de OP2 en valor

    setCC(valor, 0, 0); // analizo el valor (si es negativo o 0)
    escribirOperando(registros[2], valor); //  Escribe valor en lo que dicta OP1;
}

void verificarCarryOverflowSuma(int valor1, int valor2, int suma, int * carry, int * overflow){
    int64_t longSuma = (int64_t)valor1 + valor2;
    if ((longSuma >> 32) != 0)
        *carry = 1;

    if ((valor1 > 0 && valor2 > 0) && (suma <= 0))
        *overflow = 1;
    if ((valor1 < 0 && valor2 < 0) && (suma >= 0))
        *overflow = 1;
}

void add(){
    int valor1 = leerOperando(registros[2]);//  Guarda valor de OP1 en valor1
    int valor2 = leerOperando(registros[3]);//  Guarda valor de OP2 en valor2
    int suma = valor1 + valor2;

    int carry = 0;
    int overflow = 0;
    verificarCarryOverflowSuma(valor1,valor2,suma,&carry,&overflow);

    setCC(suma,carry, overflow);
    escribirOperando(registros[2], suma); //  Escribe 'suma' en OP1;
}

void sub(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resta = valor1 - valor2;

    int carry = 0;
    int overflow = 0;
    verificarCarryOverflowSuma(valor1,-valor2,resta,&carry,&overflow);

    setCC(resta, carry, overflow);
    escribirOperando(registros[2], resta); //  Escribe 'resta' en OP1;
}

void mul(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int producto = valor1 * valor2;             //  Guarda multiplicacion en 'producto'

    int carry = 0;
    int overflow = 0;
    int64_t longProducto = (int64_t)valor1 * (int64_t)valor2;

    if (((longProducto >> 32) & 0xFFFFFFFF) != 0)
        carry = overflow = 1;
    setCC(producto, carry, overflow);
    escribirOperando(registros[2], producto);   //  Escribe 'producto' en OP1;
}

void divis(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int cociente;
    int resto;

    if (valor2 != 0){
        cociente = valor1 / valor2;   //  Realiza DIVISION ENTERA
        resto = valor1 % valor2;      //  Guarda el RESTO

        setCC(cociente, 0, 0); // en la división no hay ni carry ni overflow
        escribirOperando(registros[2], cociente);   //  Guarda DIVISION ENTERA en OP1
        registros[16] = resto;     //  Guarda RESTO en AC
    }
    else{
        printf("Error: Division por cero\n");
        exit(1);
    }
}

void cmp(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int diferencia = valor1 - valor2;       //  Realiza DIFERENCIA entre valor1 y valor2

    int carry = 0;
    int overflow = 0;
    verificarCarryOverflowSuma(valor1,valor2,diferencia,&carry,&overflow);
    setCC(diferencia, carry, overflow);      //  Modifica CC con respecto a la diferencia anterior
}

void and(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resultado = valor1 & valor2;              //  Guarda AND LOGICO en 'ans'

    setCC(resultado,0,0);
    escribirOperando(registros[2], resultado);    //  Escribe 'ans' en OP1;
}

void or(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resultado = valor1 | valor2;

    setCC(resultado,0,0);
    escribirOperando(registros[2], resultado);}

void xor(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int resultado = valor1 ^ valor2;

    setCC(resultado,0,0);
    escribirOperando(registros[2], resultado);
}

void swap(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);

    valor1 = valor1 ^ valor2;
    valor2 = valor1 ^ valor2;
    valor1 = valor1 ^ valor2;

    setCC(valor1,0,0);
    escribirOperando(registros[2], valor1);
    escribirOperando(registros[3], valor2);
}

void shl(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando); // valor real de A
    int cantidad = leerOperando(registros[3]); // op2

    int resultado = valor << cantidad;

    int carry = 0;
    int overflow = 0;

    int64_t longResultado = (int64_t)valor << cantidad;
    if ((longResultado >> 32) != 0)
        carry = 1;

    if (valor > 0 && resultado <= 0)
        overflow = 1;
    if (valor < 0 && resultado >= 0)
        overflow = 1;

    setCC(resultado, carry, overflow);

    escribirOperando(operando, resultado);
}

void shr(){
    int operando = registros[2]; // op1
    int valorConSigno = leerOperando(operando); // valor real de A (no el descriptor)
    unsigned int valor = (unsigned int)valorConSigno;
    int cantidad = leerOperando(registros[3]); // op2

    valor = valor >> cantidad;

    int overflow = ((valorConSigno & 0x80000000) != 0); // si originalmente el número es negativo, hay overflow
    setCC(valor, 0, overflow);
    escribirOperando(operando, valor);

}

void sar(){
    int operando = registros[2]; // op1
    int valor = leerOperando(registros[2]); // valor real de A
    int cantidad = leerOperando(registros[3]); // op2

    valor = valor >> cantidad; // shift aritmético (con signo, preserva el bit de signo)

    setCC(valor, 0, 0);
    escribirOperando(operando, valor);
}


//Carga los 2 bytes menos significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2)
void ldl(){
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA);
    int valorB = leerOperando(operandoB);
    int resultado = (valorA & 0xFFFF0000) | (valorB & 0xFFFF); // conserva los 16 bits altos de A, reemplaza los 16 bits bajos con los bajos de B

    escribirOperando(operandoA, resultado);
    setCC(resultado, 0, 0); // solo chequea si es negativo o cero
}

//Carga los 2 bytes más significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2).
void ldh(){
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA);
    int valorB = leerOperando(operandoB);
    int resultado = (valorA & 0x0000FFFF) | ((valorB & 0xFFFF) << 16); // conserva los 16 bits bajos de A, reemplaza los 16 bits altos con los bajos de B

    escribirOperando(operandoA, resultado);
    setCC(resultado, 0, 0); // solo chequea si es negativo o cero
}

//Carga un numero aleatorio entre 0 y el segundo operando, al primer operando
void rnd(){
    int operandoDestino = registros[2];
    int limite = leerOperando(registros[3]); // OP2: descriptor del operando B

    srand(time(NULL));
    int valor = rand() % (limite + 1); // número aleatorio entre 0 y limite (inclusive)
    escribirOperando(operandoDestino, valor);
    setCC(valor,0,0);
}
