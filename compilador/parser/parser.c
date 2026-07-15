#include "parser.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Definición de las variables globales compartidas del módulo
Token token_actual;

static void error_sintactico(const char* formato, ...) {
    printf("\n====================================================================\n");
    printf(" ❌ ERROR SINTÁCTICO CRÍTICO [Línea %d, Columna %d]\n", token_actual.linea, token_actual.columna);
    printf("====================================================================\n");
    printf(" Detalle: ");
    
    // Procesar los argumentos variables (como printf) para mensajes personalizados
    va_list args;
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    
    printf("\n Se encontró el lexema: '%s' (Tipo: %s)\n", token_actual.lexema, tipo_token_a_string(token_actual.tipo));
    printf("====================================================================\n");
    printf(" Compilación finalizada debido a errores sintácticos.\n");
    
    exit(EXIT_FAILURE); // Termina el proceso inmediatamente de forma nativa
}

void avanzar_token(void) {
    token_actual = obtener_siguiente_token();
}

int match(TipoToken tipo) {
    return token_actual.tipo == tipo;
}

void consumir(TipoToken tipo_esperado, const char* mensaje_error) {
    if (token_actual.tipo == tipo_esperado) {
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
    
    if (match(TOKEN_EOF)) {
        instr.tipo = INSTR_DESCONOCIDA;
        return instr;
    }
    
    instr.linea = token_actual.linea;
    
    // Identificar la instrucción base y reenviar al script correspondiente
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
        else if (strcmp(mnemonico, "SUB") == 0) { instr.tipo = INSTR_SUB; avanzar_token(); parsear_alu(&instr); }
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