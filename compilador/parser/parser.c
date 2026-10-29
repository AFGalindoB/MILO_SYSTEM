#include "parser.h"
#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../helpers/tag_manager.h"

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

static int comprobar_exceso_token(TipoToken tipo_buscado) {
    int contador = 0;
    
    for (int i = 0; i < total_tokens; i++) {
        if (linea_tokens[i].tipo == tipo_buscado) {
            contador++;
            if (contador > 1) {
                return 1; // Se encontró más de 1
            }
        }
    }
    
    return 0; // Cantidad permitida (0 o 1)
}

static Operando consumir_operando(InstruccionIR* ir, int* n) {
    Operando op;
    op.tipo = OPERANDO_NULO;
    op.valor = 0;

    if (*n >= total_tokens){
        reportar_error(*n, TOKEN_EOF, "Se esperaba un operando (Registro, Inmediato o RAM) pero se alcanzó el final de la línea.");
        return op;
    } 

    Token t = linea_tokens[*n];

    // CASO 1: Registro directo (R0 - R15)
    if (t.tipo == TOKEN_REGISTRO) {
        op.tipo = OPERANDO_REG;
        op.valor = t.valor;
        (*n)++;
        return op;
    }

    // CASO 2: Inmediato directo (#n)
    if (t.tipo == TOKEN_INMEDIATO) {
        if (ir->inmediato.tipo == OPERANDO_INMEDIATO && ir->inmediato.valor != t.valor) {
            reportar_error(*n, TOKEN_INMEDIATO, "Violación de ISA: No se permite declarar diferentes valores inmediatos (#n) en la misma instrucción.");
            return op;
        }
        op.tipo = OPERANDO_INMEDIATO;
        ir->inmediato.tipo = OPERANDO_INMEDIATO;
        ir->inmediato.valor = t.valor;
        (*n)++;
        return op;
    }

    // CASO 3: Lexema 'RAM' aislado (Requiere MAR previamente establecido por un [...])
    if (t.tipo == TOKEN_RAM) {
        if (ir->MAR_Control.tipo == OPERANDO_NULO) {
            reportar_error(*n, TOKEN_RAM, "Uso ilegal de 'RAM': No se puede usar 'RAM' como operando sin haber establecido la dirección previamente mediante '[...]'");
            return op;
        }

        op.tipo = OPERANDO_RAM;
        (*n)++;
        return op;
    }

    // CASO 4: Direccionamiento explícito a RAM con corchetes [...]
    if (t.tipo == TOKEN_CORCHETE_I) {
        (*n)++; // Consumir '['

        if (*n >= total_tokens) {
            reportar_error(*n, TOKEN_CORCHETE_I, "Se esperaba un registro o inmediato dentro de los corchetes '[...]'");
            return op;
        }

        if (match_tipo(*n, TOKEN_REGISTRO)) {
            if (linea_tokens[*n].valor > 7) {
                reportar_error(*n, TOKEN_REGISTRO, "Registro inválido dentro de '[...]'. Solo se permiten R0 a R7.");
                return op;
            }
            ir->MAR_Control.tipo = OPERANDO_REG;
            ir->MAR_Control.valor = linea_tokens[*n].valor;
            (*n)++;
        } 
        else if (match_tipo(*n, TOKEN_INMEDIATO)) {
            if (ir->inmediato.tipo == OPERANDO_INMEDIATO && ir->inmediato.valor != t.valor) {
                reportar_error(*n, TOKEN_INMEDIATO, "Violación de ISA: No se permite declarar diferentes valores inmediatos (#n) en la misma instrucción.");
                return op;
            }
            ir->MAR_Control.tipo = OPERANDO_INMEDIATO;
            ir->inmediato.tipo = OPERANDO_INMEDIATO;
            ir->inmediato.valor = linea_tokens[*n].valor;
            (*n)++;
        } 
        else {
            reportar_error(*n, linea_tokens[*n].tipo, "Dirección de memoria inválida. Se esperaba Rn o #n dentro de '[...]'");
            return op;
        }

        // Cierre obligatorio de corchete
        if (*n >= total_tokens || linea_tokens[*n].tipo != TOKEN_CORCHETE_D) {
            reportar_error(*n, TOKEN_CORCHETE_D, "Se esperaba ']' al cerrar la referencia de dirección de memoria.");
            return op;
        }

        (*n)++; // Consumir ']'

        // Un direccionamiento [...] como operando equivale a acceder a la celda de memoria RAM
        op.tipo = OPERANDO_RAM;
        op.valor = 0;
        return op;
    }

    // Si el token no calza con ninguna forma válida de operando
    reportar_error(*n, t.tipo, "Operando no reconocido o sintaxis inválida ('%s')", t.lexema);
    return op;
}

