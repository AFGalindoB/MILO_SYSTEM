#include "parser.h"
#include <string.h>

void parsear_pc(InstruccionParseada* instr) {
    // =================================================================
    // 1. COMPARACIÓN (CMP R1 R2)
    // =================================================================
    if (instr->tipo == INSTR_CMP) {
        // CMP es una resta lógica que actualiza banderas. 
        // No guarda el resultado (rd = 0) pero obliga a activar flags.
        instr->operandos.alu.rd = 0; 
        instr->operandos.alu.update_flags = 1;

        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu.ra = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el primer registro para la comparación (CMP)"); 
        }

        // CMP no requiere coma intermedia entre registros
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu.rb = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el segundo registro para la comparación (CMP)"); 
        }
        return;
    }

    // =================================================================
    // 2. SALTOS (JMP, CALL, JZ, JNZ, JC, JNC, JN, JNN, JV, JNV)
    // =================================================================
    // Si no es CMP ni RET, obligatoriamente es un salto que requiere una dirección inmediata (#)
    if (match(TOKEN_INMEDIATO)) {
        instr->operandos.salto.destino = token_actual.valor;
        avanzar_token();
    } else {
        consumir(TOKEN_INMEDIATO, "Se esperaba una dirección o valor inmediato (#) como destino de salto");
    }
}