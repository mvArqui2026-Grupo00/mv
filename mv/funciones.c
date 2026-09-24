#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//  CONSTANTES: LIMITES INT
#define INT_MAX 0x7FFFFFFF
#define INT_MIN 0x80000000

// declaración de funciones

// funciones para testing
void imprimirHexa(unsigned int dato, int cantBytes);
void mostrarMemoria(unsigned int inicio, unsigned int cantidad);
void mostrarRegistros();
void mostrarTablaSegm();

// funciones auxiliares
void comprobarFalloSegmento();
unsigned char devolverByte(unsigned int dato, int nroByte);

int leerOperando(int operando);
void escribirOperando(int operando, int valor);

void leerDeMemoria();
void escribirEnMemoria();
void escribirMBR(int valor);

void calcularPunteroLogico(int operandoMemoria);
void calcularPunteroFisico(int cantidadDeBytes);

// mnemónicos y funciones auxiliares de estos
void notDefined();
void stop();
void sysRead();
void sysWrite();
void sys();
int negative();
int cero();
int carry();
int overflow();
void jmp();
void jp();
void jneg(); // jn es una función built-in (del lenguaje)
void jz();
void jc();
void jv();
void jnp();
void jnn();
void jnz();
void not();
void setCC(int resultado, int carry, int overflow);
void mov();
void verificarCarryOverflowSuma(int valor1, int valor2, int suma, int * carry, int * overflow);
void add();
void sub();
void mul();
void divis(); // div es una función de stdlib
void cmp();
void and();
void or();
void xor();
void swap();
void shl();
void shr();
void sar();
void ldl();
void ldh();
void rnd();

// FUNCIONES PARA TESTING

void imprimirHexa(unsigned int dato, int cantBytes){
    printf("0x");
    unsigned int aux;
    for (int i = (cantBytes-1); i >= 0; i--){
        aux = dato >> (i*8);
        aux &= 0xFF;
        printf("%02X ",aux & 0xFF);
    }
}

void mostrarMemoria(unsigned int inicio, unsigned int cantidad){
    printf("\nMemoria de celda [%d] a celda [%d](inclusive)\n",inicio,inicio+cantidad-1);
    for (int i=0; i<cantidad;i++){
        printf("Memoria[%d]: %02X\n",i+inicio,memoria[i+inicio]);
    }
    printf("\n");
}

void mostrarRegistros(){
    printf("Estado actual de los registros \n");
    for (int i = 0; i < CANT_REGISTROS; i++){
        printf("Registro[%d]: ",i);
        imprimirHexa(registros[i],4);
        printf("\n");
    }
    printf("\n");
}

void mostrarTablaSegm(){
    printf("Tabla de Segmentos\n");
    for (int i = 0; i < CANT_SEGM; i++){
        printf("Segmento[%d]: ",i);
        printf("Base: ");
        imprimirHexa(tablaSegm[i].base,2);
        printf("Tamaño: ");
        imprimirHexa(tablaSegm[i].tamaño,2);
        printf("\n");
    }
    printf("\n");
}

//  FUNCIONES AUXILIARES

unsigned char devolverByte(unsigned int dato, int nroByte){ // que nombre le ponemos a los parámetros
    // el nro byte es el byte a devolver, siendo 0 el byte de menor significado, 1: el segundo de menor significado, ....
    dato = dato >> (8 * nroByte);
    dato &= 0xFF;
    unsigned char aux = dato;
    return aux;
}

void comprobarFalloSegmento(){
    int lar = registros[4];
    int posSegm = devolverByte(lar,2);
    int offset = lar & 0xFFFF;

    if (posSegm < 0 || posSegm >= CANT_SEGM){
        printf("\nError: Fallo de Segmento \n");
        exit(1);
    }
    if (offset >= tablaSegm[posSegm].tamaño){
        printf("\nError: Fallo de Segmento \n");
        exit(1);
    }
        
}


int leerOperando(int operando){
    int tipo = (devolverByte(operando,3));
    int datosOperando = operando & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            return registros[datosOperando & 0x1F];

        case 2: // inmediato
            return (datosOperando & 0xFFFF);

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(4);
            leerDeMemoria();
            return registros[6];
        default: // ninguno
            return 0; // está bien esto??
    }
}

