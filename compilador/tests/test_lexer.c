#include <stdio.h>
#include "../lexer/lexer.h"

int main() {
    // Código de prueba de 10 líneas con casos estándar, límites y errores adrede
    const char* codigo_prueba = 
        "; Linea 1: Un comentario limpio inicial\n"
        "START:                 ; Linea 2: Etiqueta\n"
        "MOV R1, #10            ; Linea 3: Inmediato estandar\n"
        "mov r2, R1             ; Linea 4: Probar que vuelva mayusculas las minusculas\n"
        "ADD R3, R1, R2         ; Linea 5: Sintaxis con comas\n"
        "MOV R4, [R15]          ; Linea 6: Limite maximo de registros (R15)\n"
        "MOV R4, [R16]          ; Linea 7: ERROR ADREDE (R16 no existe, debe ser IDENTIFICADOR)\n"
        "DATA:                  ; Linea 8: Etiqueta\n"
        "MOV R5, #              ; Linea 9: ERROR ADREDE (Simbolo '#' huerfano)\n"
        "@                      ; Linea 10: ERROR ADREDE (Caracter invalido)\n"
        "ADD.F R2, R0, R1       ; Linea 11: Modificador valido adjunto (.F)\n"
        "SUB .F R3, R1, R2      ; Linea 12: Modificador separado por espacio (.F)\n"
        "NOT.F R8, R9           ; Linea 13: Modificador .F en operacion unaria\n"
        ".                      ; Linea 14: ERROR ADREDE (Punto huerfano sin letras)\n"
        ".123                   ; Linea 15: ERROR ADREDE (Punto seguido de numeros)\n"
        "CMP R1, R2             ; Linea 16: Instruccion de comparacion\n"
        "jmp START              ; Linea 17: Salto incondicional (Minusculas)\n"
        "JZ #0xab               ; Linea 18: Salto condicional con inmediato\n"
        "RET                    ; Linea 19: Retorno de subrutina sin argumentos\n"
        "call #50               ; Linea 20: Llamado a funcion\n"
        "NOT.F R8, R9           ; Linea 21: Modificador .F en operacion unaria\n"
        "JNZ.F #5               ; Linea 22: Modificador en salto (El Lexer lo tokeniza como IDENTIFICADOR + MODIFICADOR)\n"
        "MOV [R1], RAM          ; Linea 23: Lexema RAM y corchetes\n"
        "FIN_FN:                ; Linea 24: Identificador con barra baja\n"
        "JMP FIN_FN             ; Linea 25: Salto a identificador con barra baja\n";

    printf("====================================================================\n");
    printf("   🔥 EJECUTANDO TESTBENCH DE ANÁLISIS LÉXICO - MILO ASM 🔥        \n");
    printf("====================================================================\n\n");

    // Inicializamos el puntero en la memoria de la cadena de texto
    inicializar_lexer(codigo_prueba);

    Token t;
    int contador_tokens = 0;

    // Formato de tabla explícito para auditar cada salida
    printf("%-8s | %-6s | %-16s | %-12s | %-6s\n", "LINEA", "COL", "TIPO TOKEN", "LEXEMA", "VALOR");
    printf("--------------------------------------------------------------------\n");

    do {
        t = obtener_siguiente_token();
        contador_tokens++;

        // Imprimir cada campo del struct Token devuelto
        printf("[L:%02d]   | [C:%02d] | %-16s | '%-10s' | %-6u\n", 
               t.linea, 
               t.columna, 
               tipo_token_a_string(t.tipo), 
               t.tipo == TOKEN_SALTO_LINEA ? "\\n" : t.lexema, 
               t.valor);

    } while (t.tipo != TOKEN_EOF);

    printf("--------------------------------------------------------------------\n");
    printf("Análisis finalizado. Total de tokens procesados: %d\n", contador_tokens);
    printf("====================================================================\n");

    return 0;
}