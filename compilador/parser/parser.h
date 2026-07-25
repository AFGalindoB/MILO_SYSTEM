#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include <stdint.h>

typedef enum {
    INSTR_NOP,

    INSTR_MOV,
    INSTR_MOVI,
    INSTR_LOAD,
    INSTR_STORE,
    
    INSTR_ADD,
    INSTR_SUB,
    INSTR_ADC,
    INSTR_SBC,
    INSTR_AND,
    INSTR_OR,
    INSTR_XOR,
    INSTR_NOT,
    INSTR_SHL,
    INSTR_SHR,

    INSTR_CMP,
    INSTR_JMP,
    INSTR_CALL,
    INSTR_RET,
    INSTR_JZ,
    INSTR_JNZ,
    INSTR_JC,
    INSTR_JNC,
    INSTR_JN,
    INSTR_JNN,
    INSTR_JV,
    INSTR_JNV,
    INSTR_WAIT,

    INSTR_DESCONOCIDA
} TipoInstruccion;

typedef enum {
    FORMATO_DESCONOCIDO,
    FORMATO_SIN_OPERANDOS,  // NOP, RET, STOP
    FORMATO_SALTO,          // JMP, JZ, JNZ, CALL...
    FORMATO_MOV,            // MOV
    FORMATO_MOVI,           // MOVI
    FORMATO_LOAD,           // LOAD
    FORMATO_STORE,          // STORE
    FORMATO_ALU_UNARIO,     // NOT
    FORMATO_ALU_BINARIO     // ADD, SUB, AND, OR, XOR, ADC, SBC, SHL, SHR, CMP
} FormatoInstruccion;

typedef enum {
    OPERANDO_NULO = 0,
    OPERANDO_REG_RW,   // Registros R0 - R15 (valor: 0 - 15)
    OPERANDO_REG_RO,   // RINPT (valor: ID de RINPT)
    OPERANDO_REG_WO,   // TBUF, SCROLL (valor: ID de TBUF/SCROLL)
    OPERANDO_INMEDIATO // Literales o Direcciones (valor: uint32_t directo)
} TipoOperando;

typedef struct {
    TipoOperando tipo;
    uint32_t valor;
} Operando;

typedef struct {
    TipoInstruccion tipo;
    FormatoInstruccion formato;

    uint32_t linea;

    uint8_t tiene_modificador_f; // 1 si incluye .F, 0 en otro caso

    // Destinos fijos: [0] = RW (Rd), [1] = WO (TBUF/SCROLL)
    Operando destino[2];

    // Fuentes fijas: [0] = Primera fuente, [1] = Segunda fuente
    Operando fuente[2];
} InstruccionIR;

#define MAX_ERRORES 20

typedef struct {
    int linea;
    int columna;
    char linea_completa_str[256];
    char tipo_token_str[32];
    char mensaje[256];
} RegistroError;

extern Token* linea_tokens;
extern int total_tokens;
extern uint32_t contador_errores;

InstruccionIR parsear_linea_tokens(Token* tokens, int cantidad_tokens);

void reportar_error(int offset_error, TipoToken tipo_error, const char* formato, ...);
void emitir_informe_compilacion(void);

#endif // PARSER_H