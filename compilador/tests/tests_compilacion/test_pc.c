#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Código estructurado con saltos calculados a mano tras la expansión del RET
    const char* codigo_prueba = 
            "; --- Salto inicial al programa principal ---\n"
    /*0*/   "JMP #7              ; [PC: 0] Saltar sobre la funcion SUMAR directamente a MAIN (PC: 7)\n"
            "\n"
            "; --- SUBRUTINA: SUMAR (while R1 < R5) ---\n"
    /*1*/   "CMP R1, R5          ; [PC: 1] BUCLE: Comparar contador (R1) con el limite (R5)\n"
    /*2*/   "JZ #5               ; [PC: 2] Si son iguales (Z=1), salir al RET (PC: 5)\n"
    /*3*/   "ADD R1, R1, R0      ; [PC: 3] R1 = R1 + R0 (R0 tiene #1)\n"
    /*4*/   "JMP #1              ; [PC: 4] Volver al inicio del bucle\n"
    /*5*/   "RET                 ; [PC: 5-6] Retornar de la subrutina (Paso 1: dec SP [PC:7], Paso 2: ld PC [PC:8])\n"
            "\n"
            "; --- PROGRAMA PRINCIPAL (MAIN) ---\n"
    /*7*/   "MOVI R0, #1         ; [PC: 7]  R0 = 1 (Paso de incremento)\n"
    /*8*/   "MOVI R1, #0         ; [PC: 8] R1 = 0 (Contador inicial)\n"
    /*9*/   "MOVI R5, #5         ; [PC: 9] R5 = 5 (Limite del while)\n"
    /*A*/   "CALL #1             ; [PC: 10] Invocar funcion SUMAR en PC: 1 (Guarda PC+1 [13] en stack)\n"
            "\n"
            "; --- Verificacion Post-Retorno ---\n"
    /*B*/   "CMP R1, R5          ; [PC: 11] ¿R1 es igual a R5 despues de la funcion?\n"
    /*C*/   "JZ #15              ; [PC: 12] Si Z=1, ir a la seccion IGUALES (PC: 15)\n"
    /*D*/   "MOVI R2, #2         ; [PC: 13] Caso contrario: R2 = 2\n"
    /*E*/   "JMP #16             ; [PC: 14] Saltar al final (PC: 16)\n"
            "\n"
            "; --- Bloque de Exito ---\n"
    /*F*/   "MOVI R2, #1         ; [PC: 15] IGUALES: R2 = 1\n"
    /*10*/  "NOP                 ; [PC: 16]\n";

    printf("====================================================================\n");
    printf("   🔥 TEST AVANZADO: SUBRUTINAS, CICLO WHILE Y CONTROL DE FLUJO 🔥 \n");
    printf("====================================================================\n\n");

    // Ejecuta el pipeline completo y exporta la ROM binaria de 64 bits a un archivo de texto
    int resultado = compilar_y_exportar("test_pc.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}