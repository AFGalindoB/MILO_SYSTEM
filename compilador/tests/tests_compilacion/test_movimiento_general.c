#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    const char* codigo_prueba = 
        "MOVI R0, #5        ; Comprueba R0 = 5\n"
        "MOV R1, R0         ; Copia para verificar MOV\n"
        "; ---------- STORE desde registro -----------\n"
        "STORE R0, [R0]     ; RAM[5] = 5\n"
        "LOAD R5, [R0]      ; R5 = 5\n"
        "; ---------- STORE desde inmediato ----------\n"
        "MOVI R2, #1        ; R2 = 1\n"
        "STORE #3, [R2]     ; RAM[1] = 3\n"
        "LOAD R3, [R2]      ; R3 = 3\n"
        "; ---------- STORE desde entrada ----------\n"
        "BUCLE:             ; Etiqueta\n"
        "STORE RINPT, [R3]  ; RAM[3] = RINPT\n"
        "JMP BUCLE          ; Devolverse a escribir la entrada del sistema\n";

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE MOVIMIENTOS GENERALES (RAM/REGISTROS) 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_movimiento_general.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}