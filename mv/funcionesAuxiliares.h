#ifndef FUNCIONES_AUXILIARES_H
#define FUNCIONES_AUXILIARES_H

// funciones para testing
void imprimirHexa(unsigned int dato, int cantBytes);
void testOperandos(int opc, int op1, int op2);
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

#endif