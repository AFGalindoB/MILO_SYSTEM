#include <stdio.h>
#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"

// Cambia esto a 'true' para inyectar un error y probar el pánico de salida crítica
#define PROBAR_ERROR_SINTACTICO true

int main() {
    const char* codigo_prueba = 
#if !PROBAR_ERROR_SINTACTICO
        "; ---- Bloque de Pruebas Exitosas ----\n"
        "NOP                 ; Línea 2: Instrucción vacía\n"
        "MOVI R1, #25        ; Línea 3: Carga de inmediato\n"
        "MOV R2, R1          ; Línea 4: Copia de registro\n"
        "ADD R3, R1 R2       ; Línea 5: Operación ALU sin coma intermedia\n"
        "SUB R0, R3 R1       ; Línea 6: Otra operación ALU\n"
        "LOAD R4, [R1]       ; Línea 7: Transferencia desde RAM usando MAR\n"
        "STORE R2, [R5]      ; Línea 8: Escritura en RAM\n"
        "AND R10, R4 R0      ; Línea 9: Operación lógica\n"
        "XOR R15, R2 R2      ; Línea 10: Limpieza de registro";
#else
        "@ R1, #100\n"
        "ADD R2, R1, R3      ; <-- ERROR: Coma extra entre operandos ALU\n"
        "NOP";
#endif

    printf("====================================================================\n");
    printf("   🔥 EJECUTANDO TESTBENCH DEL PARSER - CPU ALPHA 🔥        \n");
    printf("====================================================================\n\n");

    // 1. Inicializar el hardware de software (Lexer)
    inicializar_lexer(codigo_prueba);
    
    // 2. Cargar el primer token en el bus global del Parser antes de arrancar
    avanzar_token();

    int lineas_procesadas = 0;

    // 3. Ciclo principal del compilador por líneas
    while (!match(TOKEN_EOF)) {
        lineas_procesadas++;
        
        // Ejecutar el análisis de la línea actual
        InstruccionParseada instr = parsear_linea();

        // Si la línea estaba vacía (solo comentarios o saltos de línea huérfanos)
        if (instr.tipo == INSTR_DESCONOCIDA && match(TOKEN_SALTO_LINEA)) {
            continue; 
        }

        // Mostrar el desempaquetado de la estructura final estructurada en la unión
        printf("[Línea %02d] Parsed con éxito. Tipo de Instrucción: ", instr.linea);
        
        switch (instr.tipo) {
            case INSTR_NOP:
                printf("NOP\n");
                break;
            case INSTR_MOVI:
                printf("MOVI -> Rd: R%d, Inmediato: #%u\n", instr.operandos.movi.rd, instr.operandos.movi.valor);
                break;
            case INSTR_MOV:
                printf("MOV  -> Rd: R%d, Rs: R%d\n", instr.operandos.mov.rd, instr.operandos.mov.rs);
                break;
            case INSTR_ADD:
                printf("ADD  -> Rd: R%d, Ra: R%d, Rb: R%d\n", instr.operandos.alu.rd, instr.operandos.alu.ra, instr.operandos.alu.rb);
                break;
            case INSTR_SUB:
                printf("SUB  -> Rd: R%d, Ra: R%d, Rb: R%d\n", instr.operandos.alu.rd, instr.operandos.alu.ra, instr.operandos.alu.rb);
                break;
            case INSTR_AND:
                printf("AND  -> Rd: R%d, Ra: R%d, Rb: R%d\n", instr.operandos.alu.rd, instr.operandos.alu.ra, instr.operandos.alu.rb);
                break;
            case INSTR_XOR:
                printf("XOR  -> Rd: R%d, Ra: R%d, Rb: R%d\n", instr.operandos.alu.rd, instr.operandos.alu.ra, instr.operandos.alu.rb);
                break;
            case INSTR_LOAD:
                printf("LOAD -> Rd: R%d, Dirección Base (MAR): [R%d]\n", instr.operandos.load.rd, instr.operandos.load.mar);
                break;
            case INSTR_STORE:
                printf("STORE-> MDR: R%d, Dirección Base (MAR): [R%d]\n", instr.operandos.store.mdr, instr.operandos.store.mar);
                break;
            default:
                printf("Desconocida o Vacía\n");
                break;
        }
    }

    printf("\n====================================================================\n");
    printf(" Análisis sintáctico completado. %d líneas validadas.\n", lineas_procesadas);
    printf(" Estado del sistema: EXCELENTE (0 errores sintácticos).\n");
    printf("====================================================================\n");

    return 0;
}