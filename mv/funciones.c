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
void imprimirHexa(unsigned int dato, int cantBytes);
void mostrarMemoria(unsigned int inicio, unsigned int cantidad);
unsigned char devolverByte(unsigned int dato, int nroByte);
int leerOperando(int operando, int cantBytes);
void escribirOperando(int operando, int valor, int cantBytes);
void leerDeMemoria();
void leerDeMemoriaReg();
void escribirEnMemoria();
void escribirMBR(int valor);
void calcularPunteroLogico(int operandoMemoria);
void calcularPunteroFisico(int cantidadDeBytes);
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

//  FUNCIONES AUXILIARES

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

// el dato lo "casteo" a unsigned int para hacer el shift lógico
unsigned char devolverByte(unsigned int dato, int nroByte){ // que nombre le ponemos a los parámetros
    // el nro byte es el byte a devolver, siendo 0 el byte de menor significado, 1: el segundo de menor significado, ....
//    printf("dato : %08X\n",dato);
    dato = dato >> (8 * nroByte);
    dato &= 0xFF;
//    printf("byte a devolver: %08X\n",dato);
    unsigned char aux;
    aux = (unsigned char) dato;
    return aux;
}


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
//    printf("\ntipo: %d \n",tipo);
    int datosOperando = operando & 0xFFFFFF;
    int aux;
    switch (tipo){
        case 1: // registro
//            printf("\nval: %d \n",valor );
//            imprimirHexa(valor,4);
//            aux = leerOperando(valor,4);        
//            printf("\naux: %d \n",aux);
//            imprimirHexa(aux,4);
            registros[datosOperando & 0x1F] = valor;
            break;

        case 3: // memoria
//            printf("\nA..\n");
//            aux = leerOperando(valor,4);
//            printf("B..\n");
            calcularPunteroLogico(datosOperando);
//            printf("C..\n");            
            calcularPunteroFisico(cantBytes);
            //printf("\n\ninmediato:%d\n\n",aux);
//            printf("D..\n");
            escribirMBR(valor);
//            printf("E..\n");
            escribirEnMemoria();
//            printf("FFFF..\n");
            break;

        default: // ninguno o inmediato (no son destino válido)
            break;
    }
}

void leerDeMemoria(){
    int n; // cant bytes a leer
    //n = devolverByte(registros[5],3);
    //n = n << 8;
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;

//       printf("posFisica: %d\n",posFisica);
    unsigned int aux = 0;

    //    printf("%d\n",n);
    for(int i=0; i < n; i++){
        aux = aux << 8;
//        printf("hola\n");
        aux = aux | memoria[posFisica + i];
//        printf("\n%02X\n",memoria[posFisica + i]);
//        printf("chau\n");
    }
    escribirMBR(aux);
}

void leerDeMemoriaReg(){
    int n;
    n = devolverByte(registros[5],2);
    int posFisica = registros[5] & 0xFFFF;
    unsigned int aux = 0;
    for (int i = 0; i < n; i++){
        aux = aux << 8;
        aux = aux | memoria[posFisica + i];
    }
    aux &= 0xFFFFFF1F;
    escribirMBR(aux);
}


void escribirEnMemoria(){
    int mbr;
    int n; // cant bytes a escribir
    mbr = registros[6];
    //n = devolverByte(registros[5],3);
    //n = n << 8;
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
    // lar -> reg[4]
    int nroReg = (devolverByte(operandoMemoria,0) & 0x1F );
    int puntLog = registros[nroReg];
    int aux = devolverByte(operandoMemoria,2);
    aux = aux << 8; //LPM, había puesto << 1 quehdp
    aux = aux | devolverByte(operandoMemoria,1);
//    printf("\n%04X\n",aux);
    puntLog += aux; // acá habría que verificar si no nos caímos del segmento   
    registros[4] = puntLog; // reg[4] -> LAR
}

void calcularPunteroFisico(int cantidadDeBytes){ // configura el MAR
//    printf("lar= %08X\n",registros[4]);
    int puntLogico = registros[4]; // lar
    int nroSegm = devolverByte(puntLogico,2);

//    printf("nroSegm = %d\n",nroSegm);
    int puntFisico = tablaSegm[nroSegm].base; // dirección base

//    printf("tablasegm0.base = %d\n",puntFisico);
    int aux = devolverByte(puntLogico,1);
    aux = aux << 8;
    aux = aux | devolverByte(puntLogico,0);

//    printf("byte 0 y 1 de lar = %04X \n",aux);
    puntFisico += aux; // + offset

//    printf("puntFis = %d\n",puntFisico);
    // acá habría que verificar si no nos caímos del segmento
    cantidadDeBytes = cantidadDeBytes << 16;
    puntFisico |= cantidadDeBytes; // cargamos en la parte alta del MAR la cant de bytes a leer/escribir

//    printf("punteroFis queda conformado: = %08X\n",puntFisico);
    registros[5] = puntFisico; // reg[5] -> MAR
}

// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("flaco sos un pelotudo");    // mira vos
}


//--------------   SIN OPERANDOS  --------------------

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
    printf("llegaste a un stop \n\n");
}

//--------------    UN OPERANDO   --------------------

void sysRead(){
    int direccionLogica = registros[13]; // EDX
    int modo = registros[10];            // EAX
    int cantidad = registros[12] & 0xFFFF;       // ECX (2 bytes bajos): cantidad de valores
    int tamano = (registros[12] >> 16) & 0xFFFF; // ECX (2 bytes altos): tamaño de cada valor

    for (int i = 0; i < cantidad; i++){
        registros[4] = direccionLogica; // config lar
        calcularPunteroFisico(cantidad); // config mar
        int dirFisica = registros[5] && 0xFFFF;

        int valor = 0;
        unsigned char texto[33]; // solo para el binario: hasta 32 bits + '\0'

//        printf("[%04X]: ", dirFisica);
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

    printf("\n\n\n\n\n felicidades, llegó a un sys write \n\n\n\n\n");
    printf("cant valores: %d\n",cantidad);
    printf("tamaño: %d \n",tamano);
    printf("modo: ");
    imprimirHexa(modo,4);
    printf("\ndirLogica: ");
    imprimirHexa(direccionLogica,4);

    for (int i = 0; i < cantidad; i++){
//        printf("hiii\n");
        registros[4] = direccionLogica; // config lar
        calcularPunteroFisico(cantidad); // config mar
        int dirFisica = registros[5] & 0xFFFF;

        int valor = 0;
        for (int b = 0; b < tamano; b++) // arma el valor con los bytes de memoria (el primero es el mas significativo)
            valor = (valor << 8) | memoria[dirFisica + b];
        printf("\n\nvalor: %08X\n",valor);
        escribirMBR(valor);

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
//            printf("halloooo\n\n");
            printf(" ");
//            for (int b = 0; b < tamano; b++){
//                unsigned char c = devolverByte(valor, b);
//            if ((c >= 32) && (c <= 126))
            if (valor >= 32 && valor <= 126)
                printf("%c", valor);
            else
                printf("."); // no imprimible
            //}
        }
        if (modo & 0x01) 
          printf(" %d", valor); // decimal

        printf("\n");
        direccionLogica += tamano;  // avanza TA BIEN
    }
//    printf("\n\n\n\n\n\n\n\n\n\n");
}

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
    int aux = leerOperando(registros[2],4);
    aux = ~aux;
    escribirOperando(registros[2], aux, 4);
    setCC(aux,0,0); // solo chequea si es negativo o cero    
}


//--------------   DOS OPERANDOS  --------------------

void setCC(int resultado, int hayCarry, int hayOverflow){    //  Modifica CC (registro 17)
    int aux = 0;
    if(resultado == 0)
        aux = 0b0100;                   //  Z: Cero
    else
        if(resultado < 0)
            aux = 0b1000;               //  N: Negativo

    aux |= 0b0010 * hayCarry;
    aux |= 0b0010 * hayOverflow;

    aux = aux << 28;    //  NZCV << 28  =  0b NZCV0000 00000000 00000000 00000000
    registros[17] = aux; //  Guarda AUX en REGISTRO CC
}


void mov(){
    printf("felicidades, estas en un mov \n\n");
//    printf("\nA..\n");
    int valor = leerOperando(registros[3],4);    //  Guarda valor de OP2 en valor
    printf("valor: ");
    imprimirHexa(valor,4);
//    valor |= 0x02000000; // esto para que escribirOperando lo pueda "leer" como un inmediato

    setCC(valor, 0, 0); // analizo el valor (si es negativo o 0)
//    printf("C..\n");
//    printf("op1: %8X \nvalor:%8X \n",registros[2],valor);
    escribirOperando(registros[2], valor,4); //  Escribe valor en lo que dicta OP1;
//    printf("D..\n");
}

void add(){
    int valor1 = leerOperando(registros[2],4);//  Guarda valor de OP1 en valor1
    int valor2 = leerOperando(registros[3],4);//  Guarda valor de OP2 en valor2
    int suma = valor1 + valor2;

    int carry;
    int overflow;
    setCC(suma,carry, overflow);
    escribirOperando(registros[2], suma, 4); //  Escribe 'suma' en OP1;
}

void sub(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int resta = valor1 - valor2;

    int carry;
    int overflow;

    setCC(resta, carry, overflow);
    escribirOperando(registros[2], resta,4); //  Escribe 'resta' en OP1;
}

