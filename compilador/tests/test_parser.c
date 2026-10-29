#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../helpers/tag_manager.h"

#define PROBAR_ERROR_SINTACTICO true

// Helper estático para obtener el nombre legible del tipo de instrucción
static const char* obtener_nombre_instruccion(TipoInstruccion tipo) {
    switch (tipo) {
        case INSTR_NOP:   return "NOP";
        case INSTR_MOV:   return "MOV";
        case INSTR_ADD:   return "ADD";
        case INSTR_SUB:   return "SUB";
        case INSTR_ADC:   return "ADC";
        case INSTR_SBC:   return "SBC";
        case INSTR_AND:   return "AND";
        case INSTR_OR:    return "OR";
        case INSTR_XOR:   return "XOR";
        case INSTR_NOT:   return "NOT";
        case INSTR_SHL:   return "SHL";
        case INSTR_SHR:   return "SHR";

        // --- Control de Flujo ---
        case INSTR_CMP:   return "CMP";
        case INSTR_JMP:   return "JMP";
        case INSTR_CALL:  return "CALL";
        case INSTR_RET:   return "RET";
        case INSTR_JZ:    return "JZ";
        case INSTR_JNZ:   return "JNZ";
        case INSTR_JC:    return "JC";
        case INSTR_JNC:   return "JNC";
        case INSTR_JN:    return "JN";
        case INSTR_JNN:   return "JNN";
        case INSTR_JV:    return "JV";
        case INSTR_JNV:   return "JNV";

        default:          return "DESCONOCIDA";
    }
}

// Auxiliar mejorado para formatear un operando de manera contextual
static void imprimir_operando(InstruccionIR* ir, Operando op) {
    switch (op.tipo) {
        case OPERANDO_REG:
            printf("R%d", op.valor);
            break;
        case OPERANDO_INMEDIATO:
            printf("#%d", ir->inmediato.valor);
            break;
        case OPERANDO_RAM:
            // Si el operando es RAM, mostramos cómo se direccionó si hay información de MAR
            if (ir->MAR_Control.tipo != OPERANDO_NULO) {
                printf("[");
                imprimir_operando(ir, ir->MAR_Control);
                printf("]");
            } else {
                printf("RAM");
            }
            break;
        case OPERANDO_NULO:
            printf("NONE");
            break;
    }
}

// Imprime de forma especializada según la categoría de la instrucción
static void desempaquetar_e_imprimir(InstruccionIR* instr) {
    const char* nombre = obtener_nombre_instruccion(instr->tipo);
    printf("[Línea %02d] Parsed: %-5s", instr->linea, nombre);

    // Agregar modificador .F si la instrucción actualiza banderas
    if (instr->tiene_modificador_f && instr->tipo != INSTR_CMP) {
        printf(".F");
    } else {
        printf("  ");
    }

    printf(" | ");

    switch (instr->formato) {

        case FORMATO_SIN_OPERANDOS:
            // Caso NOP, RET
            printf("Tipo: [Sin Operandos]");
            break;

        case FORMATO_SALTO:
            // Caso JMP, JZ, CALL, etc.
            printf("Destino Salto: [");
            if (instr->inmediato.valor != 0 || instr->fuente[0].tipo == OPERANDO_INMEDIATO) {
                printf("#%d", instr->inmediato.valor);
            } else {
                printf("Dirección/Etiqueta");
            }
            printf("]");
            break;

        case FORMATO_MOV:
            // Caso MOV Destino, Fuente
            printf("Destino: [");
            imprimir_operando(instr, instr->destino);
            printf("] <- Fuente: [");
            imprimir_operando(instr, instr->fuente[0]);
            printf("]");
            break;

        case FORMATO_ALU_UNARIO:
            // Caso NOT
            printf("Destino: [");
            imprimir_operando(instr, instr->destino);
            printf("] <- Operando: [");
            imprimir_operando(instr, instr->fuente[0]);
            printf("]");
            break;

        case FORMATO_ALU_BINARIO:
            // Caso CMP (sin destino) vs Operaciones Aritmético-Lógicas (con destino)
            if (instr->tipo == INSTR_CMP) {
                printf("Comparación: [");
                imprimir_operando(instr, instr->fuente[0]);
                printf("] vs [");
                imprimir_operando(instr, instr->fuente[1]);
                printf("]");
            } else {
                printf("Destino: [");
                imprimir_operando(instr, instr->destino);
                printf("] <- Ops: [");
                imprimir_operando(instr, instr->fuente[0]);
                printf(", ");
                imprimir_operando(instr, instr->fuente[1]);
                printf("]");
            }
            break;

        default:
            printf("Tipo: [Desconocido]");
            break;
    }

    // Información extra para depuración de accesos a memoria (MAR)
    if (instr->MAR_Control.tipo != OPERANDO_NULO && instr->formato != FORMATO_MOV) {
        printf(" | MAR Direccionado por: [");
        if (instr->MAR_Control.tipo == OPERANDO_REG) {
            printf("R%d", instr->MAR_Control.valor);
        } else {
            printf("#%d", instr->inmediato.valor);
        }
        printf("]");
    }

    printf("\n");
}

