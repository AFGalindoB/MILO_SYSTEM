#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lexer/lexer.h"
#include "../../parser/parser.h"
#include "../../codificador/codificador.h"
#include "../../helpers/escritor_rom.h"

int main() {
    // El set de instrucciones propuesto de movimientos generales
    const char* codigo_prueba = 
        "MOVI R0, #5\n"
        "MOV R1, R0\n"
        "STORE R0, [R0]\n"
        "LOAD R5, [R0]\n";

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE MOVIMIENTOS GENERALES (RAM/REGISTROS) 🔥 \n");
    printf("====================================================================\n\n");
    printf("Código fuente a procesar:\n");
    printf("--------------------------------------------------------------------\n");
    printf("%s", codigo_prueba);
    printf("--------------------------------------------------------------------\n\n");

    // Inicializar el frontend
    inicializar_lexer(codigo_prueba);
    avanzar_token();

    // Buffer para almacenar la ROM generada
    PalabraROM programa_rom[256];
    uint32_t contador_instrucciones = 0;

    // Procesar las instrucciones una por una
    while (!match(TOKEN_EOF)) {
        InstruccionParseada instr = parsear_linea();

        if (instr.tipo == INSTR_DESCONOCIDA) {
            continue;
        }

        // Pasar la estructura abstracta al codificador físico de 64 bits
        programa_rom[contador_instrucciones] = codificar_instruccion(&instr);
        contador_instrucciones++;
    }

    // Exportar el resultado usando el helper centralizado
    exportar_a_txt_rom("test_movimiento_general.txt", programa_rom, contador_instrucciones);

    printf("\n Estructuras empaquetadas y volcadas con éxito.\n");
    printf("====================================================================\n");

    return 0;
}