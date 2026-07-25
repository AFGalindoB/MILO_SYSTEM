#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Código refactorizado con resolución automática de etiquetas
    const char* codigo_prueba = 
        "; --- Salto inicial al programa principal ---\n"
        "JMP MAIN            ; Saltar sobre la función SUMAR al inicio\n"
        "\n"
        ";-------------------------\n"
        "; Subrutina: SUMAR (while R1 < R5)\n"
        ";-------------------------\n"
        "SUMAR:\n"
        "BUCLE:\n"
        "    CMP R1, R5      ; Comparar contador (R1) con el límite (R5)\n"
        "    JZ FIN_FN       ; Si son iguales (Z=1), salir al RET\n"
        "    ADD R1, R1, R0  ; R1 = R1 + R0\n"
        "    JMP BUCLE       ; Volver al inicio del bucle\n"
        "\n"
        "FIN_FN:\n"
        "    RET             ; Retornar de la subrutina (expande a 2 palabras físicas)\n"
        "\n"
        ";-------------------------\n"
        "; Programa principal\n"
        ";-------------------------\n"
        "MAIN:\n"
        "    MOVI R0, #1     ; R0 = 1 (Paso de incremento)\n"
        "    MOVI R1, #0     ; R1 = 0 (Contador inicial)\n"
        "    MOVI R5, #5     ; R5 = 5 (Límite del while)\n"
        "    CALL SUMAR      ; Invocar función SUMAR\n"
        "\n"
        "; --- Verificación Post-Retorno ---\n"
        "    CMP R1, R5      ; ¿R1 es igual a R5 después de la función?\n"
        "    JZ IGUALES      ; Si Z=1, ir a la sección IGUALES\n"
        "    MOVI R2, #2     ; Caso contrario: R2 = 2\n"
        "    JMP FIN         ; Saltar al final\n"
        "\n"
        "IGUALES:\n"
        "    MOVI R2, #1     ; Éxito: R2 = 1\n"
        "\n"
        "FIN:\n"
        "    NOP             ; Fin del programa\n";

    printf("====================================================================\n");
    printf("   🔥 TEST DE RESOLUCIÓN DE ETIQUETAS Y CONTROL DE FLUJO 🔥 \n");
    printf("====================================================================\n\n");

    // Ejecuta el pre-escaneo, la compilación y la exportación de la ROM de 64 bits
    int resultado = compilar_y_exportar("test_etiquetas.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}