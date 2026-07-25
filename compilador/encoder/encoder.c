#include "encoder.h"
#include "../parser/parser.h"
#include <stdio.h>

// Helper interno para empaquetar los campos físicos en los 64 bits
static PalabraROM empaquetar_campos(uint32_t opcode, uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t bus_c, uint32_t fine_control, uint32_t immediate) {
    
    PalabraROM palabra;
    palabra.control_word = 0;
    palabra.immediate = immediate;

    // Desplazamientos limpios usando uint32_t
    palabra.control_word |= (opcode       & 0x3F)  << 26;
    palabra.control_word |= (rd           & 0x0F)  << 22;
    palabra.control_word |= (rs2          & 0x0F)  << 18;
    palabra.control_word |= (rs1          & 0x0F)  << 14;
    palabra.control_word |= (bus_c        & 0x1F)  << 9;
    palabra.control_word |= (fine_control & 0x1FF) << 0;

    return palabra;
}

int codificar_instruccion(InstruccionIR* instr, PalabraROM* salida) {

    uint32_t rd = (instr->destino[0].tipo == OPERANDO_REG_RW) ? instr->destino[0].valor : 0;
    uint32_t regs_enable = (instr->destino[0].tipo == OPERANDO_REG_RW) ? 1 : 0;

    uint32_t we_tile_buffer = 0;
    uint32_t scroll = 0;

    if (instr->destino[1].tipo == OPERANDO_REG_WO) {
        if (instr->destino[1].valor == 0) { // TBUF
            we_tile_buffer = 1;
        } else if (instr->destino[1].valor == 1) { // SCROLL
            scroll = 1;
        }
    }

    switch (instr->tipo) {
        
        // ==========================================
        // CASO ESPECIAL: RET (Expansión física)
        // ==========================================
        case INSTR_RET: {
            // Palabra 1: Decrementar el Stack Pointer (SP = SP - 1)
            // opcode = 0x03, ENABLE_SP = 1 (bit 4), UPDOWN_SP = 0 (bit 3)
            uint32_t fine_ctrl_1 = (0 << 0) | (0 << 1) | (0 << 2) | (0 << 3) | (1 << 4);
            salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, fine_ctrl_1, 0);

            // Palabra 2: Cargar PC desde el Stack
            // opcode = 0x03, PC_JUMP = 1 (bit 0), SEL_MUX = 1 (bit 1)
            uint32_t fine_ctrl_2 = (1 << 0) | (1 << 1) | (0 << 2) | (0 << 3) | (0 << 4);
            salida[1] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, fine_ctrl_2, 0);

            return 2; // Retornamos que generó 2 microinstrucciones
        }

        // ==========================================
        // OPERACIONES ALU BINARIAS
        // ==========================================
        case INSTR_ADD:
        case INSTR_ADC:
        case INSTR_SUB:
        case INSTR_SBC:
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_XOR:
        case INSTR_SHL:
        case INSTR_SHR: {
            uint32_t ra = instr->fuente[0].valor;
            uint32_t rb = instr->fuente[1].valor;

            uint32_t alu_op = 0;
            uint32_t carry_in = 0;

            if      (instr->tipo == INSTR_ADD) { alu_op = ALU_OP_ADD; }
            else if (instr->tipo == INSTR_ADC) { alu_op = ALU_OP_ADD; carry_in = 1; }
            else if (instr->tipo == INSTR_SUB) { alu_op = ALU_OP_SUB; }
            else if (instr->tipo == INSTR_SBC) { alu_op = ALU_OP_SUB; carry_in = 1; }
            else if (instr->tipo == INSTR_AND) { alu_op = ALU_OP_AND; }
            else if (instr->tipo == INSTR_OR)  { alu_op = ALU_OP_OR;  }
            else if (instr->tipo == INSTR_XOR) { alu_op = ALU_OP_XOR; }
            else if (instr->tipo == INSTR_SHL) { alu_op = ALU_OP_SHL; }
            else if (instr->tipo == INSTR_SHR) { alu_op = ALU_OP_SHR; }

            uint32_t update_flags = instr->tiene_modificador_f ? 1 : 0;
            uint32_t fine_control = ( regs_enable << 0 )    |
                                    ( 1 << 1 )              |
                                    ( update_flags << 2 )   |
                                    ( carry_in << 3 )       |
                                    ( (alu_op & 0x07) << 4 )|
                                    ( scroll << 7 )         | 
                                    ( we_tile_buffer << 8 );

            salida[0] = empaquetar_campos(OPCODE_ALU, rd, ra, rb, BUS_C_ALU, fine_control, 0);
            return 1;
        }

        // ==========================================
        // COMPARACIÓN (CMP)
        // ==========================================
        case INSTR_CMP: {
            uint32_t ra = instr->fuente[0].valor;
            uint32_t rb = instr->fuente[1].valor;

            uint32_t fine_control = (0 << 0) | (1 << 1) | (1 << 2) | (0 << 3) | ((ALU_OP_SUB & 0x07) << 4);
            salida[0] = empaquetar_campos(OPCODE_ALU, 0, ra, rb, BUS_C_ALU , fine_control, 0);
            return 1;
        }

        // ==========================================
        // OPERACIONES ALU UNARIAS (NOT)
        // ==========================================
        case INSTR_NOT: {
            uint32_t ra = instr->fuente[0].valor;

            uint32_t update_flags = instr->tiene_modificador_f ? 1 : 0;

            uint32_t fine_control = ( regs_enable << 0)         |
                                    (1 << 1)                    |
                                    (update_flags << 2)         |
                                    (0 << 3)                    |
                                    ((ALU_OP_NOT & 0x07) << 4)  |
                                    (scroll << 7)               | 
                                    (we_tile_buffer << 8);
            salida[0] = empaquetar_campos(OPCODE_ALU, rd, ra, 0, 0x00, fine_control, 0);
            return 1;
        }

        // ==========================================
        // MOV & MOVI
        // ==========================================
        case INSTR_MOV: {

            uint32_t rs1 = (instr->fuente[0].tipo == OPERANDO_REG_RW) ? instr->fuente[0].valor : 0;

            uint32_t bus_c = (instr->fuente[0].tipo == OPERANDO_REG_RO) ? BUS_C_RINPT : BUS_C_REGSRC1;

            uint32_t fine_control = (regs_enable << 0)   | 
                                    (0 << 1)             | // RAM_WE = 0
                                    (0 << 2)             | // ENABLE_MDR_&_MAR = 0
                                    (scroll << 3)        | 
                                    (we_tile_buffer << 4);

            salida[0] = empaquetar_campos(OPCODE_MOV, rd, rs1, 0, bus_c, fine_control, 0);
            return 1;
        }

        case INSTR_MOVI: {
            uint32_t inmediato = instr->fuente[0].valor;

            uint32_t fine_control = (regs_enable << 0)   | 
                                    (0 << 1)             | 
                                    (0 << 2)             | 
                                    (scroll << 3)        | 
                                    (we_tile_buffer << 4);
            salida[0] = empaquetar_campos(OPCODE_MOV, rd, 0, 0, BUS_C_ROM_INM, fine_control, inmediato);
            return 1;
        }

        // ==========================================
        // LOAD & STORE
        // ==========================================
        case INSTR_LOAD: {
            uint32_t mar = instr->fuente[0].valor;

            uint32_t fine_control = (regs_enable << 0)   | 
                                    (0 << 1)             | // RAM_WE = 0
                                    (1 << 2)             | // ENABLE_MDR_&_MAR = 1
                                    (scroll << 3)        | 
                                    (we_tile_buffer << 4);
            salida[0] = empaquetar_campos(OPCODE_MOV, rd, mar, 0, BUS_C_RAM, fine_control, 0);
            return 1;
        }

        case INSTR_STORE: {
            uint32_t mdr = instr->fuente[0].valor;
            uint32_t mar = instr->fuente[1].valor;

            uint32_t fine_control = (1 << 2) | (1 << 1);
            salida[0] = empaquetar_campos(OPCODE_MOV, 0, mar, mdr, 0x00, fine_control, 0);
            return 1;
        }

        // ==========================================
        // CONTROL DE FLUJO (CALL & JMP)
        // ==========================================
        case INSTR_JMP: {
            uint32_t direccion = instr->fuente[0].valor;

            uint32_t fine_control = (1 << 0); // PC_JUMP = 1
            salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0x00, fine_control, direccion);
            return 1;
        }

        case INSTR_CALL: {
            int32_t direccion = instr->fuente[0].valor;

            // WE_STACK = 1, UD_SP = 1, ENABLE_SP = 1, PC_JUMP = 1
            uint32_t fine_control = (1 << 0) | (0 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
            salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0x00, fine_control, direccion);
            return 1;
        }

        // ==========================================
        // SALTOS CONDICIONALES
        // ==========================================
        case INSTR_JZ:
        case INSTR_JNZ:
        case INSTR_JC:
        case INSTR_JNC:
        case INSTR_JN:
        case INSTR_JNN:
        case INSTR_JV:
        case INSTR_JNV: {
            uint32_t direccion = instr->fuente[0].valor;

            uint32_t sel_flag = 0;
            uint32_t negate_flag = 0;

            if (instr->tipo == INSTR_JN || instr->tipo == INSTR_JNN) {
                sel_flag = FLAG_N;
                if (instr->tipo == INSTR_JNN) negate_flag = 1;
            } 
            else if (instr->tipo == INSTR_JZ || instr->tipo == INSTR_JNZ) {
                sel_flag = FLAG_Z;
                if (instr->tipo == INSTR_JNZ) negate_flag = 1;
            } 
            else if (instr->tipo == INSTR_JC || instr->tipo == INSTR_JNC) {
                sel_flag = FLAG_C;
                if (instr->tipo == INSTR_JNC) negate_flag = 1;
            } 
            else if (instr->tipo == INSTR_JV || instr->tipo == INSTR_JNV) {
                sel_flag = FLAG_V;
                if (instr->tipo == INSTR_JNV) negate_flag = 1;
            }

            uint32_t fine_control = (0 << 0) | (0 << 1) | (1 << 5) | (negate_flag << 6) | ((sel_flag & 0x03) << 7);
            salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0x00, fine_control, direccion);
            return 1;
        }

        case INSTR_WAIT: {
            uint32_t sel_wait = instr->fuente[0].valor;

            uint32_t fine_control = (1 << 0) | (sel_wait << 1);
            salida[0] = empaquetar_campos(OPCODE_STOP, 0, 0, 0, 0, fine_control, 0);
            return 1;
        }

        default:
            // NOP por defecto
            salida[0] = empaquetar_campos(0, 0, 0, 0, 0, 0, 0);
            return 1;
    }
}