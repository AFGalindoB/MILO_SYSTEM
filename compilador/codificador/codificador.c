#include "codificador.h"
#include "../parser/parser.h"
#include <stdio.h>

PalabraROM codificar_instruccion(InstruccionParseada* instr) {
    PalabraROM palabra;
    palabra.control_word = 0;
    palabra.immediate = 0;

    uint32_t opcode = 0;
    uint32_t reg_dest = 0;
    uint32_t reg_src2 = 0;
    uint32_t reg_src1 = 0;
    uint32_t bus_c_selector = 0;
    uint32_t fine_control = 0;

    switch (instr->tipo) {
        case INSTR_NOP:
            opcode = 0x00; // 000000
            // Todos los demás campos se quedan en 0 (Fine Control ignorado)
            break;

        // =================================================================
        // OPERACIONES ALU BINARIAS (3 Registros: Rd, Ra, Rb)
        // =================================================================
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_XOR:
        case INSTR_SHL:
        case INSTR_SHR:
            opcode = 0x01;         // 000001 (ALU)
            bus_c_selector = 0x00; // 00000 (Origen: ALU)
            
            reg_dest = instr->operandos.alu.rd;
            reg_src1 = instr->operandos.alu.ra;
            reg_src2 = instr->operandos.alu.rb;

            // Seleccion de operacion aritmetico logica Bits 6-4
            uint32_t alu_op = 0;
            if (instr->tipo == INSTR_ADD) alu_op = ALU_OP_ADD;
            else if (instr->tipo == INSTR_SUB) alu_op = ALU_OP_SUB;
            else if (instr->tipo == INSTR_AND) alu_op = ALU_OP_AND;
            else if (instr->tipo == INSTR_OR)  alu_op = ALU_OP_OR;
            else if (instr->tipo == INSTR_XOR) alu_op = ALU_OP_XOR;
            else if (instr->tipo == INSTR_SHL) alu_op = ALU_OP_SHL;
            else if (instr->tipo == INSTR_SHR) alu_op = ALU_OP_SHR;

            // 🛠️ Nuevo mapa de bits con bits de habilitación separados:
            // Bit 0: REGS_ENABLE = 1 (Queremos guardar el resultado)
            // Bit 1: ENABLE_ALU = 1 (Habilita entrada de datos a la ALU)
            // Bit 2: UPDATE_FLAGS = 0 (Filosofía ARM por defecto)
            // Bit 3: CARRY_IN = 0
            // Bits 6-4: alu_op desplazado 4 posiciones (<< 4)
            fine_control = (1 << 0) | (1 << 1) | ((alu_op & 0x07) << 4); 
            break;

        // =================================================================
        // OPERACIONES ALU UNARIAS (2 Registros: Rd, Ra)
        // =================================================================
        case INSTR_NOT:
            opcode = 0x01;         // 000001 (Mismo bloque funcional ALU)
            bus_c_selector = 0x00; // 00000 (Origen: ALU)
            
            // Extrae del nuevo struct alu_unaria para evitar colisiones de memoria
            reg_dest = instr->operandos.alu_unaria.rd;
            reg_src1 = instr->operandos.alu_unaria.ra; // Pasa por el Bus A
            reg_src2 = 0;                              // No se usa un segundo operando

            uint32_t alu_op_unaria = ALU_OP_NOT;

            // Mismo mapeo de Fine Control (REGS_ENABLE y ENABLE_ALU activos, UPDATE_FLAGS en 0)
            fine_control = (1 << 0) | (1 << 1) | ((alu_op_unaria & 0x07) << 4);
            break;

        case INSTR_MOV:
            opcode = 0x02;         // 000010 (MOV)
            bus_c_selector = 0x01; // 00001 (Origen: RegSrc1)
            
            reg_dest = instr->operandos.mov.rd;
            reg_src1 = instr->operandos.mov.rs; // Pasa por el Bus A
            reg_src2 = 0;

            // Fine Control: Bit 0 = ENABLE_REGS (1)
            fine_control = (1 << 0);
            break;

        case INSTR_MOVI:
            opcode = 0x02;         // 000010 (MOV comparte opcode base)
            bus_c_selector = 0x03; // 00011 (Origen: ROM Immediate)
            
            reg_dest = instr->operandos.movi.rd;
            reg_src1 = 0;
            reg_src2 = 0;

            // Fine Control: Bit 0 = ENABLE_REGS (1)
            fine_control = (1 << 0);
            
            // Payload de 32 bits ocupado por el valor inmediato
            palabra.immediate = instr->operandos.movi.valor;
            break;

        case INSTR_LOAD:
            opcode = 0x02;         // Operación de transferencia (MOV base)
            bus_c_selector = 0x02; // 00010 (Origen: RAM)
            
            reg_dest = instr->operandos.load.rd;
            reg_src1 = instr->operandos.load.mar; // RegSrc1 asume rol de bus de dirección
            reg_src2 = 0;

            // Fine Control: 
            // Bit 2: ENABLE_MDR_&_MAR (1) -> Activa modo memoria
            // Bit 0: ENABLE_REGS (1)       -> Permite guardar en RegDest
            fine_control = (1 << 2) | (1 << 0);
            break;

        case INSTR_STORE:
            opcode = 0x02;         // Operación de transferencia
            bus_c_selector = 0x00; // No escribe en registros, Bus C ignorado
            
            reg_dest = 0; 
            reg_src1 = instr->operandos.store.mar; // Dirección en memoria
            reg_src2 = instr->operandos.store.mdr; // Dato a escribir en memoria

            // Fine Control:
            // Bit 2: ENABLE_MDR_&_MAR (1) -> Activa modo memoria
            // Bit 1: RAM_WE (1)           -> Habilita la escritura física en la RAM
            fine_control = (1 << 2) | (1 << 1);
            break;

        default:
            // Instrucción desconocida genera un NOP de seguridad
            opcode = 0;
            break;
    }

    // Ensamble final de la Control Word (32 bits) desplazando cada campo a su posición:
    // Opcode (6 bits)       -> desplaza 26 bits (32 - 6)
    // RegDest (4 bits)      -> desplaza 22 bits (26 - 4)
    // RegSrc2 (4 bits)      -> desplaza 18 bits (22 - 4)
    // RegSrc1 (4 bits)      -> desplaza 14 bits (18 - 4)
    // Bus C Selector (5b)   -> desplaza 9 bits  (14 - 5)
    // Fine Control (9 bits) -> desplaza 0 bits  (9 - 9)
    
    palabra.control_word |= (opcode         & 0x3F) << 26;
    palabra.control_word |= (reg_dest       & 0x0F) << 22;
    palabra.control_word |= (reg_src2       & 0x0F) << 18;
    palabra.control_word |= (reg_src1       & 0x0F) << 14;
    palabra.control_word |= (bus_c_selector & 0x1F) << 9;
    palabra.control_word |= (fine_control   & 0x1FF); // Máscara de 9 bits (0x1FF)

    return palabra;
}