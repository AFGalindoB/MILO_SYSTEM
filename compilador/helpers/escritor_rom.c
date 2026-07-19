#include "escritor_rom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

void exportar_a_txt_rom(const char* nombre_archivo, PalabraROM* palabras, uint32_t cantidad_instrucciones) {
    // 1. Intentar crear la carpeta base "../compilaciones" con permisos de lectura/escritura/ejecución (0777)
    if (mkdir("../compilaciones", 0777) == -1) {
        if (errno != EEXIST) {
            printf("❌ ERROR CRÍTICO: No se pudo crear el directorio base '../compilaciones'.\n");
            exit(EXIT_FAILURE);
        }
    }

    // 2. Intentar crear la subcarpeta "instrucciones" dentro de compilaciones
    if (mkdir("../compilaciones/instrucciones", 0777) == -1) {
        if (errno != EEXIST) {
            printf("❌ ERROR CRÍTICO: No se pudo crear el subdirectorio '../compilaciones/instrucciones'.\n");
            exit(EXIT_FAILURE);
        }
    }


    char ruta_completa[512] = {0};
    sprintf(ruta_completa, "../compilaciones/instrucciones/%s", nombre_archivo);

    // Abrir el archivo en modo escritura de texto
    FILE* archivo = fopen(ruta_completa, "w");
    if (archivo == NULL) {
        printf("❌ ERROR CRÍTICO: No se pudo crear el archivo en la ruta final '%s'.\n", ruta_completa);
        exit(EXIT_FAILURE);
    }

    // Recorrer secuencialmente el mapa de memoria del programa
    for (uint32_t i = 0; i < cantidad_instrucciones; i++) {
        // Imprimir CW (Bits 63:32) seguido inmediatamente de IMM (Bits 31:0) sin espacios, terminando con salto de línea
        fprintf(archivo, "%08X%08X\n", palabras[i].control_word, palabras[i].immediate);
    }

    // Cerrar el descriptor de archivo para asegurar el vaciado del buffer de disco
    fclose(archivo);
    printf("💾 Compilación exitosa exportada a: %s\n", ruta_completa);
}