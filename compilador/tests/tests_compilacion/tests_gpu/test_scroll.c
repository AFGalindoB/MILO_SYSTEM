#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../helpers/compilador_helper.h"
#include "../../../helpers/compile_text.h"

int main() {

    const char* texto =
        "lorem ipsum dolor sit amet consectetur adipiscing elit sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua ut "
        "enim ad minim veniam quis nostrud exercitation ullamco laboris "
        "nisi ut aliquip ex ea commodo consequat duis aute irure dolor "
        "in reprehenderit in voluptate velit esse cillum dolore eu fugiat "
        "nulla pariatur excepteur sint occaecat cupidatat non proident "
        "sunt in culpa qui officia deserunt mollit anim id est laborum "
        "curabitur pretium tincidunt lacus nulla gravida orci a odio "
        "nullam varius turpis et commodo pharetra est eros bibendum elit "
        "nec luctus magna felis sollicitudin mauris integer in mauris "
        "eu nibh euismod gravida duis ac tellus et risus vulputate "
        "vehicula donec lobortis risus a elit aliquam tempus";

    const char* codigo_scroll =
        "\n"
        "; -----------------------------------------\n"
        "; Configuracion e Inicio de Scroll\n"
        "; -----------------------------------------\n"
        "MOVI R0, #512\n"
        "MOVI R1, #0\n"
        "\n"
        "; -----------------------------------------\n"
        "; Bucle de Animacion de Scroll\n"
        "; -----------------------------------------\n"
        "LOOP_SCROLL:\n"
        "    WAITV\n"
        "    ADD SCROLL, R1, R1, R0\n"
        "    JMP LOOP_SCROLL\n";

    char* codigo_tbuf = generar_tbuf_texto(texto);

    if (!codigo_tbuf) {
        fprintf(stderr, "Error generando el codigo ASM para el texto.\n");
        return 1;
    }

    size_t tamano = strlen(codigo_tbuf) + strlen(codigo_scroll) + 1;
    char* codigo_prueba = malloc(tamano);

    if (!codigo_prueba) {
        fprintf(stderr, "Error reservando memoria para el programa.\n");

        free(codigo_tbuf);
        return 1;
    }

    strcpy(codigo_prueba, codigo_tbuf);
    strcat(codigo_prueba, codigo_scroll);

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN GPU - SCROLL 🔥\n");
    printf("====================================================================\n\n");

    int resultado =
        compilar_y_exportar("test_gpu_scroll.txt", codigo_prueba);

    printf("====================================================================\n");

    free(codigo_tbuf);
    free(codigo_prueba);

    return resultado;
}