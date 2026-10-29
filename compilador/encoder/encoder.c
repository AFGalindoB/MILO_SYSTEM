#include "encoder.h"
#include "../parser/parser.h"
#include <stdio.h>

// Helper interno para empaquetar los campos físicos en los 64 bits
static uint64_t empaquetar_campos(
    uint8_t opcode,        // 4 bits  (63:60)
    uint8_t mar_control,   // 4 bits  (59:56)
    uint8_t reg_src_a,     // 4 bits  (55:52)
    uint8_t reg_src_b,     // 4 bits  (51:48)
    uint8_t reg_dest,      // 4 bits  (47:44)
    uint8_t bus_c_sel,     // 4 bits  (43:40)
    uint8_t bus_b_sel,     // 2 bits  (39:38)
    uint8_t bus_a_sel,     // 2 bits  (37:36)
    uint16_t fine_control, // 12 bits (35:24)
    uint32_t immediate     // 24 bits (23:0)
) {
    uint64_t palabra = 0;

    palabra |= ((uint64_t)(opcode       & 0x0F)) << 60;
    palabra |= ((uint64_t)(mar_control  & 0x0F)) << 56;
    palabra |= ((uint64_t)(reg_src_a    & 0x0F)) << 52;
    palabra |= ((uint64_t)(reg_src_b    & 0x0F)) << 48;
    palabra |= ((uint64_t)(reg_dest     & 0x0F)) << 44;
    palabra |= ((uint64_t)(bus_c_sel    & 0x0F)) << 40;
    palabra |= ((uint64_t)(bus_b_sel    & 0x03)) << 38;
    palabra |= ((uint64_t)(bus_a_sel    & 0x03)) << 36;
    palabra |= ((uint64_t)(fine_control & 0x0FFF)) << 24;
    palabra |= ((uint64_t)(immediate    & 0x00FFFFFF));

    return palabra;
}

