#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Estado interno del Lexer: Variables globales estáticas del módulo
static const char* cursor_codigo;
static uint32_t linea_actual = 1;
static uint32_t columna_actual = 1;

void inicializar_lexer(const char* dir_mem_codigo_fuente) {
    cursor_codigo = dir_mem_codigo_fuente;
    linea_actual = 1;
    columna_actual = 1;
}

// Función auxiliar para leer el carácter al que apunta el cursor actualmente SIN avanzar
static char peek(void) {
    return *cursor_codigo;
}

// Retorna el carácter actual y avanza el cursor a la siguiente dirección de memoria
static char avanzar_cursor_codigo(void) {
    char caracter_actual = *cursor_codigo;
    if (caracter_actual != '\0') {
        cursor_codigo++;  // Avanza el puntero al siguiente byte en memoria
        columna_actual++;
    }
    return caracter_actual;
}

// Construye y devuelve un Token completamente inicializado
static Token crear_token(TipoToken tipo, const char* lexema, uint32_t valor, uint32_t l, uint32_t c) {
    Token t;
    t.tipo = tipo;
    t.valor = valor;
    t.linea = l;
    t.columna = c;
    strncpy(t.lexema, lexema, sizeof(t.lexema) - 1);
    t.lexema[sizeof(t.lexema) - 1] = '\0';
    return t;
}

Token obtener_siguiente_token(void) {
    while (peek() != '\0') {
        char c = peek();

        // 1. Saltar espacios en blanco, tabulaciones y retornos de carro
        if (c == ' ' || c == '\t' || c == '\r') {
            avanzar_cursor_codigo();
            continue;
        }

        // 2. Control del salto de línea para actualizar la matriz de coordenadas
        if (c == '\n') {
            uint32_t l = linea_actual;
            uint32_t col = columna_actual;
            avanzar_cursor_codigo();
            linea_actual++;
            columna_actual = 1; // El cursor vuelve al inicio de la columna en la nueva línea
            return crear_token(TOKEN_SALTO_LINEA, "\\n", 0, l, col);
        }

        // 3. Ignorar Comentarios (Avanzar el cursor hasta encontrar el fin de la línea)
        if (c == ';') {
            while (peek() != '\n' && peek() != '\0') {
                avanzar_cursor_codigo();
            }
            continue; 
        }

        // Registrar las coordenadas exactas de dónde empieza el token actual
        uint32_t l_start = linea_actual;
        uint32_t c_start = columna_actual;

        // 4. Mapeo de caracteres de puntuación simples
        if (c == ',') { avanzar_cursor_codigo(); return crear_token(TOKEN_COMA, ",", 0, l_start, c_start); }
        if (c == '[') { avanzar_cursor_codigo(); return crear_token(TOKEN_CORCHETE_I, "[", 0, l_start, c_start); }
        if (c == ']') { avanzar_cursor_codigo(); return crear_token(TOKEN_CORCHETE_D, "]", 0, l_start, c_start); }
        if (c == ':') { avanzar_cursor_codigo(); return crear_token(TOKEN_DOSPUNTOS, ":", 0, l_start, c_start); }

        // 5. Gestion de modificadores de los identificadores
        if (c == '.') {
            avanzar_cursor_codigo();
            char buffer[32] = {0};
            int i = 0;
            buffer[0] = '.';
            i++;

            // Leer caracteres alfabéticos después del punto (ej: "F")
            while (isalpha((unsigned char)peek()) && i < 31) {
                buffer[i++] = toupper((unsigned char)avanzar_cursor_codigo());
            }

            if (i == 1) { // Error: Escribieron el punto '.' solo sin letras
                return crear_token(TOKEN_ERROR, ".", 0, l_start, c_start);
            }

            // Devolver el token completo con lexema ".F", ".W", etc.
            return crear_token(TOKEN_MODIFICADOR, buffer, 0, l_start, c_start);
        }

        // 6. Procesamiento de palabras (Registros o Identificadores generales)
        if (isalpha((unsigned char)c)) {
            char buffer[32] = {0};
            int i = 0;
            
            // Extraer caracteres alfanuméricos consecutivos
            while (isalnum((unsigned char)peek()) && i < 31) {
                buffer[i++] = toupper((unsigned char)avanzar_cursor_codigo());
            }

            // Validación de Registros (Ej: R0 - R15)
            if (buffer[0] == 'R' && isdigit((unsigned char)buffer[1])) {
                uint32_t num_reg = atoi(&buffer[1]);
                if (num_reg <= 15) {
                    return crear_token(TOKEN_REGISTRO, buffer, num_reg, l_start, c_start);
                }
            }
            
            // Si empieza por letras pero no es un registro válido, es un Identificador
            return crear_token(TOKEN_IDENTIFICADOR, buffer, 0, l_start, c_start);
        }

        // 7. Procesamiento de Inmediatos (Ej: #10)
        if (c == '#') {
            avanzar_cursor_codigo(); // Consumimos el carácter '#' en memoria
            char buffer[32] = {0};
            int i = 0;
            buffer[0] = '#';
            i++;

            while (isdigit((unsigned char)peek()) && i < 31) {
                buffer[i++] = avanzar_cursor_codigo();
            }

            if (i == 1) { // Error: Escribieron el '#' pero no le pusieron ningún número
                return crear_token(TOKEN_ERROR, "#", 0, l_start, c_start);
            }

            uint32_t val = atoi(&buffer[1]);
            return crear_token(TOKEN_INMEDIATO, buffer, val, l_start, c_start);
        }

        // 8. Números puros (Direcciones de memoria directas o literales)
        if (isdigit((unsigned char)c)) {
            char buffer[32] = {0};
            int i = 0;

            while (isdigit((unsigned char)peek()) && i < 31) {
                buffer[i++] = avanzar_cursor_codigo();
            }

            uint32_t val = atoi(buffer);
            return crear_token(TOKEN_NUMERO, buffer, val, l_start, c_start);
        }

        // 9. Error Léxico: El carácter actual no encaja en ningún autómata
        char err_lex[2] = { avanzar_cursor_codigo(), '\0' };
        return crear_token(TOKEN_ERROR, err_lex, 0, l_start, c_start);
    }

    // Fin del flujo de memoria alcanzado
    return crear_token(TOKEN_EOF, "EOF", 0, linea_actual, columna_actual);
}

const char* tipo_token_a_string(TipoToken tipo) {
    switch (tipo) {
        case TOKEN_IDENTIFICADOR: return "IDENTIFICADOR";
        case TOKEN_REGISTRO:      return "REGISTRO";
        case TOKEN_INMEDIATO:     return "INMEDIATO";
        case TOKEN_NUMERO:        return "NUMERO";
        case TOKEN_COMA:          return "COMA";
        case TOKEN_CORCHETE_I:    return "CORCHETE_IZQ";
        case TOKEN_CORCHETE_D:    return "CORCHETE_DER";
        case TOKEN_DOSPUNTOS:     return "DOSPUNTOS";
        case TOKEN_SALTO_LINEA:   return "SALTO_LINEA";
        case TOKEN_EOF:           return "EOF";
        case TOKEN_MODIFICADOR:   return "MODIFICADOR";
        default:                  return "ERROR_LEXICO";
    }
}