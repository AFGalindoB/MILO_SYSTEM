#include "escritor_rom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exportar_a_txt_rom(const char* nombre_archivo, PalabraROM* palabras, uint32_t cantidad_instrucciones) {
    // Construir la ruta hacia la carpeta hermana anterior: ../compilaciones/nombre_archivo
    char ruta_completa[512] = {0};
    sprintf(ruta_completa, "../compilaciones/%s", nombre_archivo);

    // Abrir el archivo en modo escritura de texto
    FILE* archivo = fopen(ruta_completa, "w");
    if (archivo == NULL) {
        printf("❌ ERROR CRÍTICO: No se pudo crear el archivo en la ruta '%s'.\n", ruta_completa);
        printf(" Asegúrate de que la carpeta 'compilaciones' exista en el directorio raíz.\n");
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