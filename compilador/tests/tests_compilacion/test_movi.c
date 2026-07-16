#include <stdio.h>
#include <string.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Generar el set de instrucciones dinámico (MOVI R0, #1 ... MOVI R15, #16)
    char codigo_prueba[1024] = {0};
    char linea_buffer[64];
    
    for (int i = 0; i <= 15; i++) {
        sprintf(linea_buffer, "MOVI R%d, #%d\n", i, i + 1);
        strcat(codigo_prueba, linea_buffer);
    }

    printf("====================================================================\n");
    printf("   🔥 TEST DE COMPILACIÓN MOVI CON HELPER DE EXPORTACIÓN 🔥      \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_movi.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}