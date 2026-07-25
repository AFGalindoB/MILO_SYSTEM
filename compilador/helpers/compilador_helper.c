#include "compilador_helper.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../encoder/encoder.h"
#include "escritor_rom.h"
#include <stdio.h>

int compilar_y_exportar(const char* nombre_archivo, const char* codigo_fuente) {
    // 1. Inicializamos el lexer con el buffer en memoria
    inicializar_lexer(codigo_fuente);

    contador_errores = 0;

    PalabraROM programa_rom[256];
    uint32_t contador_instrucciones = 0;

    // Búfer local para recibir las microinstrucciones generadas por el encoder (ej. RET expande a 2)
    PalabraROM buffer_temporal[4];

    Token tokens_linea[MAX_TOKENS_POR_LINEA];
    int cantidad_tokens = 0;
    int es_eof = 0;

    // 2. Ciclo principal por líneas de tokens (alineado con la arquitectura del Parser)
    while (!es_eof) {
        cantidad_tokens = obtener_linea_de_tokens(tokens_linea);

        if (cantidad_tokens == 0) {
            break;
        }

        // Si el último token es EOF, procesamos esta última línea y cerramos la iteración
        if (tokens_linea[cantidad_tokens - 1].tipo == TOKEN_EOF) {
            es_eof = 1;
        }

        // Enviamos los tokens de la línea actual al parser
        InstruccionIR instr = parsear_linea_tokens(tokens_linea, cantidad_tokens);

        // Ignoramos instrucciones inválidas, comentarios o líneas vacías
        if (instr.tipo == INSTR_DESCONOCIDA) {
            continue;
        }

        // 3. Generación de microcódigo con el Encoder
        int micros_generadas = codificar_instruccion(&instr, buffer_temporal);

        for (int i = 0; i < micros_generadas; i++) {
            if (contador_instrucciones >= 256) {
                printf("⚠️ [ERROR COMPILADOR]: Se ha excedido el límite de 256 instrucciones físicas de la ROM.\n");
                contador_errores++;
                break;
            }
            programa_rom[contador_instrucciones] = buffer_temporal[i];
            contador_instrucciones++;
        }
    }

    // 4. Imprimir informe gráfico de errores si los hubo (utilizando las comillas y la flechita '^')
    emitir_informe_compilacion();

    // 5. Generación del archivo de salida
    if (contador_errores == 0 && contador_instrucciones > 0) {
        exportar_a_txt_rom(nombre_archivo, programa_rom, contador_instrucciones);
        printf("\n🎉 ¡Compilación exitosa! Archivo '%s' generado con %u palabras de control físicas.\n", nombre_archivo, contador_instrucciones);
        return 0;
    } else {
        printf("\n❌ Compilación fallida. No se generó el archivo de salida debido a errores (%u detectados).\n", contador_errores);
        return 1;
    }
}