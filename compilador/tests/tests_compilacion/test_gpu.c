#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    // Código de Milo ASM para escribir "HOLA MUNDO" en el Tile Buffer de la GPU
    const char* codigo_prueba = 
        "; ---- Escritura de 'HOLA MUNDO' en Tile Buffer ----\n"
        "MOVI TBUF, #2048   ; H (Index: 8)  -> Dir 0  (0x0800)\n"
        "MOVI TBUF, #3841   ; O (Index: 15) -> Dir 1  (0x0F01)\n"
        "MOVI TBUF, #3074   ; L (Index: 12) -> Dir 2  (0x0C02)\n"
        "MOVI TBUF, #259    ; A (Index: 1)  -> Dir 3  (0x0103)\n"
        "MOVI TBUF, #4      ;   (Index: 0)  -> Dir 4  (0x0004)\n"
        "MOVI TBUF, #3333   ; M (Index: 13) -> Dir 5  (0x0D05)\n"
        "MOVI TBUF, #5382   ; U (Index: 21) -> Dir 6  (0x1506)\n"
        "MOVI TBUF, #3591   ; N (Index: 14) -> Dir 7  (0x0E07)\n"
        "MOVI TBUF, #1032   ; D (Index: 4)  -> Dir 8  (0x0408)\n"
        "MOVI TBUF, #3849   ; O (Index: 15) -> Dir 9  (0x0F09)\n";
        
        /*
        Agregar cuando halla vblank
        "; ---- 2. Configuración e Inicio de Scroll (ROM 10 a 11) ----\n"
        "MOVI R0, #1         ; Dir 10: R0 = 1\n"
        "MOVI R2, #127     ; Dir 11: R2 = 127\n"
        "MOVI R1, #0         ; Dir 12: R0 = 0\n"
        "\n"
        "; ---- 3. Bucle de Animación de Scroll (ROM 12+) ----\n"
        "ADD SCROLL, R1, R1, R0  ; Dir 13 (BUCLE): R1 = R1 + R0 y actualiza SCROLL GPU\n"
        "CMP R1, R2              ; Dir 14 (BUCLE): R1 = R1 + R0 y actualiza SCROLL GPU\n"
        "JZ #12                  ; Dir 15: Salta de regreso a la instrucción ADD en dir 12\n"
        "JMP #13                  ; Dir 16: Salta de regreso a la instrucción ADD en dir 12\n"
         */


    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN GPU - HOLA MUNDO CON SCROLL 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_gpu.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}