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

int codificar_instruccion(InstruccionParseada* instr, PalabraROM* salida) {
    switch (instr->tipo) {
        
        // ==========================================
        // CASO ESPECIAL: RET (Expansión física)
        // ==========================================
        case INSTR_RET: {
            // Palabra 1: Decrementar el Stack Pointer (SP = SP - 1)
            // opcode = 0x03, ENABLE_SP = 1 (bit 4), UPDOWN_SP = 0 (bit 3)
            uint32_t fine_ctrl_1 = (0 << 0) | (0 << 1) | (0 << 2) | (0 << 3) | (1 << 4);
            salida[0] = empaquetar_campos(0x03, 0, 0, 0, 0, fine_ctrl_1, 0);

            // Palabra 2: Cargar PC desde el Stack
            // opcode = 0x03, PC_JUMP = 1 (bit 0), SEL_MUX = 1 (bit 1)
            uint32_t fine_ctrl_2 = (1 << 0) | (1 << 1) | (0 << 2) | (0 << 3) | (0 << 4);
            salida[1] = empaquetar_campos(0x03, 0, 0, 0, 0, fine_ctrl_2, 0);

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
            uint32_t opcode = 0x01;
            uint32_t bus_c = 0x00;
            uint32_t rd = instr->operandos.alu.rd;
            uint32_t ra = instr->operandos.alu.ra;
            uint32_t rb = instr->operandos.alu.rb;

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

            uint32_t update_flags = instr->operandos.alu.update_flags & 0x01;
            uint32_t fine_control = (1 << 0) | (1 << 1) | (update_flags << 2) | (carry_in << 3) | ((alu_op & 0x07) << 4);

            salida[0] = empaquetar_campos(opcode, rd, ra, rb, bus_c, fine_control, 0);
            return 1;
        }

        // ==========================================
        // COMPARACIÓN (CMP)
        // ==========================================
        case INSTR_CMP: {
            uint32_t fine_control = (0 << 0) | (1 << 1) | (1 << 2) | (0 << 3) | ((ALU_OP_SUB & 0x07) << 4);
            salida[0] = empaquetar_campos(0x01, 0, instr->operandos.alu.ra, instr->operandos.alu.rb, 0x00, fine_control, 0);
            return 1;
        }

        // ==========================================
        // OPERACIONES ALU UNARIAS (NOT)
        // ==========================================
        case INSTR_NOT: {
            uint32_t update_flags = instr->operandos.alu_unaria.update_flags & 0x01;
            uint32_t fine_control = (1 << 0) | (1 << 1) | (update_flags << 2) | (0 << 3) | ((ALU_OP_NOT & 0x07) << 4);
            salida[0] = empaquetar_campos(0x01, instr->operandos.alu_unaria.rd, instr->operandos.alu_unaria.ra, 0, 0x00, fine_control, 0);
            return 1;
        }

        // ==========================================
        // MOV & MOVI
        // ==========================================
        case INSTR_MOV: {
            salida[0] = empaquetar_campos(0x02, instr->operandos.mov.rd, instr->operandos.mov.rs, 0, 0x01, (1 << 0), 0);
            return 1;
        }

        case INSTR_MOVI: {
            salida[0] = empaquetar_campos(0x02, instr->operandos.movi.rd, 0, 0, 0x03, (1 << 0), instr->operandos.movi.valor);
            return 1;
        }

        // ==========================================
        // LOAD & STORE
        // ==========================================
        case INSTR_LOAD: {
            uint32_t fine_control = (1 << 2) | (1 << 0);
            salida[0] = empaquetar_campos(0x02, instr->operandos.load.rd, instr->operandos.load.mar, 0, 0x02, fine_control, 0);
            return 1;
        }

        case INSTR_STORE: {
            uint32_t fine_control = (1 << 2) | (1 << 1);
            salida[0] = empaquetar_campos(0x02, 0, instr->operandos.store.mar, instr->operandos.store.mdr, 0x00, fine_control, 0);
            return 1;
        }

        // ==========================================
        // CONTROL DE FLUJO (CALL & JMP)
        // ==========================================
        case INSTR_JMP: {
            uint32_t fine_control = (1 << 0); // PC_JUMP = 1
            salida[0] = empaquetar_campos(0x03, 0, 0, 0, 0x00, fine_control, instr->operandos.salto.destino);
            return 1;
        }

        case INSTR_CALL: {
            // WE_STACK = 1, UD_SP = 1, ENABLE_SP = 1, PC_JUMP = 1
            uint32_t fine_control = (1 << 0) | (0 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
            salida[0] = empaquetar_campos(0x03, 0, 0, 0, 0x00, fine_control, instr->operandos.salto.destino);
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
            salida[0] = empaquetar_campos(0x03, 0, 0, 0, 0x00, fine_control, instr->operandos.salto.destino);
            return 1;
        }

        // ==========================================
        // GPU INSTRUCTIONS (TBUF, PIXOFF, TILEOFF, SCROLL)
        // ==========================================
        case INSTR_GPU_TBUF: {
            uint32_t opcode = 0x04;
            uint32_t fine_control = (1 << 1); // WE_TILE_BUFFER = 1
            
            // Bus de 24 bits empaquetado: Bits 23-8 = Índice del tile, Bits 7-0 = Dirección
            uint32_t tile = instr->operandos.gpu_tbuf.tile & 0xFFFF;
            uint32_t addr = instr->operandos.gpu_tbuf.addr & 0xFF;
            uint32_t immediate = (tile << 8) | addr;

            salida[0] = empaquetar_campos(opcode, 0, 0, 0, 0, fine_control, immediate);
            return 1;
        }

        case INSTR_GPU_PXOFF: {
            uint32_t opcode = 0x04;
            uint32_t fine_control = (1 << 2); // WE_PIXEL_OFFSET = 1
            
            // Registro de 6 bits: Bits 5-3 = Y Offset (0-7), Bits 2-0 = X Offset (0-7)
            uint32_t px = instr->operandos.gpu_off.x & 0x07;
            uint32_t py = instr->operandos.gpu_off.y & 0x07;
            uint32_t immediate = (py << 3) | px;

            salida[0] = empaquetar_campos(opcode, 0, 0, 0, 0, fine_control, immediate);
            return 1;
        }

        case INSTR_GPU_TLOFF: {
            uint32_t opcode = 0x04;
            uint32_t fine_control = (1 << 0); // WE_TILE_OFFSET = 1
            
            // Registro de 8 bits: Bits 7-4 = Tile Y (0-15), Bits 3-0 = Tile X (0-15)
            uint32_t tx = instr->operandos.gpu_off.x & 0x0F;
            uint32_t ty = instr->operandos.gpu_off.y & 0x0F;
            uint32_t immediate = (ty << 4) | tx;

            salida[0] = empaquetar_campos(opcode, 0, 0, 0, 0, fine_control, immediate);
            return 1;
        }

        case INSTR_GPU_SCROLL: {
            uint32_t opcode = 0x04;
            // Escritura simultánea: WE_PIXEL_OFFSET = 1 (bit 2) y WE_TILE_OFFSET = 1 (bit 0) -> 0x05
            uint32_t fine_control = (1 << 2) | (1 << 0);
            
            // Bus acoplado de 14 bits: 
            // Bits 13-8 = Pixel Offset (Y_fino << 3 | X_fino)
            // Bits 7-0  = Tile Offset  (Y_tile << 4 | X_tile)
            uint32_t tx = instr->operandos.gpu_scroll.tx & 0x0F;
            uint32_t ty = instr->operandos.gpu_scroll.ty & 0x0F;
            uint32_t px = instr->operandos.gpu_scroll.px & 0x07;
            uint32_t py = instr->operandos.gpu_scroll.py & 0x07;

            uint32_t pixel_offset = (py << 3) | px;
            uint32_t tile_offset  = (ty << 4) | tx;
            uint32_t immediate    = (pixel_offset << 8) | tile_offset;

            salida[0] = empaquetar_campos(opcode, 0, 0, 0, 0, fine_control, immediate);
            return 1;
        }


        default:
            // NOP por defecto
            salida[0] = empaquetar_campos(0, 0, 0, 0, 0, 0, 0);
            return 1;
    }
}