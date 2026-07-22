#include "parser.h"
#include <string.h>

void parsear_alu(InstruccionParseada* instr) {
    
    uint8_t update_flags = 0;

    if (match_tipo(1, TOKEN_MODIFICADOR)) {
        if (match_lexema(1, ".F")) {
            update_flags = 1;
        } else {
            reportar_error(1, TOKEN_MODIFICADOR, "Modificador desconocido. Solo se permite '.F' para actualizar flags.");
            return;
        }
    } else {
        update_flags = 0; // No se actualizan flags por defecto
    }

    int es_unaria = (instr->tipo == INSTR_NOT);

    uint8_t n = obtener_destinos_ortogonales(instr);

    if (n == 0) {
        return; // Error ya reportado dentro de obtener_destinos_ortogonales
    }

    if (es_unaria) {
        // ==========================================
        // FLUJO UNARIO (Ej: NOT Rd, Ra)
        // ==========================================
        
        instr->fuentes.alu_unaria.update_flags = update_flags;
        
        // 1. Registro Fuente Único (Ra)
        if (!match_tipo(n, TOKEN_REGISTRO)) {
            reportar_error(n, TOKEN_REGISTRO, "Se esperaba un registro fuente (Ra) para la operación unaria.");
            return;
        }
        instr->fuentes.alu_unaria.ra = linea_tokens[n].valor;

    } else {
        // ==========================================
        // FLUJO BINARIO (Ej: ADD Rd, Ra Rb)
        // ==========================================
        
        instr->fuentes.alu.update_flags = update_flags;
        
        // 1. Primer Registro Fuente (Ra)
        if (!match_tipo(n, TOKEN_REGISTRO)) {
            reportar_error(n, TOKEN_REGISTRO, "Se esperaba el primer registro fuente (Ra) para la operación binaria.");
            return;
        }
        instr->fuentes.alu.ra = linea_tokens[n].valor;
        
        if (!match_tipo(n + 1, TOKEN_COMA)) {
            reportar_error(n + 1, TOKEN_COMA, "Se esperaba una coma ',' después del primer registro fuente (Ra).");
            return;
        }

        if (!match_tipo(n + 2, TOKEN_REGISTRO)) {
            reportar_error(n + 2, TOKEN_REGISTRO, "Se esperaba el segundo registro fuente (Rb) para la operación binaria.");
            return;
        }
        instr->fuentes.alu.rb = linea_tokens[n + 2].valor;
    }
}