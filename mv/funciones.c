#include "registros.c"
#include "memoria.c"
#include "tablaSeg.c"

//void devolverOperandos(int tipo1, int tipo2, int * op1,int * op2){

//  implementamos o no?

//}

// 1 función por mnemónico (28 mnémonicos + 5 sin definir (notDefined))

//--------------    NO DEFINIDO  ---------------------

void notDefined(){
    printf("flaco sos un pelotudo");
}

void movXD(){}   //   Santi hace desde MOV

// mauro:
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
}

void stop(){
    registros[0] = -1; // ip = 0xFFFFFFFF
}
//--------------   DOS OPERANDOS  --------------------
//  SANTI
void mov()
void add()
void sub()
void mul()
void div()
void cmp()
void and()
void or()
void xor()

//  NICO
void swap()
void shl()
void shr()
void sar()
void ldl()
void ldh()
void rnd()

//--------------    UN OPERANDO   --------------------
void sys()

//  funciones de mauro

//--------------   SIN OPERANDOS  --------------------

//  STOP de mauro
