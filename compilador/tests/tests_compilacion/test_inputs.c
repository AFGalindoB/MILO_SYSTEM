#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    const char* codigo_prueba = 
        "; ==========================================================\n"
        "; ---- BUCLE PRINCIPAL DE LECTURA DE CONTROLES (RINPT) -----\n"
        "; ==========================================================\n"
        "\n"
        "; --- 1. Lectura del Registro de Entrada ---\n"
        "MOV R0, RINPT           ; Dir 0: Copia el estado de los botones a R0\n"
        "MOVI R1, #0             ; Dir 1: Default = Espacio en blanco (Índice 0, Dir 0)\n"
        "\n"
        "; --- 2. Evaluación de Botón A (Bit 0 = 0x01) ---\n"
        "MOVI R2, #1             ; Dir 2: Máscara Bit 0 (A)\n"
        "AND.F R3, R0, R2        ; Dir 3: Test Bit 0\n"
        "JZ #7                   ; Dir 4: Si no está presionado, saltar a evaluar B\n"
        "MOVI R1, #256           ; Dir 5: Tile 'A' (Índice 1 << 8 = 256) en Dir 0\n"
        "JMP #41                 ; Dir 6: Ir a dibujar\n"
        "\n"
        "; --- 3. Evaluación de Botón B (Bit 1 = 0x02) ---\n"
        "MOVI R2, #2             ; Dir 7: Máscara Bit 1 (B)\n"
        "AND.F R3, R0, R2        ; Dir 8: Test Bit 1\n"
        "JZ #12                  ; Dir 9: Si no está presionado, saltar a evaluar X\n"
        "MOVI R1, #512           ; Dir 10: Tile 'B' (Índice 2 << 8 = 512) en Dir 0\n"
        "JMP #41                 ; Dir 11: Ir a dibujar\n"
        "\n"
        "; --- 4. Evaluación de Botón X (Bit 2 = 0x04) ---\n"
        "MOVI R2, #4             ; Dir 12: Máscara Bit 2 (X)\n"
        "AND.F R3, R0, R2        ; Dir 13: Test Bit 2\n"
        "JZ #17                  ; Dir 14: Si no está presionado, saltar a evaluar Y\n"
        "MOVI R1, #6144          ; Dir 15: Tile 'X' (Índice 24 << 8 = 6144) en Dir 0\n"
        "JMP #41                 ; Dir 16: Ir a dibujar\n"
        "\n"
        "; --- 5. Evaluación de Botón Y (Bit 3 = 0x08) ---\n"
        "MOVI R2, #8             ; Dir 17: Máscara Bit 3 (Y)\n"
        "AND.F R3, R0, R2        ; Dir 18: Test Bit 3\n"
        "JZ #22                  ; Dir 19: Si no está presionado, saltar a evaluar UP\n"
        "MOVI R1, #6400          ; Dir 20: Tile 'Y' (Índice 25 << 8 = 6400) en Dir 0\n"
        "JMP #41                 ; Dir 21: Ir a dibujar\n"
        "\n"
        "; --- 6. Evaluación de Botón UP (Bit 4 = 0x10) ---\n"
        "MOVI R2, #16            ; Dir 22: Máscara Bit 4 (UP)\n"
        "AND.F R3, R0, R2        ; Dir 23: Test Bit 4\n"
        "JZ #27                  ; Dir 24: Si no está presionado, saltar a evaluar LEFT\n"
        "MOVI R1, #5376          ; Dir 25: Tile 'U' (Índice 21 << 8 = 5376) en Dir 0\n"
        "JMP #41                 ; Dir 26: Ir a dibujar\n"
        "\n"
        "; --- 7. Evaluación de Botón LEFT (Bit 5 = 0x20) ---\n"
        "MOVI R2, #32            ; Dir 27: Máscara Bit 5 (LEFT)\n"
        "AND.F R3, R0, R2        ; Dir 28: Test Bit 5\n"
        "JZ #32                  ; Dir 29: Si no está presionado, saltar a evaluar RIGHT\n"
        "MOVI R1, #3072          ; Dir 30: Tile 'L' (Índice 12 << 8 = 3072) en Dir 0\n"
        "JMP #41                 ; Dir 31: Ir a dibujar\n"
        "\n"
        "; --- 8. Evaluación de Botón RIGHT (Bit 6 = 0x40) ---\n"
        "MOVI R2, #64            ; Dir 32: Máscara Bit 6 (RIGHT)\n"
        "AND.F R3, R0, R2        ; Dir 33: Test Bit 6\n"
        "JZ #37                  ; Dir 34: Si no está presionado, saltar a evaluar DOWN\n"
        "MOVI R1, #4608          ; Dir 35: Tile 'R' (Índice 18 << 8 = 4608) en Dir 0\n"
        "JMP #41                 ; Dir 36: Ir a dibujar\n"
        "\n"
        "; --- 9. Evaluación de Botón DOWN (Bit 7 = 0x80) ---\n"
        "MOVI R2, #128         ; Dir 37: Máscara Bit 7 (DOWN)\n"
        "AND.F R3, R0, R2        ; Dir 38: Test Bit 7\n"
        "JZ #41                  ; Dir 39: Si no está presionado, ir directo a sincronizar\n"
        "MOVI R1, #1024        ; Dir 40: Tile 'D' (Índice 4 << 8 = 1024) en Dir 0\n"
        "\n"
        "; --- 10. Sincronización con GPU y Presentación ---\n"
        "WAITV                   ; Dir 41: Esperar el periodo de VBLANK\n"
        "MOV TBUF, R1            ; Dir 42: Escribir la palabra preparada en el Tile Buffer\n"
        "JMP #0                  ; Dir 43: Volver al inicio a leer el control\n";


    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN INPUTS 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_input.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}