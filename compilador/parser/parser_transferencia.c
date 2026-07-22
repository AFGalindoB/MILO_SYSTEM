#include "parser.h"
#include <string.h>

void parsear_movimiento(InstruccionParseada* instr) {
    // ---- CASO: STORE ----
    if (instr->tipo == INSTR_STORE) {
        if (!match_tipo(1, TOKEN_REGISTRO)) {
            reportar_error(1, TOKEN_REGISTRO, "Se esperaba el registro de datos (MDR) después de STORE");
            return;
        }
        instr->fuentes.store.mdr = linea_tokens[1].valor;

        if (!match_tipo(2, TOKEN_COMA)) {
            reportar_error(2, TOKEN_COMA, "Se esperaba una coma ',' después del registro de datos (MDR)");
            return;
        }

        if (!match_tipo(3, TOKEN_CORCHETE_I)) {
            reportar_error(3, TOKEN_CORCHETE_I, "Se esperaba abrir corchete '[' después de la coma");
            return;
        }

        if (!match_tipo(4, TOKEN_REGISTRO)) {
            reportar_error(4, TOKEN_REGISTRO, "Se esperaba el registro de dirección (MAR) después de abrir corchete '['");
            return;
        }
        instr->fuentes.store.mar = linea_tokens[4].valor;

        if (!match_tipo(5, TOKEN_CORCHETE_D)) {
            reportar_error(5, TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']' después del registro de dirección (MAR)");
            return;
        }
        return;
    }

    uint8_t n = obtener_destinos_ortogonales(instr);

    if (n == 0) {
        return; // Error ya reportado dentro de obtener_destinos_ortogonales
    }

    // ---- CASO: MOVI ----
    if (instr->tipo == INSTR_MOVI) {
        if (!match_tipo(n, TOKEN_INMEDIATO)) {
            reportar_error(n, TOKEN_INMEDIATO, "Se esperaba un registro o un inmediato después del destino.");
            return;
        }
        instr->fuentes.movi.valor = linea_tokens[n].valor;
    }

    // ---- CASO: MOV ----
    if (instr->tipo == INSTR_MOV) {
        if (!match_tipo(n, TOKEN_REGISTRO)) {
            reportar_error(n, TOKEN_REGISTRO, "Se esperaba un registro después del destino.");
            return;
        }
        instr->fuentes.mov.rs = linea_tokens[n].valor;
    }

    // ---- CASO: LOAD ----
    if (instr->tipo == INSTR_LOAD) {
        if (!match_tipo(n, TOKEN_CORCHETE_I)) {
            reportar_error(n, TOKEN_CORCHETE_I, "Se esperaba abrir corchete '[' después del destino.");
            return;
        }

        if (!match_tipo(n + 1, TOKEN_REGISTRO)) {
            reportar_error(n + 1, TOKEN_REGISTRO, "Se esperaba un registro de dirección (MAR) después de abrir corchete '['");
            return;
        }
        instr->fuentes.load.mar = linea_tokens[n + 1].valor;

        if (!match_tipo(n + 2, TOKEN_CORCHETE_D)) {
            reportar_error(n + 2, TOKEN_CORCHETE_D, "Se esperaba cerrar corchete ']' después del registro de dirección (MAR)");
            return;
        }
    }

}