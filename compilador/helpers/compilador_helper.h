#ifndef COMPILADOR_HELPER_H
#define COMPILADOR_HELPER_H

/**
 * @brief Compila una cadena de texto con código fuente Milo ASM, la codifica a microcódigo
 * de 64 bits y exporta el archivo ROM resultante en formato de texto.
 * * @param nombre_archivo Nombre del archivo TXT de salida (ej. "test_alu.txt").
 * @param codigo_fuente Cadena de texto que contiene las instrucciones en ensamblador.
 * @return int Retorna 0 si la compilación fue exitosa, o 1 si hubo errores sintácticos.
 */
int compilar_y_exportar(const char* nombre_archivo, const char* codigo_fuente);

#endif // COMPILADOR_HELPER_H