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

    INSTR_GPU_TBUF,
    INSTR_GPU_PXOFF,
    INSTR_GPU_TLOFF,
    INSTR_GPU_SCROLL,

    INSTR_DESCONOCIDA
} TipoInstruccion;

typedef struct {
    TipoInstruccion tipo;
    uint32_t linea;
    
    union {
        struct { uint8_t rd; uint8_t ra; uint8_t rb; uint8_t update_flags; } alu;
        struct { uint8_t rd; uint8_t ra; uint8_t update_flags; } alu_unaria;
        struct { uint8_t rd; uint8_t rs; } mov;
        struct { uint8_t rd; uint32_t valor; } movi;
        struct { uint8_t rd; uint8_t mar; } load;
        struct { uint8_t mdr; uint8_t mar; } store;
        struct { uint32_t destino; } salto;

        struct { uint32_t addr; uint32_t tile;} gpu_tbuf;
        struct { uint32_t x; uint32_t y; } gpu_off;
        struct { uint32_t tx; uint32_t ty; uint32_t px; uint32_t py;} gpu_scroll;
    } operandos;
} InstruccionParseada;

#define MAX_ERRORES 20

typedef struct {
    int linea;
    int columna;
    char lexema[64];
    char tipo_token_str[32];
    char mensaje[256];
} RegistroError;

extern Token token_actual;
extern uint32_t contador_errores;

void avanzar_token(void);
int match(TipoToken tipo);
int match_lexema(TipoToken tipo, const char* lexema);

void consumir(TipoToken tipo_esperado, const char* mensaje_error);
void consumir_lexema(TipoToken tipo_esperado, const char* lexema_esperado, const char* mensaje_error);

// Interfaces de parsing
InstruccionParseada parsear_linea(void);
void parsear_movimiento(InstruccionParseada* instr);
void parsear_alu(InstruccionParseada* instr);
void parsear_pc(InstruccionParseada* instr);
void parsear_gpu(InstruccionParseada* instr);

void emitir_informe_compilacion(const char* codigo_fuente);

#endif // PARSER_H