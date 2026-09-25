#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

void mostrarOperando(unsigned int dato, int nroByte);
void pasoDisassembler(unsigned int * dirInstr);
void mostrarAssembler(unsigned int tamCod);

extern char mnemonicos[32][5];
extern char reg[32][5];

#endif