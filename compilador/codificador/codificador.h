#ifndef CODIFICADOR_H
#define CODIFICADOR_H

#include "../parser/parser.h"
#include <stdint.h>

// Definición de operaciones de la ALU (Mapeado a los bits 2-0 de Fine Control)
#define ALU_OP_ADD 0
#define ALU_OP_SUB 1
#define ALU_OP_AND 2
#define ALU_OP_OR  3
#define ALU_OP_XOR 4
#define ALU_OP_NOT 5
#define ALU_OP_SHL 6
#define ALU_OP_SHR 7

// Estructura que representa la palabra final de 64 bits de la ROM
typedef struct {
    uint32_t control_word; // Bits 63:32
    uint32_t immediate;    // Bits 31:0
} PalabraROM;

// Función principal del codificador
PalabraROM codificar_instruccion(InstruccionParseada* instr);

#endif // CODIFICADOR_H