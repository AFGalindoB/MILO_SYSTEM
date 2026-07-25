#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"

#define PROBAR_ERROR_SINTACTICO false

// Helper estático para obtener el nombre legible del tipo de instrucción
static const char* obtener_nombre_instruccion(TipoInstruccion tipo) {
    switch (tipo) {
        case INSTR_NOP:   return "NOP";
        case INSTR_MOV:   return "MOV";
        case INSTR_MOVI:  return "MOVI";
        case INSTR_LOAD:  return "LOAD";
        case INSTR_STORE: return "STORE";
        case INSTR_ADD:   return "ADD";
        case INSTR_SUB:   return "SUB";
        case INSTR_ADC:   return "ADC";
        case INSTR_SBC:   return "SBC";
        case INSTR_AND:   return "AND";
        case INSTR_OR:    return "OR";
        case INSTR_XOR:   return "XOR";
        case INSTR_NOT:   return "NOT";
        case INSTR_SHL:   return "SHL";
        case INSTR_SHR:   return "SHR";

        // --- Control de Flujo ---
        case INSTR_CMP:   return "CMP";
        case INSTR_JMP:   return "JMP";
        case INSTR_CALL:  return "CALL";
        case INSTR_RET:   return "RET";
        case INSTR_JZ:    return "JZ";
        case INSTR_JNZ:   return "JNZ";
        case INSTR_JC:    return "JC";
        case INSTR_JNC:   return "JNC";
        case INSTR_JN:    return "JN";
        case INSTR_JNN:   return "JNN";
        case INSTR_JV:    return "JV";
        case INSTR_JNV:   return "JNV";
        case INSTR_WAIT:  return "WAIT";

        default:          return "DESCONOCIDA";
    }
}

// Auxiliar para formatear un operando individual en texto legible
static void imprimir_operando(Operando op) {
    switch (op.tipo) {
        case OPERANDO_REG_RW:
            printf("R%d", op.valor);
            break;
        case OPERANDO_REG_WO:
            if (op.valor == 1) printf("TBUF");
            else if (op.valor == 2) printf("SCROLL");
            else printf("WO#%d", op.valor);
            break;
        case OPERANDO_REG_RO:
            if (op.valor == 1) printf("RINPT");
            else printf("RO#%d", op.valor);
            break;
        case OPERANDO_INMEDIATO:
            printf("#%d", op.valor);
            break;
        case OPERANDO_NULO:
            printf("NONE");
            break;
    }
}

// Función auxiliar para desempaquetar e imprimir de forma genérica
static void desempaquetar_e_imprimir(InstruccionIR* instr) {
    const char* nombre = obtener_nombre_instruccion(instr->tipo);
    printf("[Línea %02d] Parsed: %-5s | ", instr->linea, nombre);

    // ====================================================
    // 1. IMPRESIÓN DE DESTINOS (Ortogonalidad)
    // ====================================================
    bool tiene_destino = (instr->destino[0].tipo != OPERANDO_NULO || 
                          instr->destino[1].tipo != OPERANDO_NULO);

    if (tiene_destino) {
        printf("Destinos: [");
        bool primero = true;
        for (int i = 0; i < 2; i++) {
            if (instr->destino[i].tipo != OPERANDO_NULO) {
                if (!primero) printf(", ");
                imprimir_operando(instr->destino[i]);
                primero = false;
            }
        }
        printf("] | ");
    }

    // ====================================================
    // 2. IMPRESIÓN DE OPERANDOS FUENTE Y ATRIBUTOS
    // ====================================================
    printf("Fuentes: [");
    bool primera_fuente = true;
    for (int i = 0; i < 2; i++) {
        if (instr->fuente[i].tipo != OPERANDO_NULO) {
            if (!primera_fuente) printf(", ");
            imprimir_operando(instr->fuente[i]);
            primera_fuente = false;
        }
    }
    printf("]");

    // Mostrar modificador de banderas (.F) si la instrucción lo posee
    if (instr->tiene_modificador_f) {
        printf(" | UpdateFlags: .F");
    }

    printf("\n");
}

