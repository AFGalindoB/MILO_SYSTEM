#ifndef ENCODER_H
#define ENCODER_H

#pragma once
#include "../parser/parser.h"

// Mapeo de operaciones ALU
#define ALU_OP_ADD 0
#define ALU_OP_SUB 1
#define ALU_OP_AND 2
#define ALU_OP_OR  3
#define ALU_OP_XOR 4
#define ALU_OP_NOT 5
#define ALU_OP_SHL 6
#define ALU_OP_SHR 7

// Mapeo de banderas
#define FLAG_N 0
#define FLAG_Z 1
#define FLAG_C 2
#define FLAG_V 3


// Estructura que representa la palabra final de la ROM
typedef struct {
    uint32_t control_word; // Bits 63:32
    uint32_t immediate;    // Bits 31:0
} PalabraROM;

/**
 * Codifica una instrucción parseada.
 * Llena el buffer 'salida' con las palabras ROM físicas resultantes.
 * Retorna la cantidad de microinstrucciones generadas (normalmente 1, o 2 para RET).
 */
int codificar_instruccion(InstruccionParseada* instr, PalabraROM* salida);

#endif // ENCODER_H