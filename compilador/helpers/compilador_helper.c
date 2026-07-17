#include "compilador_helper.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../encoder/encoder.h"
#include "escritor_rom.h"
#include <stdio.h>

int compilar_y_exportar(const char* nombre_archivo, const char* codigo_fuente) {
    inicializar_lexer(codigo_fuente);
    avanzar_token();

    contador_errores = 0;

    PalabraROM programa_rom[256];
    uint32_t contador_instrucciones = 0;

    // Búfer local para recibir las microinstrucciones generadas por el encoder
    PalabraROM buffer_temporal[4];

    while (!match(TOKEN_EOF)) {
        InstruccionParseada instr = parsear_linea();

        // Ignoramos instrucciones inválidas producidas por errores
        if (instr.tipo == INSTR_DESCONOCIDA) {
            if (match(TOKEN_SALTO_LINEA)) {
                avanzar_token();
            }
            continue;
        }

        // Dejar que el encoder maneje la lógica interna y expansiones
        int micros_generadas = codificar_instruccion(&instr, buffer_temporal);

        for (int i = 0; i < micros_generadas; i++) {
            if (contador_instrucciones >= 256) {
                printf("[ERROR COMPILADOR]: Se ha excedido el límite de 256 instrucciones físicas de la ROM.\n");
                break;
            }
            programa_rom[contador_instrucciones] = buffer_temporal[i];
            contador_instrucciones++;
        }

        if (match(TOKEN_SALTO_LINEA)) {
            avanzar_token();
        }
    }

    emitir_informe_compilacion(codigo_fuente);

    if (contador_errores == 0 && contador_instrucciones > 0) {
        exportar_a_txt_rom(nombre_archivo, programa_rom, contador_instrucciones);
        printf(" 🎉 Compilación exitosa. Archivo '%s' generado con %u palabras de control físicas.\n", nombre_archivo, contador_instrucciones);
        return 0;
    } else {
        printf(" ❌ Compilación fallida. No se generó el archivo de salida debido a errores (%u detectados).\n", contador_errores);
        return 1;
    }
}