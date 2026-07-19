#include "parser.h"
#include <string.h>

void parsear_gpu(InstruccionParseada* instr) {
    // ---- CASO: TBUF ----
    if (instr->tipo == INSTR_GPU_TBUF) {
        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_tbuf.addr = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para la dirección en TBUF");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_tbuf.tile = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para el índice de tile en TBUF");
        }
    }
    // ---- CASO: PIXOFF ----
    if (instr->tipo == INSTR_GPU_PXOFF) {
        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_off.x = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para X en PIXOFF");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_off.y = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para Y en PIXOFF");
        }
    }
    // ---- CASO: TILEOFF ----
    if (instr->tipo == INSTR_GPU_TLOFF) {
        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_off.x = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para X en TILEOFF");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_off.y = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para Y en TILEOFF");
        }
    }
    // ---- CASO: SCROLL ----
    if (instr->tipo == INSTR_GPU_SCROLL) {
        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_scroll.tx = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para TX en SCROLL");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_scroll.ty = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para TY en SCROLL");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_scroll.px = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para PX en SCROLL");
        }

        if (match(TOKEN_INMEDIATO)) {
            instr->operandos.gpu_scroll.py = token_actual.valor;
            avanzar_token();
        } else {
            consumir(TOKEN_INMEDIATO, "Se esperaba un valor inmediato (#) para PY en SCROLL");
        }
    }
}