void escribirOperando(int operando, int valor){
    int tipo = devolverByte(operando,3);
    int datosOperando = operando & 0xFFFFFF;

    switch (tipo){
        case 1: // registro
            registros[datosOperando & 0x1F] = valor;
            break;

        case 3: // memoria
            calcularPunteroLogico(datosOperando);
            calcularPunteroFisico(4);
            escribirMBR(valor);
            escribirEnMemoria();
            break;

        default: // ninguno o inmediato (no son destino válido)
            break;
    }
}

void leerDeMemoria(){
    int n; // cant bytes a leer
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;
    unsigned int aux = 0;

    for(int i=0; i < n; i++){
        aux = aux << 8;
        aux = aux | memoria[posFisica + i];
    }
    escribirMBR(aux);
}

void escribirEnMemoria(){
    int mbr;
    int n; // cant bytes a escribir
    mbr = registros[6];
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;

    int aux = 0; // por las dudas que el n sea 0, hay que verificarlo en el testing
    for(int i=0; i < n; i++){
        memoria[posFisica+i] = devolverByte(mbr,n-1 -i);
    }
}

void escribirMBR(int valor){ // configura el MBR
    registros[6] = valor;
}

void calcularPunteroLogico(int operandoMemoria){ // configura el LAR
    int nroReg = (devolverByte(operandoMemoria,0) & 0x1F );
    int puntLog = registros[nroReg];
    int aux = devolverByte(operandoMemoria,2);
    aux = aux << 8;
    aux = aux | devolverByte(operandoMemoria,1);
    puntLog += aux;
    registros[4] = puntLog;
    comprobarFalloSegmento();
}

void calcularPunteroFisico(int cantidadDeBytes){ // configura el MAR
    int puntLogico = registros[4];
    int nroSegm = devolverByte(puntLogico,2);
    int puntFisico = tablaSegm[nroSegm].base; // dirección base

    int aux = devolverByte(puntLogico,1);
    aux = aux << 8;
    aux = aux | devolverByte(puntLogico,0);

    puntFisico += aux; // + offset

    cantidadDeBytes = cantidadDeBytes << 16;
    puntFisico |= cantidadDeBytes; // cargamos en la parte alta del MAR la cant de bytes a leer/escribir

    registros[5] = puntFisico; // reg[5] -> MAR
}

// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("Error: Mnemónico no Definido");
    exit(1);
}


//--------------   SIN OPERANDOS  --------------------

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
}

//--------------    UN OPERANDO   --------------------

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
        dirFisica = registros[5] && 0xFFFF;

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
    registros[0] &= 0xFFFF0000;
    registros[0] += leerOperando(registros[2]);
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
        jmp;
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
    long int longSuma = (long int)valor1 + valor2;
    if ((longSuma >> 32) != 0)
        *carry = 1;

    if ((valor1 > 0 && valor2 > 0) && suma <= 0)
        *overflow = 1;
    if ((valor1 < 0 && valor2 < 0) && suma >= 0)
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
    unsigned long int longProducto = (unsigned long int)abs(valor1) * (unsigned long int)abs(valor2);
    if ((longProducto >> 32) != 0)
        carry = overflow = 1;
    setCC(producto, carry, overflow);
    escribirOperando(registros[2], producto);   //  Escribe 'producto' en OP1;
}

void divis(){
    int valor1 = leerOperando(registros[2]);
    int valor2 = leerOperando(registros[3]);
    int cociente;
    int resto;
    //  !!!
    if (valor2 != 0){
        cociente = valor1 / valor2;   //  Realiza DIVISION ENTERA
        resto = valor1 % valor2;      //  Guarda el RESTO

        setCC(cociente, 0, 0); // en la división no hay ni carry ni overflow
        escribirOperando(registros[2], cociente);   //  Guarda DIVISION ENTERA en OP1
        registros[16] = resto;     //  Guarda RESTO en AC
    }
    else{
        printf("Error: División por cero\n");
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
    xor();
    xor();
    xor();
}

void shl(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando); // valor real de A
    int cantidad = leerOperando(registros[3]); // op2

    int resultado = valor << cantidad;

    int carry = 0;
    int overflow = 0;

    long int longResultado = (unsigned long int) valor << cantidad;
    if ((longResultado & 0xFFFF0000) != 0)
        carry = 1;

    if (valor > 0 && resultado < 0)
        overflow = 1;
    if (valor < 0 && resultado > 0)
        overflow = 1;

    setCC(resultado, carry, overflow);

    escribirOperando(operando, valor);
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
