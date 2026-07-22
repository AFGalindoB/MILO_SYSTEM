#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

// Tus tipos de Token exactos
typedef enum {
    TOKEN_IDENTIFICADOR,
    TOKEN_REGISTRO,
    TOKEN_INMEDIATO,
    TOKEN_NUMERO,
    TOKEN_COMA,
    TOKEN_CORCHETE_I,
    TOKEN_CORCHETE_D,
    TOKEN_DOSPUNTOS,
    TOKEN_SALTO_LINEA,
    TOKEN_MODIFICADOR,
    TOKEN_EOF,
    TOKEN_ERROR
} TipoToken;

#define MAX_TOKENS_POR_LINEA 16

// Tu estructura de Token optimizada
typedef struct {
    TipoToken tipo;
    char lexema[32];
    uint32_t valor;
    uint32_t linea;
    uint32_t columna;
} Token;

// Funciones del módulo
void inicializar_lexer(const char* dir_mem_codigo_fuente);
Token obtener_siguiente_token(void);
const char* tipo_token_a_string(TipoToken tipo);

int obtener_linea_de_tokens(Token* linea_tokens);

#endif // LEXER_H