void mul(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int producto = valor1 * valor2;             //  Guarda multiplicacion en 'producto'

    //setCC(producto, carry, overflow); // esto lo comento temporalmente para prueba de compilación
    escribirOperando(registros[2], producto,4);   //  Escribe 'producto' en OP1;
}

void divis(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int cociente;
    int resto;
    //  !!!
    if (valor2 != 0){                 //  Verifica si el contenido de OP2 es 0
        cociente = valor1 / valor2;   //  Realiza DIVISION ENTERA
        resto = valor1 % valor2;      //  Guarda el RESTO

        int carry;
        int overflow;
        setCC(cociente, carry, overflow);
        escribirOperando(registros[2], cociente,4);   //  Guarda DIVISION ENTERA en OP1
        escribirOperando(registros[16], resto,4);     //  Guarda RESTO en AC
    }
    else
        //  Si el valor del operando 2 ES CERO,
        //  DETIENE EL PROGRAMA POR COMPLETO
        stop();
}

void cmp(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int diferencia = valor1 - valor2;       //  Realiza DIFERENCIA entre valor1 y valor2

    int carry;
    int overflow;
    setCC(diferencia, carry, overflow);      //  Modifica CC con respecto a la diferencia anterior
}

void and(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int ans = valor1 & valor2;              //  Guarda AND LOGICO en 'ans'

    setCC(ans,0,0);
    escribirOperando(registros[2], ans, 4);    //  Escribe 'ans' en OP1;
}

void or(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int ans = valor1 | valor2;              //  Guarda OR LOGICO en 'ans'

    setCC(ans,0,0);
    escribirOperando(registros[2], ans, 4);    //  Escribe 'ans' en OP1;
}

void xor(){
    int valor1 = leerOperando(registros[2],4);
    int valor2 = leerOperando(registros[3],4);
    int ans = valor1 ^ valor2;              //  Guarda XOR LOGICO en 'ans'

    setCC(ans,0,0);
    escribirOperando(registros[2], ans, 4);    //  Escribe 'ans' en OP1;
}

void swap(){
    int a = leerOperando(registros[2],4); // valor de A
    int b = leerOperando(registros[3],4); // valor de B

    escribirOperando(registros[2], b,4);  // A = B
    escribirOperando(registros[3], a,4);  // B = A
    setCC(b, 0, 0); // solo chequea si es negativo o cero      
}

void shl(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando,4); // valor real de A
    int cantidad = leerOperando(registros[3],4); // op2

    valor = valor << cantidad;

    int carry;
    int overflow;
    escribirOperando(operando, valor, 4);
    setCC(valor, carry, overflow);
}

void shr(){
    int operando = registros[2]; // op1
    int valor = leerOperando(operando,4); // valor real de A (no el descriptor)
    int cantidad = leerOperando(registros[3],4); // op2

    valor = valor >> cantidad;

    int overflow;
    escribirOperando(operando, valor, 4);
    setCC(valor, 0, overflow);
}

void sar(){
    int operando = registros[2]; // op1
    unsigned int valor = leerOperando(operando,4); // valor real de A
    int cantidad = leerOperando(registros[3],4); // op2

    valor = valor >> cantidad; // shift aritmético (con signo, preserva el bit de signo)

    escribirOperando(operando, valor,4);
    setCC(valor, 0, 0);
}


//Carga los 2 bytes menos significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2)
void ldl(){
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA,4);
    int valorB = leerOperando(operandoB,4);
    int resultado = (valorA & 0xFFFF0000) | (valorB & 0xFFFF); // conserva los 16 bits altos de A, reemplaza los 16 bits bajos con los bajos de B

    escribirOperando(operandoA, resultado,4);
    setCC(resultado, 0, 0); // solo chequea si es negativo o cero    
}

//Carga los 2 bytes más significativos del primer operando (OP1), con los 2 bytes menos significativos del segundo operando (OP2).
void ldh(){
    int operandoA = registros[2]; // OP1: destino
    int operandoB = registros[3]; // OP2: fuente
    int valorA = leerOperando(operandoA,4);
    int valorB = leerOperando(operandoB,4);
    int resultado = (valorA & 0x0000FFFF) | ((valorB & 0xFFFF) << 16); // conserva los 16 bits bajos de A, reemplaza los 16 bits altos con los bajos de B

    escribirOperando(operandoA, resultado,4);
    setCC(resultado, 0, 0); // solo chequea si es negativo o cero
}

//Carga un numero aleatorio entre 0 y el segundo operando, al primer operando
void rnd(){
    int operandoDestino = registros[2]; // OP1: descriptor del operando A (destino)
    int limite = leerOperando(registros[3],4); // OP2: descriptor del operando B

    srand(time(NULL));
    int valor = rand() % (limite + 1); // número aleatorio entre 0 y limite (inclusive)
    escribirOperando(operandoDestino, valor,4);
    setCC(valor,0,0);
}