int main() {
    // 1. Único código de prueba que contiene las nuevas incorporaciones (ADC, SBC, modificadores .F)
    const char* codigo_prueba = 
    #if !PROBAR_ERROR_SINTACTICO
        "; ---- Bloque de Pruebas Exitosas (Milo ISA v2.0.0) ----\n"
        "NOP\n"
        "MOV R1, #25                 ; Reemplazo de MOVI: Inmediato a registro decimal\n"
        "MOV R3, #0x12               ; Reemplazo de MOVI: Inmediato a registro hexadecimal\n"
        "MOV [#0x12], #0x12          ; Escritura de inmediato a memoria (Dir inmediata)\n"
        "MOV R2, R1                  ; MOV estándar: Registro a registro\n"
        "MOV R3, [#0x10]             ; Reemplazo de LOAD: Lectura desde memoria (Dir inmediata)\n"
        "MOV [R1], R2                ; Reemplazo de STORE: Escritura a memoria (Dir por registro)\n"
        "MOV [R0], #15               ; Escritura de inmediato a memoria\n"
        "MOV [#5], RAM               ; Transferencia dummy (sintácticamente válida en ISA)\n"
        "ADD.F R3, R1, R2            ; ALU Binaria estándar con .F y comas\n"
        "SUB R0, R3, R1              ; ALU Binaria sin modificar banderas\n"
        "ADC.F R4, R2, R3            ; Suma con acarreo y actualización de flags\n"
        "SBC R5, R4, R1              ; Resta con acarreo\n"
        "ADD R0, [R1], R2            ; ALU: Memoria como Operando A\n"
        "ADD [R0], R1, RAM           ; ALU: Escritura en memoria y uso como Operando B\n"
        "ADD [#0x10], RAM, RAM       ; ALU: Uso de RAM múltiple tras definir MAR\n"
        "NOT.F R6, R2                ; ALU Unaria estándar con flags\n"
        "NOT [R7], RAM               ; ALU Unaria: Modificación in-place en memoria\n"
        "CMP R1, R2                  ; Comparación básica (A - B) descartando resultado\n"
        "JZ #16                      ; Salto condicional si Z=1\n"
        "CALL MI_FUNCION             ; Llamada a subrutina usando etiqueta\n"
        "CALL #50                    ; Llamada a subrutina usando inmediato\n"
        "MI_FUNCION:                 ; Declaración de etiqueta\n"
        "RET                         ; Retorno de subrutina\n"
        "JNZ #8                      ; Salto condicional si Z=0\n"
        ;
    #else
        "; ---- Bloque de Pruebas con Errores Sintácticos (Milo ISA v2.0.0) ----\n"
        "ADD R1 R2 R3         ; Error 1: Falta coma obligatoria entre operandos\n"
        "MOV R2, R1, R3       ; Error 2: MOV solo acepta un destino y un fuente\n"
        "MOV R2,              ; Error 3: Operando faltante tras la coma\n"
        "MOV [#0x01], #15     ; Error 4: No se permite declarar distintos valores inmediatos en la misma instrucción\n"
        "INVENTADO R1, R2     ; Error 5: Mnemónico ilegal / desconocido\n"
        "NOT R5, R6, R7       ; Error 6: NOT es unaria, no acepta segundo operando fuente\n"
        "ADD.F.G R8, R1, R2   ; Error 7: Doble modificador inválido en el lexema\n"
        "ADD.X R1, R2, R3     ; Error 8: Modificador de instrucción inexistente (.X)\n"
        ".                    ; Error 9: Modificador aislado / inválido\n"
        "CMP R1               ; Error 10: CMP requiere estrictamente dos operandos (A, B)\n"
        "JMP                  ; Error 11: JMP requiere una dirección destino (Etiqueta o Inmediato)\n"
        "JZ R1                ; Error 12: Saltos condicionales requieren Inmediato o Etiqueta, no un Registro\n"
        "RET R1               ; Error 13: RET no acepta parámetros\n"
        "ADD RAM, RAM, RAM    ; Error 14: Uso de RAM sin definir dirección previa con [...] (MAR nulo)\n"
        "ADD [R1], [R2], R3   ; Error 15: Violación arquitectónica, doble acceso a direcciones distintas\n"
        "ADD R0, #5, #10      ; Error 16: No se permite declarar distintos valores inmediatos en la misma instrucción\n"
        "FIN: NOP             ; Error 17: Tokens adicionales tras la declaración de una etiqueta\n"
        "MOV [R8], #15        ; Error 18: Registro inválido dentro de '[...]'. Solo se permiten R0 a R7.\n"
        "MOV.G [#0x01], R1    ; Error 19: MOV no tiene modificadores\n"
        ;
    #endif

    printf("====================================================================\n");
    printf("   🔥 EJECUTANDO TESTBENCH DEL PARSER - MILO ASM 🔥        \n");
    printf("====================================================================\n\n");

    pre_escaneo_etiquetas(codigo_prueba);

    // Inicializar el lexer con el código fuente en memoria
    inicializar_lexer(codigo_prueba);

    Token tokens_linea[MAX_TOKENS_POR_LINEA];
    int cantidad_tokens = 0;
    int lineas_procesadas = 0;
    int es_eof = 0;

    // Ciclo principal: Extrae líneas de tokens completas y las envía al parser
    while (!es_eof) {
        cantidad_tokens = obtener_linea_de_tokens(tokens_linea);

        if (cantidad_tokens == 0) {
            break;
        }

        // Verificar si la línea termina en EOF para salir del bucle al terminar de parsear
        if (tokens_linea[cantidad_tokens - 1].tipo == TOKEN_EOF) {
            es_eof = 1;
        }

        InstruccionIR instr = parsear_linea_tokens(tokens_linea, cantidad_tokens);

        // Si la línea tenía contenido válido (no era comentario ni línea vacía)
        if (instr.tipo != INSTR_DESCONOCIDA) {
            lineas_procesadas++;
            desempaquetar_e_imprimir(&instr);
        }
    }

    // Emitir el reporte gráfico de errores
    emitir_informe_compilacion();

    printf("\n====================================================================\n");
    printf(" Análisis sintáctico completado. %d instrucciones procesadas.\n", lineas_procesadas);
    printf("====================================================================\n");

    return (contador_errores > 0) ? 1 : 0;
}