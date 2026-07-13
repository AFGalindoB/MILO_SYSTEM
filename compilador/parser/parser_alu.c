#include "parser.h"

void parsear_alu(InstruccionParseada* instr) {
    // 1. Registro Destino (Rd)
    if (match(TOKEN_REGISTRO)) {
        instr->operandos.alu.rd = token_actual.valor;
        avanzar_token();
    } else { consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd)"); }
    
    // 2. Coma obligatoria
    consumir(TOKEN_COMA, "Se esperaba una coma ',' después del registro destino");
    
    // 3. Primer Registro Fuente (Ra)
    if (match(TOKEN_REGISTRO)) {
        instr->operandos.alu.ra = token_actual.valor;
        avanzar_token();
    } else { consumir(TOKEN_REGISTRO, "Se esperaba el primer registro fuente (Ra)"); }
    
    // 4. Tu Regla Especial: Segundo Registro Fuente (Rb) directo sin coma previa
    if (match(TOKEN_REGISTRO)) {
        instr->operandos.alu.rb = token_actual.valor;
        avanzar_token();
    } else { consumir(TOKEN_REGISTRO, "Se esperaba el segundo registro fuente (Rb) separado por espacio"); }
}