#include "compilador_helper.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../encoder/encoder.h"
#include "escritor_rom.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static TablaSimbolos tabla_simbolos = { .cantidad = 0 };

static bool registrar_etiqueta(const char* nombre, uint32_t direccion) {
    // Validar si la etiqueta ya fue registrada previamente
    for (int i = 0; i < tabla_simbolos.cantidad; i++) {
        if (strcmp(tabla_simbolos.lista[i].nombre, nombre) == 0) {
            printf("⚠️ [ERROR COMPILADOR]: Redefinición de etiqueta '%s'.\n", nombre);
            return false;
        }
    }

    if (tabla_simbolos.cantidad >= MAX_ETIQUETAS) {
        printf("⚠️ [ERROR COMPILADOR]: Se ha excedido el límite de %d etiquetas.\n", MAX_ETIQUETAS);
        return false;
    }

    // Copiar el nombre asegurando terminación nula
    strncpy(tabla_simbolos.lista[tabla_simbolos.cantidad].nombre, nombre, TAMANO_NOMBRE_ETIQUETA - 1);
    tabla_simbolos.lista[tabla_simbolos.cantidad].nombre[TAMANO_NOMBRE_ETIQUETA - 1] = '\0';
    tabla_simbolos.lista[tabla_simbolos.cantidad].direccion = direccion;
    tabla_simbolos.cantidad++;

    return true;
}

int32_t buscar_etiqueta(const char* nombre) {
    for (int i = 0; i < tabla_simbolos.cantidad; i++) {
        if (strcmp(tabla_simbolos.lista[i].nombre, nombre) == 0) {
            return (int32_t)tabla_simbolos.lista[i].direccion;
        }
    }
    return -1; // Retorna -1 si la etiqueta no existe en la tabla
}

static void pre_escaneo_etiquetas(const char* codigo_fuente) {
    inicializar_lexer(codigo_fuente);
    
    Token tokens_linea[MAX_TOKENS_POR_LINEA];
    uint32_t pc_fisico = 0;
    int es_eof = 0;

    while (!es_eof) {
        int n_tokens = obtener_linea_de_tokens(tokens_linea);
        if (n_tokens == 0) break;

        if (tokens_linea[n_tokens - 1].tipo == TOKEN_EOF) {
            es_eof = 1;
        }

        // 1. Detección del patrón: [TOKEN_IDENTIFICADOR] + [TOKEN_DOSPUNTOS]
        if (n_tokens >= 2 && 
            tokens_linea[0].tipo == TOKEN_IDENTIFICADOR && 
            tokens_linea[1].tipo == TOKEN_DOSPUNTOS) 
        {
            registrar_etiqueta(tokens_linea[0].lexema, pc_fisico);
            
            // Una línea de etiqueta sola NUNCA suma al PC físico
            continue;
        }

        // 2. Conteo de instrucciones físicas en la ROM
        // Buscar el primer token significativo de la línea
        Token primer_token = tokens_linea[0];

        if (strcmp(primer_token.lexema, "RET") == 0) {
            pc_fisico += 2;
        } else {
            pc_fisico += 1;
        }
    }
}

int compilar_y_exportar(const char* nombre_archivo, const char* codigo_fuente) {
    
    pre_escaneo_etiquetas(codigo_fuente);

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