int codificar_instruccion(InstruccionIR* instr, uint64_t* salida) {

    uint8_t mar_control = 0;
    uint32_t inmediato = instr->inmediato.valor & 0x00FFFFFF; // Tomamos solo los 24 bits menos significativos

    if (instr->MAR_Control.tipo == OPERANDO_REG) {
        // MAR_SRC = 0 (bit 3 en 0)
        // MAR_REG = R0 a R7 (bits 2:0)
        mar_control = (uint8_t)(instr->MAR_Control.valor & 0x07);
    } else if (instr->MAR_Control.tipo == OPERANDO_INMEDIATO) {
        // MAR_SRC = 1 (bit 3 en 1 -> 1 << 3)
        // MAR_REG = 000 (bits 2:0 en 0)
        mar_control = (1 << 3); // 0x08
    } else {
        // Sin uso de MAR en la instrucción
        mar_control = 0;
    }

    uint8_t reg_dest = 0;
    uint8_t regs_enable = 0;

    if (instr->destino.tipo == OPERANDO_REG) {
        regs_enable = 1;
        reg_dest = (uint8_t)(instr->destino.valor & 0x0F); // Mascara de 4 bits para R0-R15
    } else {
        regs_enable = 0;
        reg_dest = 0;
    }

    uint8_t ram_we = (instr->destino.tipo == OPERANDO_RAM) ? 1 : 0;

    switch (instr->formato) {
        
        case FORMATO_SIN_OPERANDOS: {
            if (instr->tipo == INSTR_RET) {
                // Palabra 1: Decrementar el Stack Pointer (SP = SP - 1)
                // opcode = 0x03, ENABLE_SP = 1 (bit 4), UPDOWN_SP = 0 (bit 3)
                uint32_t fine_ctrl_1 = (0 << 0) | (0 << 1) | (0 << 2) | (0 << 3) | (1 << 4);
                salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, 0, 0, 0, fine_ctrl_1, 0);

                // Palabra 2: Cargar PC desde el Stack
                // opcode = 0x03, PC_JUMP = 1 (bit 0), SEL_MUX = 1 (bit 1)
                uint32_t fine_ctrl_2 = (1 << 0) | (1 << 1);
                salida[1] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, 0, 0, 0, fine_ctrl_2, 0);

                return 2; // Retornamos que generó 2 microinstrucciones   
            }else{
                salida[0] = empaquetar_campos(OPCODE_NOP, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                return 1;
            }
        }

        case FORMATO_SALTO: {
            uint32_t direccion = inmediato & 0x00FFFFFF; // Tomamos solo los 24 bits menos significativos
            if (instr->tipo == INSTR_JMP) {
                salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, 0, 0, 0, (1 << 0), direccion);
                return 1;    
            } else if (instr->tipo == INSTR_CALL) {
                // WE_STACK = 1, UD_SP = 1, ENABLE_SP = 1, PC_JUMP = 1
                uint32_t fine_control = (1 << 0) | (0 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
                salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, 0, 0, 0, fine_control, direccion);
                return 1;
            } else {
                uint32_t sel_flag = 0;
                uint32_t negate_flag = 0;

                if (instr->tipo == INSTR_JN || instr->tipo == INSTR_JNN) {
                    sel_flag = FLAG_N;
                    if (instr->tipo == INSTR_JNN) negate_flag = 1;
                } else if (instr->tipo == INSTR_JZ || instr->tipo == INSTR_JNZ) {
                    sel_flag = FLAG_Z;
                    if (instr->tipo == INSTR_JNZ) negate_flag = 1;
                } else if (instr->tipo == INSTR_JC || instr->tipo == INSTR_JNC) {
                    sel_flag = FLAG_C;
                    if (instr->tipo == INSTR_JNC) negate_flag = 1;
                } else if (instr->tipo == INSTR_JV || instr->tipo == INSTR_JNV) {
                    sel_flag = FLAG_V;
                    if (instr->tipo == INSTR_JNV) negate_flag = 1;
                }

                uint32_t fine_control = (0 << 0) | (0 << 1) | (1 << 5) | (negate_flag << 6) | ((sel_flag & 0x03) << 7);
                salida[0] = empaquetar_campos(OPCODE_PC, 0, 0, 0, 0, 0, 0, 0, fine_control, direccion);
                return 1;
            }
        }

        case FORMATO_MOV: {
            uint8_t reg_srcb = (instr->fuente[0].tipo == OPERANDO_REG) ? (uint8_t)(instr->fuente[0].valor & 0x0F) : 0;
            uint8_t bus_c_sel;

            switch (instr->fuente[0].tipo) {
                case OPERANDO_REG:
                    bus_c_sel = BUS_C_SRC_REG_B; break;
                case OPERANDO_INMEDIATO:
                    bus_c_sel = BUS_C_INM; break;
                case OPERANDO_RAM:
                    bus_c_sel = BUS_C_RAM; break;
                default:
                    reportar_error(0, TOKEN_IDENTIFICADOR, "Operando fuente no válido para MOV.");
                    return 0;
            }

            uint32_t fine_control = ((ram_we & 0x01) << 0)      | 
                                    ((regs_enable & 0x01) << 1) ;
            salida[0] = empaquetar_campos(OPCODE_MOV, mar_control, 0, reg_srcb, reg_dest, bus_c_sel, 0, 0,fine_control, inmediato);
            return 1;
        }

        case FORMATO_ALU_BINARIO: {
            uint8_t regsrc_a = (instr->fuente[0].tipo == OPERANDO_REG) ? (uint8_t)(instr->fuente[0].valor & 0x0F) : 0;
            uint8_t regsrc_b = (instr->fuente[1].tipo == OPERANDO_REG) ? (uint8_t)(instr->fuente[1].valor & 0x0F) : 0;

            uint8_t bus_b_sel;
            switch (instr->fuente[1].tipo) {
                case OPERANDO_REG:
                    bus_b_sel = BUS_B_REG; break;
                case OPERANDO_INMEDIATO:
                    bus_b_sel = BUS_B_INM; break;
                case OPERANDO_RAM:
                    bus_b_sel = BUS_B_RAM; break;
                default:
                    reportar_error(0, TOKEN_IDENTIFICADOR, "Operando fuente B no válido para instrucción ALU.");
                    return 0;
            }

            uint8_t bus_a_sel;
            switch (instr->fuente[0].tipo) {
                case OPERANDO_REG:
                    bus_a_sel = BUS_A_REG; break;
                case OPERANDO_INMEDIATO:
                    bus_a_sel = BUS_A_INM; break;
                case OPERANDO_RAM:
                    bus_a_sel = BUS_A_RAM; break;
                default:
                    reportar_error(0, TOKEN_IDENTIFICADOR, "Operando fuente A no válido para instrucción ALU.");
                    return 0;
            }

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
            else if (instr->tipo == INSTR_CMP) { alu_op = ALU_OP_SUB; }

            uint32_t update_flags = instr->tiene_modificador_f ? 1 : 0;
            uint32_t fine_control = (regs_enable & 0x01) << 0 |
                                    (1 & 0x01)           << 1 | // ENABLE_ALU
                                    (update_flags & 0x01)<< 2 |
                                    (carry_in & 0x01)    << 3 |
                                    (alu_op & 0x07)      << 4 | // Bits 6:4
                                    (ram_we & 0x01)      << 7;  // Bit 7

            salida[0] = empaquetar_campos(OPCODE_ALU, mar_control, regsrc_a, regsrc_b, reg_dest, BUS_C_ALU, bus_b_sel, bus_a_sel, fine_control, inmediato);
            return 1;
        }

        case FORMATO_ALU_UNARIO: {
            uint8_t regsrc_a = (instr->fuente[0].tipo == OPERANDO_REG) ? (uint8_t)(instr->fuente[0].valor & 0x0F) : 0;
            uint8_t bus_a_sel;
            switch (instr->fuente[0].tipo) {
                case OPERANDO_REG:
                    bus_a_sel = BUS_A_REG; break;
                case OPERANDO_INMEDIATO:
                    bus_a_sel = BUS_A_INM; break;
                case OPERANDO_RAM:
                    bus_a_sel = BUS_A_RAM; break;
                default:
                    reportar_error(0, TOKEN_IDENTIFICADOR, "Operando fuente A no válido para instrucción ALU.");
                    return 0;
            }

            uint32_t update_flags = instr->tiene_modificador_f ? 1 : 0;

            uint32_t fine_control = (regs_enable & 0x01) << 0 |
                                    (1 & 0x01)           << 1 | // ENABLE_ALU
                                    (update_flags & 0x01)<< 2 |
                                    (0 & 0x01)           << 3 |
                                    (ALU_OP_NOT & 0x07)  << 4 | // Bits 6:4
                                    (ram_we & 0x01)      << 7 ;  // Bit 7
            salida[0] = empaquetar_campos(OPCODE_ALU, mar_control, regsrc_a, 0, reg_dest, BUS_C_ALU, 0, bus_a_sel, fine_control, inmediato);
            return 1;
        }

        default:
            reportar_error(0, TOKEN_IDENTIFICADOR, "Formato de instrucción no soportado para codificación.");
            return 0;
    }
}