#ifndef MNEMONICOS_H
#define MNEMONICOS_H

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

#endif