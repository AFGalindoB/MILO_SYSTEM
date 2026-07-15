#include "parser.h"

void parsear_alu(InstruccionParseada* instr) {
    
    int es_unaria = (instr->tipo == INSTR_NOT);

    if (es_unaria) {
        // ==========================================
        // FLUJO UNARIO (Ej: NOT Rd, Ra)
        // ==========================================
        
        // 1. Registro Destino (Rd)
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu_unaria.rd = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd) para la operación unaria"); 
        }
        
        // 2. Coma obligatoria
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después del registro destino");
        
        // 3. Registro Fuente Único (Ra)
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu_unaria.ra = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el registro fuente (Ra) para la operación unaria"); 
        }

    } else {
        // ==========================================
        // FLUJO BINARIO (Ej: ADD Rd, Ra Rb)
        // ==========================================
        
        // 1. Registro Destino (Rd)
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu.rd = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd)"); 
        }
        
        // 2. Coma obligatoria
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después del registro destino");
        
        // 3. Primer Registro Fuente (Ra)
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu.ra = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el primer registro fuente (Ra)"); 
        }
        
        // 4. Segundo Registro Fuente (Rb) - Directo sin coma intermedia
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.alu.rb = token_actual.valor;
            avanzar_token();
        } else { 
            consumir(TOKEN_REGISTRO, "Se esperaba el segundo registro fuente (Rb) separado por espacio"); 
        }
    }
}