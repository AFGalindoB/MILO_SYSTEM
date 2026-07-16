#include "parser.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Definición de las variables globales compartidas del módulo
Token token_actual;
uint32_t contador_errores = 0;

// Bandera interna para evitar errores en cascada dentro de la misma instrucción
static int en_panico = 0;

// Búfer privado para almacenar el historial de errores
static RegistroError historial_errores[MAX_ERRORES];

void emitir_informe_compilacion(void) {
    if (contador_errores == 0) {
        printf("\n====================================================================\n");
        printf(" 🎉 COMPILACIÓN COMPLETADA EXITOSAMENTE (0 Errores) 🎉\n");
        printf("====================================================================\n");
        printf(" El análisis sintáctico ha terminado sin novedades.\n");
        printf(" Estado del sistema: EXCELENTE. Listo para codificación.\n");
        printf("====================================================================\n\n");
        return;
    }

    printf("\n====================================================================\n");
    printf(" 🔴 REPORTE DE ERRORES SINTÁCTICOS DETECTADOS (%d/%d)\n", contador_errores, MAX_ERRORES);
    printf("====================================================================\n");

    for (uint32_t i = 0; i < contador_errores; i++) {
        printf("\n [%u] Error en Línea %d, Columna %d:\n", i + 1, historial_errores[i].linea, historial_errores[i].columna);
        printf("     Detalle: %s\n", historial_errores[i].mensaje);
        printf("     Causa: se encontró el lexema '%s' de tipo [%s]\n", historial_errores[i].lexema, historial_errores[i].tipo_token_str);
        printf(" --------------------------------------------------------------------\n");
    }

    printf("\n Resultado del proceso: COMPILACIÓN ABORTADA DEBIDO A ERRORES.\n");
    printf("====================================================================\n\n");
}

static void error_sintactico(const char* formato, ...) {
    
    if (en_panico) return;

    en_panico = 1;

    if (contador_errores >= MAX_ERRORES) return;
    
    
    RegistroError* err = &historial_errores[contador_errores];
    err->linea = token_actual.linea;
    err->columna = token_actual.columna;

    strncpy(err->lexema, token_actual.lexema, sizeof(err->lexema) - 1);
    strncpy(err->tipo_token_str, tipo_token_a_string(token_actual.tipo), sizeof(err->tipo_token_str) - 1);

    va_list args;
    va_start(args, formato);
    vsnprintf(err->mensaje, sizeof(err->mensaje) - 1, formato, args);
    va_end(args);

    contador_errores++;

    if (contador_errores >= MAX_ERRORES) {
        printf("\n⚠️  Se ha alcanzado el límite máximo de errores sintácticos (%d).\n", MAX_ERRORES);
        emitir_informe_compilacion();
        exit(EXIT_FAILURE);
    }

    // Sincronizar: saltamos el resto de la línea ruidosa
    while (!match(TOKEN_SALTO_LINEA) && !match(TOKEN_EOF)) {
        avanzar_token();
    }
}

void avanzar_token(void) {
    token_actual = obtener_siguiente_token();
}

int match(TipoToken tipo) {
    return token_actual.tipo == tipo;
}

int match_lexema(TipoToken tipo, const char* lexema) {
    return (token_actual.tipo == tipo && strcmp(token_actual.lexema, lexema) == 0);
}

void consumir(TipoToken tipo_esperado, const char* mensaje_error) {
    if (token_actual.tipo == tipo_esperado) {
        avanzar_token();
    } else {
        error_sintactico("%s", mensaje_error);
    }
}

void consumir_lexema(TipoToken tipo_esperado, const char* lexema_esperado, const char* mensaje_error) {
    if (token_actual.tipo == tipo_esperado && strcmp(token_actual.lexema, lexema_esperado) == 0) {
        avanzar_token();
    } else {
        error_sintactico("%s", mensaje_error);
    }
}

InstruccionParseada parsear_linea(void) {
    InstruccionParseada instr;
    memset(&instr, 0, sizeof(InstruccionParseada));
    
    while (match(TOKEN_SALTO_LINEA)) {
        avanzar_token();
    }

    en_panico = 0;
    
    if (match(TOKEN_EOF)) {
        instr.tipo = INSTR_DESCONOCIDA;
        return instr;
    }
    
    instr.linea = token_actual.linea;
    
    if (match(TOKEN_IDENTIFICADOR)) {
        const char* mnemonico = token_actual.lexema;
        
        if (strcmp(mnemonico, "NOP") == 0) {
            instr.tipo = INSTR_NOP;
            avanzar_token();
        } 
        // --- Bloque de Reenvío a Movimientos y Transferencias ---
        else if (strcmp(mnemonico, "MOV") == 0)   { instr.tipo = INSTR_MOV;  avanzar_token(); parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "MOVI") == 0)  { instr.tipo = INSTR_MOVI; avanzar_token(); parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "LOAD") == 0)  { instr.tipo = INSTR_LOAD; avanzar_token(); parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "STORE") == 0) { instr.tipo = INSTR_STORE; avanzar_token(); parsear_movimiento(&instr); }
        
        // --- Bloque de Reenvío a ALU ---
        else if (strcmp(mnemonico, "ADD") == 0) { instr.tipo = INSTR_ADD; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "ADC") == 0) { instr.tipo = INSTR_ADC; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SUB") == 0) { instr.tipo = INSTR_SUB; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SBC") == 0) { instr.tipo = INSTR_SBC; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "AND") == 0) { instr.tipo = INSTR_AND; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "OR") == 0)  { instr.tipo = INSTR_OR;  avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "XOR") == 0) { instr.tipo = INSTR_XOR; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "NOT") == 0) { instr.tipo = INSTR_NOT; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SHL") == 0) { instr.tipo = INSTR_SHL; avanzar_token(); parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SHR") == 0) { instr.tipo = INSTR_SHR; avanzar_token(); parsear_alu(&instr); }
        else {
            error_sintactico("Mnemónico de instrucción desconocido '%s'", mnemonico);
        }
    } else {
        error_sintactico("Se esperaba un mnemónico válido al inicio de la línea de comandos");
    }
    
    return instr;
}