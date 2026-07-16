#include "compilador_helper.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../codificador/codificador.h"
#include "escritor_rom.h"
#include <stdio.h>

int compilar_y_exportar(const char* nombre_archivo, const char* codigo_fuente) {
    // 1. Inicializar el frontend del compilador
    inicializar_lexer(codigo_fuente);
    avanzar_token();

    // Reiniciar contadores y flags globales de control de errores
    contador_errores = 0;

    // Buffer temporal en memoria para simular el almacenamiento de la ROM (máximo 256 palabras de 64 bits)
    PalabraROM programa_rom[256];
    uint32_t contador_instrucciones = 0;

    // 2. Ciclo principal de parsing y codificación directa
    while (!match(TOKEN_EOF)) {
        InstruccionParseada instr = parsear_linea();

        // Si la instrucción es inválida debido a errores, la ignoramos para la ROM
        if (instr.tipo == INSTR_DESCONOCIDA) {
            if (match(TOKEN_SALTO_LINEA)) {
                avanzar_token();
            }
            continue;
        }

        // Si se supera la capacidad máxima física de la ROM simulada
        if (contador_instrucciones >= 256) {
            printf("[ERROR COMPILADOR]: Se ha excedido el límite de 256 instrucciones para la ROM.\n");
            break;
        }

        // Codificar la instrucción parseada a microcódigo binario
        programa_rom[contador_instrucciones] = codificar_instruccion(&instr);
        contador_instrucciones++;

        if (match(TOKEN_SALTO_LINEA)) {
            avanzar_token();
        }
    }

    // 3. Emitir informe final del análisis sintáctico
    emitir_informe_compilacion();

    // 4. Exportar el archivo si no hubo errores que detuvieran el proceso de compilación
    if (contador_errores == 0 && contador_instrucciones > 0) {
        exportar_a_txt_rom(nombre_archivo, programa_rom, contador_instrucciones);
        printf(" 🎉 Compilación exitosa. Archivo '%s' generado con %u instrucciones.\n", nombre_archivo, contador_instrucciones);
        return 0;
    } else {
        printf(" ❌ Compilación fallida. No se generó el archivo de salida debido a errores sintácticos (%u detectados).\n", contador_errores);
        return 1;
    }
}