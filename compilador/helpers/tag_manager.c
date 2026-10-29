#include "tag_manager.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <string.h>

// Instancia estática global para la tabla de símbolos
static TablaSimbolos tabla_simbolos = { .cantidad = 0 };

void inicializar_tag_manager(void) {
    tabla_simbolos.cantidad = 0;
}

bool registrar_etiqueta(const char* nombre, uint32_t direccion) {
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

void pre_escaneo_etiquetas(const char* codigo_fuente) {
    inicializar_tag_manager();
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
        Token primer_token = tokens_linea[0];

        if (strcmp(primer_token.lexema, "RET") == 0) {
            pc_fisico += 2;
        } else {
            pc_fisico += 1;
        }
    }
}