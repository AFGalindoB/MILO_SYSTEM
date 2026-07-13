#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lexer/lexer.h"
#include "../../parser/parser.h"
#include "../../codificador/codificador.h"
#include "../../helpers/escritor_rom.h"

int main() {
    // Generar el set de instrucciones dinámico (MOVI R0, #1 ... MOVI R15, #16)
    char codigo_prueba[1024] = {0};
    char linea_buffer[64];
    
    for (int i = 0; i <= 15; i++) {
        sprintf(linea_buffer, "MOVI R%d, #%d\n", i, i + 1);
        strcat(codigo_prueba, linea_buffer);
    }

    // Inicializar el frontend del compilador
    inicializar_lexer(codigo_prueba);
    avanzar_token();

    // Crear un arreglo estático en memoria para simular el almacenamiento total de la ROM (máximo 256 instrucciones por ahora)
    PalabraROM programa_rom[256];
    uint32_t contador_instrucciones = 0;

    printf("====================================================================\n");
    printf("   🔥 TEST DE COMPILACIÓN COMPLETA CON HELPER DE EXPORTACIÓN 🔥      \n");
    printf("====================================================================\n\n");

    // Procesar el flujo y llenar el buffer del programa
    while (!match(TOKEN_EOF)) {
        InstruccionParseada instr = parsear_linea();

        if (instr.tipo == INSTR_DESCONOCIDA) {
            continue;
        }

        // Codificar a microcódigo binario
        programa_rom[contador_instrucciones] = codificar_instruccion(&instr);
        contador_instrucciones++;
    }

    // Invocar al helper para que empaquete y genere el archivo de texto en la carpeta externa
    exportar_a_txt_rom("test_movi.txt", programa_rom, contador_instrucciones);

    printf("\n Proceso finalizado. Verifica el archivo generado en la carpeta 'compilaciones/'.\n");
    printf("====================================================================\n");

    return 0;
}