#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    const char* codigo_prueba = 
        "MOVI R0, #5\n"
        "MOV R1, R0\n"
        "STORE R0, [R0]\n"
        "LOAD R5, [R0]\n";

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE MOVIMIENTOS GENERALES (RAM/REGISTROS) 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_movimiento_general.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}