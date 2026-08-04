#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Código de Milo ASM para escribir "HOLA MUNDO" en el Tile Buffer de la GPU
    const char* codigo_prueba = 
        "; ---- Escritura de 'HOLA MUNDO' en Tile Buffer ----\n"
        "MOVI TBUF, #32768   ; H (Index: 8)  -> Dir 0  (0x8000)\n"
        "MOVI TBUF, #61441   ; O (Index: 15) -> Dir 1  (0xF001)\n"
        "MOVI TBUF, #49154   ; L (Index: 12) -> Dir 2  (0xC002)\n"
        "MOVI TBUF, #4099  ; A (Index: 1)  -> Dir 3  (0x1003)\n"
        "MOVI TBUF, #4       ;   (Index: 0)  -> Dir 4  (0x0004)\n"
        "MOVI TBUF, #53253   ; M (Index: 13) -> Dir 5  (0xD005)\n"
        "MOVI TBUF, #86022   ; U (Index: 21) -> Dir 6  (0x15006)\n"
        "MOVI TBUF, #57351   ; N (Index: 14) -> Dir 7  (0xE007)\n"
        "MOVI TBUF, #16392   ; D (Index: 4)  -> Dir 8  (0x4008)\n"
        "MOVI TBUF, #61449   ; O (Index: 15) -> Dir 9  (0xF009)\n"
        "; ---- 2. Configuración e Inicio de Scroll (ROM 10 a 11) ----\n"
        "MOVI R0, #1         ; Dir 10: R0 = 1\n"
        "MOVI R2, #127     ; Dir 11: R2 = 127\n"
        "MOVI R1, #0         ; Dir 12: R0 = 0\n"
        "; ---- 3. Bucle de Animación de Scroll (ROM 12+) ----\n"
        "WAITV                   ; Dir 13 (BUCLE): Esperar señal de vblank\n"
        "ADD SCROLL, R1, R1, R0  ; Dir 14 (BUCLE): R1 = R1 + R0 y actualiza SCROLL GPU\n"
        "CMP R1, R2              ; Dir 15 (BUCLE): Revisar si se llego a 127 (Recorrio todo x)\n"
        "JZ #12                  ; Dir 16: Salta de regreso al reinicio del contador\n"
        "JMP #13                 ; Dir 17: Salta de regreso a la instrucción WAITV en dir 13\n";


    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN GPU - HOLA MUNDO CON SCROLL 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_gpu.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}