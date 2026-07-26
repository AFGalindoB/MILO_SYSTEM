#include "parser.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../helpers/compilador_helper.h"

uint32_t contador_errores = 0;
static RegistroError historial_errores[MAX_ERRORES];

Token* linea_tokens = NULL;
int total_tokens = 0;

static int match_tipo(int offset, TipoToken tipo) {
    if (offset >= total_tokens){
        return 0;
    }

    return linea_tokens[offset].tipo == tipo;
}

static int match_lexema(int offset, const char* lexema) {
    if (offset >= total_tokens){
        return 0;
    }

    return strcmp(linea_tokens[offset].lexema, lexema) == 0;
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

static Operando token_a_operando(Token t) {
    Operando op;
    op.valor = t.valor;

    switch (t.tipo) {
        case TOKEN_REGISTRO_RW: op.tipo = OPERANDO_REG_RW; break;
        case TOKEN_REGISTRO_RO: op.tipo = OPERANDO_REG_RO; break;
        case TOKEN_REGISTRO_WO: op.tipo = OPERANDO_REG_WO; break;
        case TOKEN_INMEDIATO:   
        case TOKEN_NUMERO:      op.tipo = OPERANDO_INMEDIATO; break;
        default:                op.tipo = OPERANDO_NULO; break;
    }
    return op;
}

static FormatoInstruccion obtener_formato_mnemonico(const char* lexema) {
    if (strcmp(lexema, "NOP") == 0 || strcmp(lexema, "RET") == 0 || strcmp(lexema, "STOP") == 0)
        return FORMATO_SIN_OPERANDOS;
    
    if (strcmp(lexema, "JMP") == 0 || strcmp(lexema, "JZ") == 0  || strcmp(lexema, "JNZ") == 0 ||
        strcmp(lexema, "JC") == 0  || strcmp(lexema, "JNC") == 0 || strcmp(lexema, "JN") == 0  ||
        strcmp(lexema, "JNN") == 0 || strcmp(lexema, "JV") == 0  || strcmp(lexema, "JNV") == 0 ||
        strcmp(lexema, "CALL") == 0)
        return FORMATO_SALTO;

    if (strcmp(lexema, "MOV") == 0)   return FORMATO_MOV;
    if (strcmp(lexema, "MOVI") == 0)  return FORMATO_MOVI;
    if (strcmp(lexema, "LOAD") == 0)  return FORMATO_LOAD;
    if (strcmp(lexema, "STORE") == 0) return FORMATO_STORE;
    if (strcmp(lexema, "NOT") == 0)   return FORMATO_ALU_UNARIO;

    if (strcmp(lexema, "ADD") == 0 || strcmp(lexema, "SUB") == 0 || strcmp(lexema, "AND") == 0 ||
        strcmp(lexema, "OR")  == 0 || strcmp(lexema, "XOR") == 0 || strcmp(lexema, "ADC") == 0 ||
        strcmp(lexema, "SBC") == 0 || strcmp(lexema, "SHL") == 0 || strcmp(lexema, "SHR") == 0 ||
        strcmp(lexema, "CMP") == 0)
        return FORMATO_ALU_BINARIO;

    return FORMATO_DESCONOCIDO;
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

void emitir_informe_compilacion(void) {
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

static uint8_t obtener_destinos_ortogonales(InstruccionIR* ir, int n) {
    if (match_tipo(n, TOKEN_REGISTRO_WO)) {
        ir->destino[1] = token_a_operando(linea_tokens[n]);
        n++;

        if (!match_tipo(n, TOKEN_COMA)) {
            reportar_error(n, TOKEN_COMA, "Se esperaba ',' después del registro especial WO.");
            return 0;
        }
        n++; // Consumir coma
        
        uint8_t fuentes_requeridas = 0;
        if (ir->formato == FORMATO_MOV || ir->formato == FORMATO_ALU_UNARIO || ir->formato == FORMATO_LOAD) {
            fuentes_requeridas = 1;
        } else if (ir->formato == FORMATO_ALU_BINARIO) {
            fuentes_requeridas = 2;
        }

        uint8_t registros_encontrados = contar_tokens_de_tipo(TOKEN_REGISTRO_RW) 
                                      + contar_tokens_de_tipo(TOKEN_REGISTRO_RO);

        if (match_tipo(n, TOKEN_REGISTRO_RW) && registros_encontrados > fuentes_requeridas) {
            ir->destino[0] = token_a_operando(linea_tokens[n]);
            
            if (!match_tipo(n + 1, TOKEN_COMA)) {
                reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después de Rd.");
                return 0;
            }
            n += 2;
        }

    } else {
        if (!match_tipo(n, TOKEN_REGISTRO_RW)) {
            reportar_error(n, TOKEN_REGISTRO_RW, "Se esperaba un registro destino (Rd) válido.");
            return 0;
        }

        if (!match_tipo(n + 1, TOKEN_COMA)) {
            reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después del registro destino.");
            return 0;
        }

        ir->destino[0] = token_a_operando(linea_tokens[n]);
        n += 2;
    }

    return n; // Retornamos el índice del primer token después de los destinos ortogonales
}

static void parsear_formato_salto(InstruccionIR* ir, int n) {
    if (match_tipo(n, TOKEN_IDENTIFICADOR)) {
        int destino = buscar_etiqueta(linea_tokens[n].lexema);
        if (destino != -1) {
            linea_tokens[n].tipo = TOKEN_INMEDIATO;
            linea_tokens[n].valor = destino;
        }
    }
    if (!match_tipo(n, TOKEN_INMEDIATO) && !match_tipo(n, TOKEN_NUMERO)) {
        reportar_error(n, TOKEN_INMEDIATO, "Se esperaba una dirección o etiqueta para la instrucción de salto.");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);
}

static void parsear_formato_store(InstruccionIR* ir, int n) {
    // Capturar MDR (Registro fuente de datos)
    if (!match_tipo(n, TOKEN_REGISTRO_RW) && !match_tipo(n, TOKEN_INMEDIATO) && !match_tipo(n, TOKEN_REGISTRO_RO)) {
        reportar_error(n, TOKEN_REGISTRO_RW, "Se esperaba una fuente de datos (MDR) en STORE.");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);

    if (!match_tipo(n + 1, TOKEN_COMA)) {
        reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después del registro MDR.");
        return;
    }
    if (!match_tipo(n + 2, TOKEN_CORCHETE_I)) {
        reportar_error(n + 2, TOKEN_CORCHETE_I, "Se esperaba abrir corchete '[' para la dirección del MAR.");
        return;
    }
    if (!match_tipo(n + 3, TOKEN_REGISTRO_RW)) {
        reportar_error(n + 3, TOKEN_REGISTRO_RW, "Se esperaba el registro de dirección (MAR) dentro de los corchetes.");
        return;
    }
    ir->fuente[1] = token_a_operando(linea_tokens[n + 3]);

    if (!match_tipo(n + 4, TOKEN_CORCHETE_D)) {
        reportar_error(n + 4, TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']' tras el registro MAR.");
        return;
    }
}

static void parsear_formato_load(InstruccionIR* ir, int n) {
    // Extraer destinos ortogonales (Rd y/o WO)
    n = obtener_destinos_ortogonales(ir, n);
    if (n == 0) return;

    if (!match_tipo(n, TOKEN_CORCHETE_I)) {
        reportar_error(n, TOKEN_CORCHETE_I, "Se esperaba '[' para la dirección de memoria en LOAD.");
        return;
    }
    if (!match_tipo(n + 1, TOKEN_REGISTRO_RW)) {
        reportar_error(n + 1, TOKEN_REGISTRO_RW, "Se esperaba el registro de dirección (MAR) dentro de los corchetes.");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n + 1]);

    if (!match_tipo(n + 2, TOKEN_CORCHETE_D)) {
        reportar_error(n + 2, TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']' tras el MAR.");
        return;
    }
}

static void parsear_formato_mov_inm(InstruccionIR* ir, int n) {
    n = obtener_destinos_ortogonales(ir, n);
    if (n == 0) return;

    if (!match_tipo(n, TOKEN_INMEDIATO) && !match_tipo(n, TOKEN_NUMERO)) {
        reportar_error(n, TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#inm) en MOVI.");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);
}

static void parsear_formato_alu_unario(InstruccionIR* ir, int n) {
    if (match_tipo(n, TOKEN_MODIFICADOR)) {
        if (match_lexema(n, ".F")){
            ir->tiene_modificador_f = 1;
            n++;
        }else{
            reportar_error(n, TOKEN_MODIFICADOR, "Modificador desconocido. Solo se permite '.F' para actualizar flags.");
            return;
        }
    }

    n = obtener_destinos_ortogonales(ir, n);
    if (n == 0) return;

    if (!match_tipo(n, TOKEN_REGISTRO_RW)) {
        reportar_error(n, TOKEN_REGISTRO_RW, "Se esperaba un registro fuente (Rs).");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);
}

static void parsear_formato_mov(InstruccionIR* ir, int n) {
    n = obtener_destinos_ortogonales(ir, n);
    if (n == 0) return;

    if (!match_tipo(n, TOKEN_REGISTRO_RW) && !match_tipo(n, TOKEN_REGISTRO_RO)) {
        reportar_error(n, TOKEN_REGISTRO_RW, "Se esperaba un registro fuente (Rs / Ra).");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);
}

static void parsear_formato_binario(InstruccionIR* ir, int n) {
    // Si la instrucción es CMP no escribe en destinos, va directo a fuentes
    if (match_tipo(n, TOKEN_MODIFICADOR)) {
        if (match_lexema(n, ".F")){
            ir->tiene_modificador_f = 1;
            n++;
        }else{
            reportar_error(n, TOKEN_MODIFICADOR, "Modificador desconocido. Solo se permite '.F' para actualizar flags.");
            return;
        }
    }

    if (ir->tipo != INSTR_CMP) {
        n = obtener_destinos_ortogonales(ir, n);
        if (n == 0) return;
    }else{
        ir->tiene_modificador_f = 1;
    }

    if (!match_tipo(n, TOKEN_REGISTRO_RW)) {
        reportar_error(n, TOKEN_REGISTRO_RW, "Se esperaba el primer registro fuente (Ra).");
        return;
    }
    ir->fuente[0] = token_a_operando(linea_tokens[n]);

    if (!match_tipo(n + 1, TOKEN_COMA)) {
        reportar_error(n + 1, TOKEN_COMA, "Se esperaba ',' entre los registros fuentes.");
        return;
    }

    if (!match_tipo(n + 2, TOKEN_REGISTRO_RW)) {
        reportar_error(n + 2, TOKEN_REGISTRO_RW, "Se esperaba el segundo registro fuente (Rb).");
        return;
    }
    ir->fuente[1] = token_a_operando(linea_tokens[n + 2]);
}

InstruccionIR parsear_linea_tokens(Token* tokens, int cantidad_tokens) {
    InstruccionIR ir;
    memset(&ir, 0, sizeof(InstruccionIR));

    linea_tokens = tokens;
    total_tokens = cantidad_tokens;

    ir.linea = linea_tokens[0].linea;

    if (total_tokens == 0 || match_tipo(0, TOKEN_EOF)) {
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    // 1. Mnemónico obligatorio al inicio
    if (!match_tipo(0, TOKEN_IDENTIFICADOR)) {
        reportar_error(0, TOKEN_IDENTIFICADOR, "Se esperaba un mnemónico válido al inicio de la línea.");
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    if (total_tokens >= 2 && match_tipo(0, TOKEN_IDENTIFICADOR) && match_tipo(1, TOKEN_DOSPUNTOS)) {
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    const char* mnemonico = linea_tokens[0].lexema;

    if      (strcmp(mnemonico, "NOP") == 0)   ir.tipo = INSTR_NOP;
    else if (strcmp(mnemonico, "MOV") == 0)   ir.tipo = INSTR_MOV;
    else if (strcmp(mnemonico, "MOVI") == 0)  ir.tipo = INSTR_MOVI;
    else if (strcmp(mnemonico, "LOAD") == 0)  ir.tipo = INSTR_LOAD;
    else if (strcmp(mnemonico, "STORE") == 0) ir.tipo = INSTR_STORE;
    else if (strcmp(mnemonico, "ADD") == 0)   ir.tipo = INSTR_ADD;
    else if (strcmp(mnemonico, "SUB") == 0)   ir.tipo = INSTR_SUB;
    else if (strcmp(mnemonico, "ADC") == 0)   ir.tipo = INSTR_ADC;
    else if (strcmp(mnemonico, "SBC") == 0)   ir.tipo = INSTR_SBC;
    else if (strcmp(mnemonico, "AND") == 0)   ir.tipo = INSTR_AND;
    else if (strcmp(mnemonico, "OR") == 0)    ir.tipo = INSTR_OR;
    else if (strcmp(mnemonico, "XOR") == 0)   ir.tipo = INSTR_XOR;
    else if (strcmp(mnemonico, "NOT") == 0)   ir.tipo = INSTR_NOT;
    else if (strcmp(mnemonico, "SHL") == 0)   ir.tipo = INSTR_SHL;
    else if (strcmp(mnemonico, "SHR") == 0)   ir.tipo = INSTR_SHR;
    else if (strcmp(mnemonico, "CMP") == 0)   ir.tipo = INSTR_CMP;
    else if (strcmp(mnemonico, "JMP") == 0)   ir.tipo = INSTR_JMP;
    else if (strcmp(mnemonico, "CALL") == 0)  ir.tipo = INSTR_CALL;
    else if (strcmp(mnemonico, "RET") == 0)   ir.tipo = INSTR_RET;
    else if (strcmp(mnemonico, "JZ") == 0)    ir.tipo = INSTR_JZ;
    else if (strcmp(mnemonico, "JNZ") == 0)   ir.tipo = INSTR_JNZ;
    else if (strcmp(mnemonico, "JC") == 0)    ir.tipo = INSTR_JC;
    else if (strcmp(mnemonico, "JNC") == 0)   ir.tipo = INSTR_JNC;
    else if (strcmp(mnemonico, "JN") == 0)    ir.tipo = INSTR_JN;
    else if (strcmp(mnemonico, "JNN") == 0)   ir.tipo = INSTR_JNN;
    else if (strcmp(mnemonico, "JV") == 0)    ir.tipo = INSTR_JV;
    else if (strcmp(mnemonico, "JNV") == 0)   ir.tipo = INSTR_JNV;
    else if (strcmp(mnemonico, "WAITH") == 0 || strcmp(mnemonico, "WAITV") == 0) {
        ir.tipo = INSTR_WAIT;
        ir.fuente[0].tipo = OPERANDO_INMEDIATO;
        ir.fuente[0].valor = (strcmp(mnemonico, "WAITV") == 0) ? 1 : 0;
        return ir;
    } else {
        reportar_error(0, TOKEN_IDENTIFICADOR, "Mnemónico no reconocido '%s'", mnemonico);
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    ir.formato = obtener_formato_mnemonico(mnemonico);

    int n = 1;

    switch (ir.formato) {
        case FORMATO_SIN_OPERANDOS:
            break;

        case FORMATO_SALTO:
            parsear_formato_salto(&ir, n);
            break;

        case FORMATO_STORE:
            parsear_formato_store(&ir, n);
            break;

        case FORMATO_LOAD:
            parsear_formato_load(&ir, n);
            break;

        case FORMATO_MOVI:
            parsear_formato_mov_inm(&ir, n);
            break;

        case FORMATO_MOV:
            parsear_formato_mov(&ir, n);
            break;
        case FORMATO_ALU_UNARIO:
            parsear_formato_alu_unario(&ir, n);
            break;

        case FORMATO_ALU_BINARIO:
            parsear_formato_binario(&ir, n);
            break;

        default:
            reportar_error(0, TOKEN_IDENTIFICADOR, "Formato de instrucción no soportado.");
            break;
    }

    return ir;
}