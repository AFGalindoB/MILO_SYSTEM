#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"

#define PROBAR_ERROR_SINTACTICO true

// Helper estático para obtener el nombre legible del tipo de instrucción
static const char* obtener_nombre_instruccion(TipoInstruccion tipo) {
    switch (tipo) {
        case INSTR_NOP:   return "NOP";
        case INSTR_MOV:   return "MOV";
        case INSTR_MOVI:  return "MOVI";
        case INSTR_LOAD:  return "LOAD";
        case INSTR_STORE: return "STORE";
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
        default:          return "DESCONOCIDA";
    }
}

// Función auxiliar para desempaquetar e imprimir de forma genérica
static void desempaquetar_e_imprimir(InstruccionParseada* instr) {
    const char* nombre = obtener_nombre_instruccion(instr->tipo);
    printf("[Línea %02d] Parsed: %-5s | ", instr->linea, nombre);

    switch (instr->tipo) {
        case INSTR_NOP:
            printf("Sin operandos\n");
            break;

        // ALU Binarias (Rd, Ra, Rb) + update_flags
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_ADC:
        case INSTR_SBC:
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_XOR:
        case INSTR_SHL:
        case INSTR_SHR:
            printf("Categoría: ALU_BINARIA | Rd: R%d, Ra: R%d, Rb: R%d | UpdateFlags: %d\n",
                   instr->operandos.alu.rd,
                   instr->operandos.alu.ra,
                   instr->operandos.alu.rb,
                   instr->operandos.alu.update_flags);
            break;

        // ALU Unarias (Rd, Ra) + update_flags
        case INSTR_NOT:
            printf("Categoría: ALU_UNARIA  | Rd: R%d, Ra: R%d | UpdateFlags: %d\n",
                   instr->operandos.alu_unaria.rd,
                   instr->operandos.alu_unaria.ra,
                   instr->operandos.alu_unaria.update_flags);
            break;

        // Movimientos simples (Rd, Rs)
        case INSTR_MOV:
            printf("Categoría: MOVIMIENTO  | Rd: R%d, Rs: R%d\n",
                   instr->operandos.mov.rd,
                   instr->operandos.mov.rs);
            break;

        // Movimientos inmediatos (Rd, valor)
        case INSTR_MOVI:
            printf("Categoría: MOV_IMMED   | Rd: R%d, Inmediato: #%u\n",
                   instr->operandos.movi.rd,
                   instr->operandos.movi.valor);
            break;

        // Cargas de memoria (Rd, [mar])
        case INSTR_LOAD:
            printf("Categoría: MEM_LOAD    | Rd: R%d, MAR: [R%d]\n",
                   instr->operandos.load.rd,
                   instr->operandos.load.mar);
            break;

        // Escrituras en memoria (mdr, [mar])
        case INSTR_STORE:
            printf("Categoría: MEM_STORE   | MDR: R%d, MAR: [R%d]\n",
                   instr->operandos.store.mdr,
                   instr->operandos.store.mar);
            break;

        default:
            printf("Estructura vacía o inválida\n");
            break;
    }
}

int main() {
    // 1. Único código de prueba que contiene las nuevas incorporaciones (ADC, SBC, modificadores .F)
    const char* codigo_prueba = 
    #if !PROBAR_ERROR_SINTACTICO
        "; ---- Bloque de Pruebas Exitosas ----\n"
        "NOP\n"
        "MOVI R1, #25\n"
        "MOV R2, R1\n"
        "ADD.F R3, R1 R2     ; ALU Binaria estándar con actualizador de banderas (.F)\n"
        "SUB R0, R3 R1       ; ALU Binaria sin modificar banderas\n"
        "ADC.F R4, R2 R3     ; Nuevo mnemónico: Suma con acarreo (con .F)\n"
        "SBC R5, R4 R1       ; Nuevo mnemónico: Resta con acarreo (sin .F)\n"
        "NOT.F R6, R2        ; ALU Unaria con actualizador de banderas (.F)\n"
        "NOT R7, R3          ; ALU Unaria estándar sin .F\n"
        "LOAD R8, [R1]\n"
        "STORE R8, [R5]\n";
    #else
        "; ---- Bloque de Pruebas con Errores Sintácticos ----\n"
        "ADD R1 R2 R3        ; Error 1: Falta coma obligatoria\n"
        "MOVI R2,             ; Error 2: Inmediato vacío\n"
        "STORE R3, R4         ; Error 3: Store sin corchetes\n"
        "NOP                  ; ¡Línea correcta que se sincronizará tras el error anterior!\n"
        "INVENTADO R1, R2     ; Error 4: Mnemónico ilegal\n"
        "NOT R5, R6 R7        ; Error 5: NOT no acepta segundo operando fuente\n"
        "ADD.F.G R8, R1 R2    ; Error 6: Doble modificador inválido\n"
        "LOAD R1, R2          ; Error 7: LOAD sin corchetes\n"
        "ADD.G R1, R2 R3      ; Error 8: Modificador inexistente\n"
        ".                    ; Error 9: Modificador invalido\n";
    #endif

    printf("====================================================================\n");
    printf("   🔥 EJECUTANDO TESTBENCH DEL PARSER - MILO ASM 🔥        \n");
    printf("====================================================================\n\n");

    // Inicializar el lexer y arrancar el pipeline de tokens
    inicializar_lexer(codigo_prueba);
    avanzar_token();

    contador_errores = 0;
    int lineas_procesadas = 0;

    // Ciclo principal de parsing estructurado
    while (!match(TOKEN_EOF)) {
        lineas_procesadas++;
        
        InstruccionParseada instr = parsear_linea();

        // Evitar procesar líneas sin instrucciones (comentarios o líneas vacías)
        if (instr.tipo == INSTR_DESCONOCIDA) {
            if (match(TOKEN_SALTO_LINEA)) {
                avanzar_token();
            }
            continue; 
        }

        // Desempaquetado genérico del AST sin requerir switches gigantes en la rutina de test
        desempaquetar_e_imprimir(&instr);

        if (match(TOKEN_SALTO_LINEA)) {
            avanzar_token();
        }
    }

    emitir_informe_compilacion();

    printf("\n====================================================================\n");
    printf(" Análisis sintáctico completado. %d instrucciones revisadas.\n", lineas_procesadas);
    printf("====================================================================\n");

    return (contador_errores > 0) ? 1 : 0;
}