static FormatoInstruccion obtener_formato_mnemonico(const char* lexema) {
    if (strcmp(lexema, "NOP") == 0 || strcmp(lexema, "RET") == 0)
        return FORMATO_SIN_OPERANDOS;
    
    if (strcmp(lexema, "JMP") == 0 || strcmp(lexema, "JZ") == 0  || strcmp(lexema, "JNZ") == 0 ||
        strcmp(lexema, "JC") == 0  || strcmp(lexema, "JNC") == 0 || strcmp(lexema, "JN") == 0  ||
        strcmp(lexema, "JNN") == 0 || strcmp(lexema, "JV") == 0  || strcmp(lexema, "JNV") == 0 ||
        strcmp(lexema, "CALL") == 0)
        return FORMATO_SALTO;

    if (strcmp(lexema, "MOV") == 0)   return FORMATO_MOV;
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
        emitir_informe_compilacion();
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

static void parsear_formato_salto(InstruccionIR* ir, int* n) {
    if (match_tipo(*n, TOKEN_IDENTIFICADOR)) {
        int destino = buscar_etiqueta(linea_tokens[*n].lexema);
        if (destino != -1) {
            linea_tokens[*n].tipo = TOKEN_INMEDIATO;
            linea_tokens[*n].valor = destino;
        }
    }
    if (!match_tipo(*n, TOKEN_INMEDIATO)) {
        reportar_error(*n, TOKEN_INMEDIATO, "Se esperaba una dirección o etiqueta para la instrucción de salto.");
        return;
    }
    // Consumir el inmediato y asignarlo al campo inmediato de la instrucción
    consumir_operando(ir, n);
}

static void parsear_formato_alu_unario(InstruccionIR* ir, int* n) {
    if (match_tipo(*n, TOKEN_MODIFICADOR)) {
        if (match_lexema(*n, ".F")){
            ir->tiene_modificador_f = 1;
            (*n)++;
        }else{
            reportar_error(*n, TOKEN_MODIFICADOR, "Modificador desconocido. Solo se permite '.F' para actualizar flags.");
            return;
        }
    }

    ir->destino = consumir_operando(ir, n);
    if (ir->destino.tipo == OPERANDO_NULO) return;
    
    if (!match_tipo(*n, TOKEN_COMA)) {
        reportar_error(*n, TOKEN_COMA, "Se esperaba ',' entre el destino y la fuente.");
        return;
    }
    (*n)++; // Consumir ','

    ir->fuente[0] = consumir_operando(ir, n);
    if (ir->fuente[0].tipo == OPERANDO_NULO) return;
}

static void parsear_formato_mov(InstruccionIR* ir, int* n) {
    ir->destino = consumir_operando(ir, n);
    if (ir->destino.tipo == OPERANDO_NULO) return;
    
    if (!match_tipo(*n, TOKEN_COMA)) {
        reportar_error(*n, TOKEN_COMA, "Se esperaba ',' entre el destino y la fuente en MOV.");
        return;
    }
    (*n)++; // Consumir ','

    ir->fuente[0] = consumir_operando(ir, n);
    if (ir->fuente[0].tipo == OPERANDO_NULO) return;
}

static void parsear_formato_alu_binario(InstruccionIR* ir, int* n) {
    // Si la instrucción es CMP no escribe en destinos, va directo a fuentes
    if (match_tipo(*n, TOKEN_MODIFICADOR)) {
        if (match_lexema(*n, ".F")){
            ir->tiene_modificador_f = 1;
            (*n)++;
        }else{
            reportar_error(*n, TOKEN_MODIFICADOR, "Modificador desconocido. Solo se permite '.F' para actualizar flags.");
            return;
        }
    }

    if (ir->tipo == INSTR_CMP) {
        ir->tiene_modificador_f = 1;
        ir->destino.tipo = OPERANDO_NULO;
    } else {
        ir->destino = consumir_operando(ir, n);
        if (ir->destino.tipo == OPERANDO_NULO) return;

        if (!match_tipo(*n, TOKEN_COMA)) {
            reportar_error(*n, TOKEN_COMA, "Se esperaba ',' tras el destino.");
            return;
        }
        (*n)++; // Consumir ','
    }

    ir->fuente[0] = consumir_operando(ir, n);
    if (ir->fuente[0].tipo == OPERANDO_NULO) return;

    if (!match_tipo(*n, TOKEN_COMA)) {
        reportar_error(*n, TOKEN_COMA, "Se esperaba ',' entre los operandos fuente.");
        return;
    }
    (*n)++; // Consumir ','

    ir->fuente[1] = consumir_operando(ir, n);
    if (ir->fuente[1].tipo == OPERANDO_NULO) return;
}

InstruccionIR parsear_linea_tokens(Token* tokens, int cantidad_tokens) {
    InstruccionIR ir;
    memset(&ir, 0, sizeof(InstruccionIR));
    uint32_t errores_antes = contador_errores;

    linea_tokens = tokens;
    total_tokens = cantidad_tokens;

    ir.linea = linea_tokens[0].linea;

    // Inicialización explícita del control de MAR como nulo
    ir.MAR_Control.tipo = OPERANDO_NULO;
    ir.MAR_Control.valor = 0;
    ir.tiene_modificador_f = 0;
    ir.inmediato.tipo = OPERANDO_NULO;

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

    // Ignorar líneas que solo contienen una etiqueta (ej. "inicio:")
    if (total_tokens >= 2 && match_tipo(0, TOKEN_IDENTIFICADOR) && match_tipo(1, TOKEN_DOSPUNTOS)) {
        if (total_tokens != 2) {
            reportar_error(2, linea_tokens[2].tipo, "Tokens adicionales tras la declaración de una etiqueta.");
        }
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    if (comprobar_exceso_token(TOKEN_CORCHETE_I)) {
        reportar_error(0, TOKEN_CORCHETE_I, "Violación de ISA: Solo se permite una referencia de dirección RAM '[...]' por instrucción.");
        ir.tipo = INSTR_DESCONOCIDA;
        return ir;
    }

    const char* mnemonico = linea_tokens[0].lexema;

    if      (strcmp(mnemonico, "NOP") == 0)   ir.tipo = INSTR_NOP;
    else if (strcmp(mnemonico, "MOV") == 0)   ir.tipo = INSTR_MOV;
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
    else {
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
            parsear_formato_salto(&ir, &n);
            break;

        case FORMATO_MOV:
            parsear_formato_mov(&ir, &n);
            break;

        case FORMATO_ALU_UNARIO:
            parsear_formato_alu_unario(&ir, &n);
            break;

        case FORMATO_ALU_BINARIO:
            parsear_formato_alu_binario(&ir, &n);
            break;

        default:
            reportar_error(0, TOKEN_IDENTIFICADOR, "Formato de instrucción no soportado.");
            break;
    }

    if (errores_antes == contador_errores && n < total_tokens && !match_tipo(n, TOKEN_EOF)) {
        reportar_error(n, linea_tokens[n].tipo, "Sintaxis inválida: Operacion realizada (%s) con Tokens sobrantes al final de la línea ('%s').", linea_tokens[0].lexema, linea_tokens[n].lexema);
    }

    return ir;
}