int main() {
    // 1. Único código de prueba que contiene las nuevas incorporaciones (ADC, SBC, modificadores .F)
    const char* codigo_prueba = 
    #if !PROBAR_ERROR_SINTACTICO
        "; ---- Bloque de Pruebas Exitosas ----\n"
        "NOP\n"
        "MOVI R1, #25                ; MOVI estándar\n"
        "MOVI TBUF, #100           ; MOVI ortogonal directo a registro especial\n"
        "MOVI TBUF, R1, #50          ; MOVI ortogonal con Rd + Reg Especial\n"
        "MOV R2, R1                  ; MOV estándar\n"
        "MOV TBUF, R2                ; MOV ortogonal directo a registro especial\n"
        "MOV TBUF, R3, R2            ; MOV ortogonal con Rd + Reg Especial\n"
        "ADD.F R3, R1, R2            ; ALU Binaria estándar con .F y comas\n"
        "ADD.F TBUF, R3, R1, R2      ; ALU Binaria ortogonal (Especial + Rd + Rs1 + Rs2)\n"
        "SUB R0, R3, R1              ; ALU Binaria sin modificar banderas\n"
        "ADC.F R4, R2, R3            ; Suma con acarreo\n"
        "SBC R5, R4, R1              ; Resta con acarreo\n"
        "NOT.F R6, R2                ; ALU Unaria estándar\n"
        "NOT.F TBUF, R6, R2          ; ALU Unaria ortogonal (Especial + Rd + Rs)\n"
        "NOT SCROLL, R2              ; ALU Unaria ortogonal directo a especial (Especial + Rs)\n"
        "LOAD R8, [R1]               ; LOAD estándar\n"
        "LOAD TBUF, R8, [R1]         ; LOAD ortogonal con Rd + Especial\n"
        "LOAD SCROLL, [R1]           ; LOAD ortogonal directo a especial\n"
        "STORE R8, [R5]              ; STORE estándar\n"
        "CMP R1, R2                  ; Comparación básica con coma\n"
        "JZ #16                      ; Salto si Z=1\n"
        "CALL #128                 ; Llamada a subrutina\n"
        "RET                         ; Retorno de subrutina\n"
        "ADD.F SCROLL, R1, R2        ; Suma con actualizador de banderas y guardar resultado en GPU\n"
        "JNZ #8                      ; Salto condicional si Z=0\n"
        "MOV R0, RINPT               ; Pooling a mando\n"
        ;
    #else
        "; ---- Bloque de Pruebas con Errores Sintácticos ----\n"
        "ADD R1 R2 R3         ; Error 1: Falta coma obligatoria\n"
        "MOV R2, R1 R3        ; Error 2: Solo acepta un registro destino y un registro fuente\n"
        "MOVI R2,             ; Error 3: Inmediato vacío\n"
        "STORE R3, R4         ; Error 4: Store sin corchetes\n"
        "LOAD R1, R2          ; Error 5: LOAD sin corchetes\n"
        "INVENTADO R1, R2     ; Error 6: Mnemónico ilegal\n"
        "NOT R5, R6 R7        ; Error 7: NOT no acepta segundo operando fuente\n"
        "ADD.F.G R8, R1 R2    ; Error 8: Doble modificador inválido\n"
        "ADD.X R1, R2 R3      ; Error 9: Modificador inexistente\n"
        ".                    ; Error 10: Modificador invalido\n"
        "CMP R1               ; Error 11: CMP requiere dos operandos\n"
        "JMP                  ; Error 12: JMP requiere un valor inmediato\n"
        "JZ R1                ; Error 13: JZ requiere un inmediato, no un registro\n"
        "RET R1               ; Error 14: RET no acepta parámetros\n"
        "MOV TBUF,            ; Error 15: MOV con registro especial sin operando fuente\n"
        "LOAD SCROLL,         ; Error 16: LOAD con registro especial sin operando fuente\n"
        "ADD SCROLL, R1,      ; Error 17: ADD con registro especial sin segundo operando fuente\n"
        "SBC.F TBUF, R1,      ; Error 18: SBC con registro especial sin segundo operando fuente\n"
        "LOAD R1, [RINPT]     ; Error 19: RAM no puede leer de RINPT\n"
        ;
    #endif

    printf("====================================================================\n");
    printf("   🔥 EJECUTANDO TESTBENCH DEL PARSER - MILO ASM 🔥        \n");
    printf("====================================================================\n\n");

    // Inicializar el lexer con el código fuente en memoria
    inicializar_lexer(codigo_prueba);

    Token tokens_linea[MAX_TOKENS_POR_LINEA];
    int cantidad_tokens = 0;
    int lineas_procesadas = 0;
    int es_eof = 0;

    // Ciclo principal: Extrae líneas de tokens completas y las envía al parser
    while (!es_eof) {
        cantidad_tokens = obtener_linea_de_tokens(tokens_linea);

        if (cantidad_tokens == 0) {
            break;
        }

        // Verificar si la línea termina en EOF para salir del bucle al terminar de parsear
        if (tokens_linea[cantidad_tokens - 1].tipo == TOKEN_EOF) {
            es_eof = 1;
        }

        InstruccionIR instr = parsear_linea_tokens(tokens_linea, cantidad_tokens);

        // Si la línea tenía contenido válido (no era comentario ni línea vacía)
        if (instr.tipo != INSTR_DESCONOCIDA) {
            lineas_procesadas++;
            desempaquetar_e_imprimir(&instr);
        }
    }

    // Emitir el reporte gráfico de errores
    emitir_informe_compilacion();

    printf("\n====================================================================\n");
    printf(" Análisis sintáctico completado. %d instrucciones procesadas.\n", lineas_procesadas);
    printf("====================================================================\n");

    return (contador_errores > 0) ? 1 : 0;
}