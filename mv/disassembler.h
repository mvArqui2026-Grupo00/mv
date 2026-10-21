#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

void mostrarOperando(unsigned int dato, int nroByte);
void pasoCodeDisassembler(unsigned int * dirInstr);
void pasoConstDisassembler(unsigned int * dirInstr);
void mostrarAssembler(int version,int offsetEntryPoint);

extern char mnemonicos[32][5];
extern char reg[32][5];

#endif