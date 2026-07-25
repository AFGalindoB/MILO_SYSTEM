#ifndef ENCODER_H
#define ENCODER_H

#pragma once
#include "../parser/parser.h"

// Definición de OpCodes físicos de Milo Alpha
#define OPCODE_NOP   0x00
#define OPCODE_ALU   0x01
#define OPCODE_MOV   0x02
#define OPCODE_PC    0x03
#define OPCODE_STOP  0x04

// Bus C Source Selector
#define BUS_C_ALU       0x00
#define BUS_C_REGSRC1   0x01
#define BUS_C_RAM       0x02
#define BUS_C_ROM_INM   0x03
#define BUS_C_RINPT     0x04

// Operaciones de ALU (ALU_OP: bits 6-4 de Fine Control)
#define ALU_OP_ADD  0x00
#define ALU_OP_SUB  0x01
#define ALU_OP_AND  0x02
#define ALU_OP_OR   0x03
#define ALU_OP_XOR  0x04
#define ALU_OP_NOT  0x05
#define ALU_OP_SHL  0x06
#define ALU_OP_SHR  0x07

// Flags para saltos (SEL_FLAG: bits 8-7 de Fine Control en PC)
#define FLAG_N  0x00
#define FLAG_Z  0x01
#define FLAG_C  0x02
#define FLAG_V  0x03


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
int codificar_instruccion(InstruccionIR* instr, PalabraROM* salida);

#endif // ENCODER_H