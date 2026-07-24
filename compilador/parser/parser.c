#include "parser.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

uint32_t contador_errores = 0;
static RegistroError historial_errores[MAX_ERRORES];

Token* linea_tokens = NULL;
int total_tokens = 0;

int match_tipo(int offset, TipoToken tipo) {
    if (offset >= total_tokens){
        return 0;
    }

    return linea_tokens[offset].tipo == tipo;
}

int match_lexema(int offset, const char* lexema) {
    if (offset >= total_tokens){
        return 0;
    }

    return strcmp(linea_tokens[offset].lexema, lexema) == 0;
}

static uint8_t mapear_registro_especial(uint8_t offset) {
    if (linea_tokens[offset].tipo == TOKEN_IDENTIFICADOR) {
        if (strcmp(linea_tokens[offset].lexema, "TBUF") == 0)   { return 1; }
        if (strcmp(linea_tokens[offset].lexema, "SCROLL") == 0) { return 2; }
    }
    return 0;
}

static int contar_tokens_de_tipo(TipoToken tipo) {
    int conteo = 0;
    for (int i = 0; i < total_tokens; i++) {
        if (linea_tokens[i].tipo == tipo) {
            conteo++;
        }
    }
    return conteo;
}

uint8_t obtener_destinos_ortogonales(InstruccionParseada* instr) {
    uint8_t n = 1; // Índice del primer token después del mnemónico
    if (match_tipo(n, TOKEN_MODIFICADOR)) {
        n++; // Avanzamos si hay un modificador
    }

    uint8_t reg_especial = mapear_registro_especial(n);

    if(reg_especial) {
        instr->reg_gpu = reg_especial;

        if (!match_tipo(n + 1, TOKEN_COMA)) {
            reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después del registro especial.");
            return 0;
        }

        n += 2; 
        
        uint8_t registros_minimos = 0;

        if (instr->tipo == INSTR_MOVI) {
            registros_minimos = 1;
        } else if (instr->tipo == INSTR_MOV || instr->tipo == INSTR_LOAD || instr->tipo == INSTR_NOT) {
            registros_minimos = 2;
        }else{
            registros_minimos = 3;
        }

        uint8_t registros_encontrados = contar_tokens_de_tipo(TOKEN_REGISTRO) + 1; // +1 para incluir el registro especial como un registro.

        // Si hay MÁS registros de los mínimos requeridos por la fuente, el primero es obligatoriamente Rd
        if (match_tipo(n, TOKEN_REGISTRO) && registros_encontrados > registros_minimos) {
            instr->tiene_rd = 1;
            instr->rd = linea_tokens[n].valor;
            
            n += 2;
        }

    }else{
        if (!match_tipo(n, TOKEN_REGISTRO)) {
            reportar_error(n, TOKEN_REGISTRO, "Se esperaba un registro destino después del mnemónico.");
            return 0 ;
        }

        if (!match_tipo(n + 1, TOKEN_COMA)) {
            reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después del registro destino.");
            return 0;
        }

        instr->tiene_rd = 1;
        instr->rd = linea_tokens[n].valor;
        n += 2; // Avanzamos al siguiente token después del registro destino y la coma
    }

    return n; // Retornamos el índice del primer token después de los destinos ortogonales
}

void reportar_error(int offset_error, TipoToken tipo_error, const char* formato, ...) {
    
    RegistroError* err = &historial_errores[contador_errores];
    
    err->linea_completa_str[0] = '\0';
    int columna_error = 0;
    int encontrado = 0;

    err->linea = linea_tokens[0].linea;
    strncpy(err->tipo_token_str, tipo_token_a_string(tipo_error), sizeof(err->tipo_token_str) - 1);

    for (int i = 0; i < total_tokens; i++) {

        if (i == offset_error) {
            err->columna = linea_tokens[i].columna;
            encontrado = 1;
        }

        strncat(err->linea_completa_str, linea_tokens[i].lexema, sizeof(err->linea_completa_str) - strlen(err->linea_completa_str) - 1);

        if (i < total_tokens - 1) {
            strncat(err->linea_completa_str, " ", sizeof(err->linea_completa_str) - strlen(err->linea_completa_str) - 1);
        }

        if (!encontrado) {
            columna_error += strlen(linea_tokens[i].lexema) + 1; 
        }
    }

    if (!encontrado) {
        err->columna = columna_error + 1; 
    }

    va_list args;
    va_start(args, formato);
    vsnprintf(err->mensaje, sizeof(err->mensaje) - 1, formato, args);
    va_end(args);

    contador_errores++;

    if (contador_errores >= MAX_ERRORES) {
        printf("\n⚠️ Límite máximo de errores alcanzado (%d).\n", MAX_ERRORES);
        exit(EXIT_FAILURE);
    }
}

