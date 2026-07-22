#include "parser.h"
#include <string.h>

void parsear_pc(InstruccionParseada* instr) {
    // =================================================================
    // 1. COMPARACIÓN (CMP R1 R2)
    // =================================================================

    instr->rd = 0; 
    instr->tiene_rd = 0;
    instr->reg_gpu = 0;

    if (instr->tipo == INSTR_CMP) {
        // CMP es una resta lógica que actualiza banderas. 
        // No guarda el resultado (rd = 0) pero obliga a activar flags.
        instr->fuentes.alu.update_flags = 1;

        if (!match_tipo(1, TOKEN_REGISTRO)) {
            reportar_error(1, TOKEN_REGISTRO, "Se esperaba el primer registro para la comparación (CMP)");
            return;
        }
        instr->fuentes.alu.ra = linea_tokens[1].valor;

        if (!match_tipo(2, TOKEN_COMA)) {
            reportar_error(2, TOKEN_COMA, "Se esperaba una coma ',' entre los registros de comparación (CMP)");
            return;
        }

        // CMP no requiere coma intermedia entre registros
        if (!match_tipo(3, TOKEN_REGISTRO)) {
            reportar_error(3, TOKEN_REGISTRO, "Se esperaba el segundo registro para la comparación (CMP)");
            return;
        }
        instr->fuentes.alu.rb = linea_tokens[3].valor;
        return;
    }

    // =================================================================
    // 2. SALTOS (JMP, CALL, JZ, JNZ, JC, JNC, JN, JNN, JV, JNV)
    // =================================================================
    // Si no es CMP ni RET, obligatoriamente es un salto que requiere una dirección inmediata (#)
    if (!match_tipo(1, TOKEN_INMEDIATO)) {
        reportar_error(1, TOKEN_INMEDIATO, "Se esperaba un valor inmediato (etiqueta o dirección) para el salto.");
        return;
    }
    instr->fuentes.salto.destino = linea_tokens[1].valor;
}