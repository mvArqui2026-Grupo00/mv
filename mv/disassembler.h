#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

void mostrarOperando(unsigned int dato, int nroByte);
void pasoCodeDisassembler(unsigned int * dirInstr);
void pasoConstDisassembler(unsigned int * dirInstr);
void mostrarAssembler(int version,int offsetEntryPoint);

extern char mnemonicos[32][5];
extern char reg[32][5];
extern char regWord[6][3];
extern char regLow[6][3];
extern char regHigh[6][3];
#endif