void emitir_informe_compilacion(void) { // <-- ¡Ya no requiere parámetros de texto!
    if (contador_errores == 0) {
        printf("\n====================================================================\n");
        printf(" 🎉 COMPILACIÓN COMPLETADA EXITOSAMENTE (0 Errores) 🎉\n");
        printf("====================================================================\n\n");
        return;
    }

    printf("\n====================================================================\n");
    printf(" 🔴 REPORTE DE ERRORES SINTÁCTICOS DETECTADOS (%d)\n", contador_errores);
    printf("====================================================================\n");

    for (uint32_t i = 0; i < contador_errores; i++) {
        RegistroError* err = &historial_errores[i];

        printf("\n [%u] Error sintáctico en Línea %d, Columna %d:\n", i + 1, err->linea, err->columna);
        printf("     Detalle: %s\n", err->mensaje);

        // Imprimir la línea reconstruida que guardamos
        printf("     %4d | %s\n", err->linea, err->linea_completa_str);
        
        // Graficar el puntero '^' usando la columna almacenada
        printf("            "); // Margen base para alinear con el separador '| '
        for (int k = 1; k < err->columna; k++) {
            printf(" ");
        }
        printf(" ^\n");
        printf(" --------------------------------------------------------------------\n");
    }

    printf("\n Resultado del proceso: COMPILACIÓN ABORTADA.\n");
    printf("====================================================================\n\n");
}

InstruccionParseada parsear_linea_tokens(Token* tokens, int cantidad_tokens) {
    InstruccionParseada instr;
    memset(&instr, 0, sizeof(InstruccionParseada));

    // Inicializamos el entorno de la línea
    linea_tokens = tokens;
    total_tokens = cantidad_tokens;

    instr.linea = linea_tokens[0].linea; // Guardamos la línea para reportes de error

    // Si es una línea vacía o el final del archivo, terminamos de inmediato
    if (total_tokens == 0 || match_tipo(0, TOKEN_EOF)) {
        instr.tipo = INSTR_DESCONOCIDA;
        return instr;
    }

    // Cada instrucción DEBE comenzar con un Mnemónico (TOKEN_IDENTIFICADOR)
    if (match_tipo(0, TOKEN_IDENTIFICADOR)) {
        const char* mnemonico = linea_tokens[0].lexema;

        if (strcmp(mnemonico, "NOP") == 0) {instr.tipo = INSTR_NOP;}
        else if (strcmp(mnemonico, "MOV") == 0)   { instr.tipo = INSTR_MOV;   parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "MOVI") == 0)  { instr.tipo = INSTR_MOVI;  parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "LOAD") == 0)  { instr.tipo = INSTR_LOAD;  parsear_movimiento(&instr); }
        else if (strcmp(mnemonico, "STORE") == 0) { instr.tipo = INSTR_STORE; parsear_movimiento(&instr); }

        // --- Bloque de Reenvío a ALU ---
        else if (strcmp(mnemonico, "ADD") == 0) { instr.tipo = INSTR_ADD; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "ADC") == 0) { instr.tipo = INSTR_ADC; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SUB") == 0) { instr.tipo = INSTR_SUB; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SBC") == 0) { instr.tipo = INSTR_SBC; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "AND") == 0) { instr.tipo = INSTR_AND; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "OR") == 0)  { instr.tipo = INSTR_OR;  parsear_alu(&instr); }
        else if (strcmp(mnemonico, "XOR") == 0) { instr.tipo = INSTR_XOR; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "NOT") == 0) { instr.tipo = INSTR_NOT; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SHL") == 0) { instr.tipo = INSTR_SHL; parsear_alu(&instr); }
        else if (strcmp(mnemonico, "SHR") == 0) { instr.tipo = INSTR_SHR; parsear_alu(&instr); }

        // --- Bloque de Control de Flujo (CMP, Saltos, Subrutinas) ---
        else if (strcmp(mnemonico, "CMP") == 0)   { instr.tipo = INSTR_CMP;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JMP") == 0)   { instr.tipo = INSTR_JMP;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JZ") == 0)    { instr.tipo = INSTR_JZ;   parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JNZ") == 0)   { instr.tipo = INSTR_JNZ;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JC") == 0)    { instr.tipo = INSTR_JC;   parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JNC") == 0)   { instr.tipo = INSTR_JNC;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JN") == 0)    { instr.tipo = INSTR_JN;   parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JNN") == 0)   { instr.tipo = INSTR_JNN;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JV") == 0)    { instr.tipo = INSTR_JV;   parsear_pc(&instr); }
        else if (strcmp(mnemonico, "JNV") == 0)   { instr.tipo = INSTR_JNV;  parsear_pc(&instr); }
        else if (strcmp(mnemonico, "CALL") == 0)  { instr.tipo = INSTR_CALL; parsear_pc(&instr); }
        else if (strcmp(mnemonico, "RET") == 0)   { instr.tipo = INSTR_RET; } 

        // --- Bloque de control de interrupciones ---
        else if (strcmp(mnemonico, "WAITH") == 0) { instr.tipo = INSTR_WAIT; instr.fuentes.salto.destino = 0; } 
        else if (strcmp(mnemonico, "WAITV") == 0) { instr.tipo = INSTR_WAIT; instr.fuentes.salto.destino = 1; } 

        else { instr.tipo = INSTR_DESCONOCIDA; }

    } else {
        reportar_error(0, TOKEN_IDENTIFICADOR, "Se esperaba un mnemónico válido (identificador) al comienzo de la línea");
    }

    return instr;
}