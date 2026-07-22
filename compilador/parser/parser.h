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

    INSTR_DESCONOCIDA
} TipoInstruccion;

typedef struct {
    TipoInstruccion tipo;
    uint32_t linea;
    
    // ====================================================
    // CAPA DE DESTINOS (Buses de Escritura / Orto-gonalidad)
    // ====================================================
    uint8_t reg_gpu;        // ID del registro especial (TBUF, SCROLL...)
    
    int tiene_rd;           // Booleano: ¿Escribe en el Banco de Registros?
    uint8_t rd;             // ID del registro general (R0 - R15)

    // ====================================================
    // CAPA DE OPERANDOS FUENTE (Unidades Funcionales)
    // ====================================================
    union {
        // La ALU solo necesita saber sus fuentes y si altera banderas
        struct { uint8_t ra; uint8_t rb; uint8_t update_flags; } alu;
        struct { uint8_t ra; uint8_t update_flags; } alu_unaria;
        
        // MOV solo necesita su fuente
        struct { uint8_t rs; } mov;
        
        // MOVI solo necesita su constante
        struct { uint32_t valor; } movi;
        
        // LOAD solo necesita el puntero de dirección
        struct { uint8_t mar; } load;
        
        // STORE es el único que no usa destinos superiores (usa fuentes fijas)
        struct { uint8_t mdr; uint8_t mar; } store;
        
        // Saltos de flujo
        struct { uint32_t destino; } salto;
    } fuentes;
    
} InstruccionParseada;

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

InstruccionParseada parsear_linea_tokens(Token* tokens, int cantidad_tokens);

int match_lexema(int offset, const char* lexema);
int match_tipo(int offset, TipoToken tipo);
void reportar_error(int offset_error, TipoToken tipo_error, const char* formato, ...);

// Prototipos de submódulos de parseo
void parsear_movimiento(InstruccionParseada* instr);
void parsear_alu(InstruccionParseada* instr);
void parsear_pc(InstruccionParseada* instr);

uint8_t obtener_destinos_ortogonales(InstruccionParseada* instr);

void emitir_informe_compilacion(void);

#endif // PARSER_H