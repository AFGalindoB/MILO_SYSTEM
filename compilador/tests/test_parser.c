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

        default:          return "DESCONOCIDA";
    }
}

// Función auxiliar para desempaquetar e imprimir de forma genérica
static void desempaquetar_e_imprimir(InstruccionParseada* instr) {
    const char* nombre = obtener_nombre_instruccion(instr->tipo);
    printf("[Línea %02d] Parsed: %-5s | ", instr->linea, nombre);

    // ====================================================
    // 1. IMPRESIÓN DE DESTINOS (Ortogonalidad)
    // ====================================================
    // Imprimimos la capa superior de destinos si existen
    if (instr->reg_gpu || instr->tiene_rd) {
        printf("Destinos: [");
        if (instr->reg_gpu) {
            printf("GPU: Reg#%d", instr->reg_gpu);
            if (instr->tiene_rd) printf(", ");
        }
        if (instr->tiene_rd) {
            printf("Rd: R%d", instr->rd);
        }
        printf("] | ");
    }

    // ====================================================
    // 2. IMPRESIÓN DE OPERANDOS FUENTE
    // ====================================================
    switch (instr->tipo) {
        case INSTR_NOP:
            printf("Sin operandos\n");
            break;

        case INSTR_RET:
            printf("Categoría: SUBRUTINA    | Retorno de función (RET)\n");
            break;

        // ALU Binarias (Ra, Rb) + update_flags
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_ADC:
        case INSTR_SBC:
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_XOR:
        case INSTR_SHL:
        case INSTR_SHR:
            printf("Categoría: ALU_BINARIA | Ra: R%d, Rb: R%d | UpdateFlags: %d\n",
                   instr->fuentes.alu.ra,
                   instr->fuentes.alu.rb,
                   instr->fuentes.alu.update_flags);
            break;

        // ALU Unarias (Ra) + update_flags
        case INSTR_NOT:
            printf("Categoría: ALU_UNARIA  | Ra: R%d | UpdateFlags: %d\n",
                   instr->fuentes.alu_unaria.ra,
                   instr->fuentes.alu_unaria.update_flags);
            break;

        // Movimiento simple (Rs)
        case INSTR_MOV:
            printf("Categoría: MOVIMIENTO  | Rs: R%d\n",
                   instr->fuentes.mov.rs);
            break;

        // Movimiento inmediato (valor)
        case INSTR_MOVI:
            printf("Categoría: MOV_IMMED   | Inmediato: #%u\n",
                   instr->fuentes.movi.valor);
            break;

        // Carga de memoria ([mar])
        case INSTR_LOAD:
            printf("Categoría: MEM_LOAD    | MAR: [R%d]\n",
                   instr->fuentes.load.mar);
            break;

        // Escritura en memoria (mdr, [mar])
        case INSTR_STORE:
            printf("Categoría: MEM_STORE   | MDR: R%d, MAR: [R%d]\n",
                   instr->fuentes.store.mdr,
                   instr->fuentes.store.mar);
            break;

        case INSTR_CMP:
            printf("Categoría: COMPARE     | Ra: R%d, Rb: R%d | (Evalúa SUB R0, Ra, Rb | UpdateFlags: 1)\n",
                   instr->fuentes.alu.ra,
                   instr->fuentes.alu.rb);
            break;
        
        case INSTR_JMP:
        case INSTR_CALL:
        case INSTR_JZ:
        case INSTR_JNZ:
        case INSTR_JC:
        case INSTR_JNC:
        case INSTR_JN:
        case INSTR_JNN:
        case INSTR_JV:
        case INSTR_JNV:
            printf("Categoría: CONTROL_FLG | Destino PC: #%u\n",
                   instr->fuentes.salto.destino);
            break;

        default:
            printf("Estructura vacía o inválida\n");
            break;
    }
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
        "LOAD SCROLL, [R1]            ; LOAD ortogonal directo a especial\n"
        "STORE R8, [R5]              ; STORE estándar\n"
        "CMP R1, R2                  ; Comparación básica con coma\n"
        "JZ #16                      ; Salto si Z=1\n"
        "CALL #128                 ; Llamada a subrutina\n"
        "RET                         ; Retorno de subrutina\n"
        "ADD.F SCROLL, R1, R2        ; Suma con actualizador de banderas y guardar resultado en GPU\n"
        "JNZ #8                      ; Salto condicional si Z=0\n";
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
        "ADD SCROLL, R1,       ; Error 17: ADD con registro especial sin segundo operando fuente\n"
        "SBC.F TBUF, R1,     ; Error 18: SBC con registro especial sin segundo operando fuente\n";
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

        InstruccionParseada instr = parsear_linea_tokens(tokens_linea, cantidad_tokens);

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