# mv
Proyecto de Maquina Virtual que interpreta y ejecuta código máquina traducido desde Assembly.

# como usar
En la carpeta "ejecutable" se encuentran los binarios listos para ejecutar (el .exe es para windows)

También se pueden compilar desde el código fuente.
- Desde la carpeta "mv" (código fuente):

- Windows:
- gcc arranque.c procesador.c mnemonicos.c puntFunc.c funcionesAuxiliares.c estructuras.c disassembler.c -o vmx.exe

- Linux:
- gcc arranque.c procesador.c mnemonicos.c puntFunc.c funcionesAuxiliares.c estructuras.c disassembler.c -o vmx