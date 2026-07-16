#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    const char* codigo_prueba = 
        "MOVI R0, #5\n"
        "MOVI R1, #1\n"
        "ADD R2, R0 R1\n"
        "SUB R3, R0 R1\n"
        "AND R4, R0 R0\n"
        "OR R5, R0 R1\n"
        "XOR R6, R0 R2\n"
        "MOVI R7, #8\n"
        "MOVI R8, #4\n"
        "SHL R9, R1 R7\n"
        "SHR R10, R9 R8\n"
        "NOT R11, R1\n";

   printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE OPERACIONES ARITMÉTICO-LÓGICAS (ALU) 🔥  \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_alu.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}