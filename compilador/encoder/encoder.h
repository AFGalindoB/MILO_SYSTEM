#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "../parser/parser.h"

// OpCodes físicos de Milo Alpha (4 bits)
#define OPCODE_NOP   0x00
#define OPCODE_ALU   0x01
#define OPCODE_MOV   0x02
#define OPCODE_PC    0x03

// Fuentes Bus A (2 bits)
#define BUS_A_REG    0x00
#define BUS_A_RAM    0x01
#define BUS_A_INM    0x02

// Fuentes Bus B (2 bits)
#define BUS_B_REG    0x00
#define BUS_B_RAM    0x01
#define BUS_B_INM    0x02

// Fuentes Bus C (4 bits)
#define BUS_C_ALU        0x00
#define BUS_C_SRC_REG_B  0x01
#define BUS_C_RAM        0x02
#define BUS_C_INM        0x03

// Operaciones ALU (3 bits)
#define ALU_OP_ADD   0x00
#define ALU_OP_SUB   0x01
#define ALU_OP_AND   0x02
#define ALU_OP_OR    0x03
#define ALU_OP_XOR   0x04
#define ALU_OP_NOT   0x05
#define ALU_OP_SHL   0x06
#define ALU_OP_SHR   0x07

// Selección de Flags para Saltos (2 bits)
#define FLAG_N       0x00
#define FLAG_Z       0x01
#define FLAG_C       0x02
#define FLAG_V       0x03

/**
 * Recibe un puntero a la estructura intermedia del parser/IR y 
 * empaqueta los 64 bits listos para escribir en la memoria ROM.
 * 
 * Retorna 0 si la codificación fue exitosa, o un código distinto de 0 en caso de error.
 */
int codificar_instruccion(InstruccionIR* instr, uint64_t* salida);

#endif // ENCODER_H