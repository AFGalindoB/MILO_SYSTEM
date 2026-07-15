#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lexer/lexer.h"
#include "../../parser/parser.h"
#include "../../codificador/codificador.h"
#include "../../helpers/escritor_rom.h"

int main() {
    // El set de instrucciones propuesto para la ALU (con la corrección del registro destino en XOR)
    const char* codigo_prueba = 
        "MOVI R0, #5\n"
        "MOVI R1, #1\n"
        "ADD R2, R0 R1\n"
        "SUB R3, R0 R1\n"
        "AND R4, R0 R0\n"
        "OR R5, R0 R1\n"
        "XOR R6, R0 R2\n"
        "MOVI R7, #8\n"
        "MOVI R8, #4\n"
        "SHL R9, R1 R7\n"
        "SHR R10, R9 R8\n"
        "NOT R11, R1\n";

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE OPERACIONES ARITMÉTICO-LÓGICAS (ALU) 🔥  \n");
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
    exportar_a_txt_rom("test_alu.txt", programa_rom, contador_instrucciones);

    printf("\n Estructuras de la ALU empaquetadas y volcadas con éxito.\n");
    printf("====================================================================\n");

    return 0;
}