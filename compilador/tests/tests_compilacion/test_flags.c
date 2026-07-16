#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Código de Milo ASM diseñado para activar secuencialmente todas las flags físicas
    const char* codigo_prueba = 
        "; --- 1. PROBAR CARRY (C) --- \n"
        "MOVI R0, #4294967295 ; Carga 0xFFFFFFFF\n"
        "MOVI R1, #1          ; Carga 1\n"
        "ADD.F R2, R0 R1     ; 0xFFFFFFFF + 1 = 0x00000000 (Genera Carry = 1)\n"
        "\n"
        "; --- 2. PROBAR PROPAGACIÓN DE CARRY (ADC) ---\n"
        "ADC R4, R3 R3        ; 0 + 0 + Carry(1) = 1\n"
        "\n"
        "; --- 3. PROBAR CERO (Z) ---\n"
        "MOVI R5, #5\n"
        "SUB.F R6, R5 R5      ; 5 - 5 = 0 (Genera Zero = 1)\n"
        "\n"
        "; --- 4. PROBAR NEGATIVO (N) ---\n"
        "MOVI R7, #2\n"
        "SUB.F R8, R7 R5     ; 2 - 5 = -3 (0xFFFFFFFD -> MSB = 1 -> Genera Negative = 1)\n"
        "\n"
        "; --- 5. PROBAR RESTA CON ACARREO/PRÉSTAMO (SBC) ---\n"
        "SBC R9, R3 R3       ; Resta usando el estado del acarreo actual\n"
        "\n"
        "; --- 6. PROBAR OVERFLOW (V) ---\n"
        "MOVI R10, #2147483647; Carga el máximo entero positivo firmado (0x7FFFFFFF)\n"
        "ADD.F R11, R10 R1    ; 0x7FFFFFFF + 1 = 0x80000000 (Positivo + Positivo = Negativo -> Genera Overflow = 1)\n";

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN Y ANÁLISIS DE CONTROL DE FLAGS (C,Z,N,V) 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_flags.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}