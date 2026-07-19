#include <stdio.h>
#include "../../helpers/compilador_helper.h"

int main() {
    const char* codigo_prueba = 
        "TBUF #0 #8\n"    
        "TBUF #1 #15\n"
        "TBUF #2 #12\n"
        "TBUF #3 #1\n"
        "TBUF #4 #0\n"
        "TBUF #5 #13\n"
        "TBUF #6 #21\n"
        "TBUF #7 #14\n"
        "TBUF #8 #4\n"
        "TBUF #9 #15\n"
        "TILEOFF #1 #0\n"
        ;

    printf("====================================================================\n");
    printf("   🔥 TEST: COMPILACIÓN DE GPU 🔥 \n");
    printf("====================================================================\n\n");

    int resultado = compilar_y_exportar("test_GPU.txt", codigo_prueba);

    printf("====================================================================\n");
    return resultado;
}