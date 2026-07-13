#include "parser.h"
#include <string.h>

void parsear_movimiento(InstruccionParseada* instr) {
    // ---- CASO: LOAD ----
    if (instr->tipo == INSTR_LOAD) {
        if (match(TOKEN_REGISTRO)) { instr->operandos.load.rd = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd)"); }
        
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después de Rd");
        consumir(TOKEN_CORCHETE_I, "Se esperaba abrir corchete '['");
        
        if (match(TOKEN_REGISTRO)) { instr->operandos.load.mar = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro de dirección (MAR)"); }
        
        consumir(TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']'");
    }
    // ---- CASO: STORE ----
    else if (instr->tipo == INSTR_STORE) {
        if (match(TOKEN_REGISTRO)) { instr->operandos.store.mdr = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro de datos (MDR)"); }
        
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después de MDR");
        consumir(TOKEN_CORCHETE_I, "Se esperaba abrir corchete '['");
        
        if (match(TOKEN_REGISTRO)) { instr->operandos.store.mar = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro de dirección (MAR)"); }
        
        consumir(TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']'");
    }
    // ---- CASO: MOV REGISTRO A REGISTRO ----
    else if (instr->tipo == INSTR_MOV) {
        if (match(TOKEN_REGISTRO)) { instr->operandos.mov.rd = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd)"); }
        
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después de Rd");
        
        if (match(TOKEN_REGISTRO)) {
            instr->operandos.mov.rs = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_REGISTRO, "Se esperaba un registro fuente (Rs) para la instrucción MOV");
        }
    }
    // ---- CASO: MOVI REGISTRO E INMEDIATO ----
    else if (instr->tipo == INSTR_MOVI) {
        if (match(TOKEN_REGISTRO)) { instr->operandos.movi.rd = token_actual.valor; avanzar_token(); }
        else { consumir(TOKEN_REGISTRO, "Se esperaba el registro destino (Rd)"); }
        
        consumir(TOKEN_COMA, "Se esperaba una coma ',' después de Rd");
        
        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.movi.valor = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para la instrucción MOVI");
        }
    }
}