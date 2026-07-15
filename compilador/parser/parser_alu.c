#include "parser.h"
#include <string.h>

void parsear_alu(InstruccionParseada* instr) {
    
    uint8_t update_flags = 0;

    if (match(TOKEN_MODIFICADOR)) {
        if (strcmp(token_actual.lexema, ".F") == 0) {
            update_flags = 1;
            avanzar_token(); // Consumir el modificador ".F"
        } else {
            // Error sintáctico controlado si viene un modificador inesperado en la ALU
            consumir(TOKEN_MODIFICADOR, "El único modificador permitido para operaciones de la ALU es '.F'");
        }
    }

    int es_unaria = (instr->tipo == INSTR_NOT);

    if (es_unaria) {
        // ==========================================
        // FLUJO UNARIO (Ej: NOT Rd, Ra)
        // ==========================================
        
        instr->operandos.alu_unaria.update_flags = update_flags;

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
        
        instr->operandos.alu.update_flags = update_flags;

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