#include "escritor_rom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>

#define MAX_RUTA 512

static char directorio_compilaciones[MAX_RUTA] = {0};

void establecer_directorio_compilaciones(const char* directorio)
{
    if (directorio == NULL) {
        fprintf(stderr,
                "❌ ERROR: Directorio de compilaciones inválido.\n");
        exit(EXIT_FAILURE);
    }

    int resultado = snprintf(
        directorio_compilaciones,
        sizeof(directorio_compilaciones),
        "%s",
        directorio
    );

    if (resultado < 0 || resultado >= MAX_RUTA) {
        fprintf(stderr,
                "❌ ERROR: La ruta del directorio de compilaciones es demasiado larga.\n");
        exit(EXIT_FAILURE);
    }
}


void exportar_a_txt_rom(
    const char* nombre_archivo,
    uint64_t* palabras,
    uint32_t cantidad_instrucciones
)
{
    if (directorio_compilaciones[0] == '\0') {
        fprintf(stderr,
                "❌ ERROR: No se ha establecido el directorio de compilaciones.\n");
        exit(EXIT_FAILURE);
    }

    char directorio_instrucciones[MAX_RUTA] = {0};

    snprintf(
        directorio_instrucciones,
        sizeof(directorio_instrucciones),
        "%s/instrucciones",
        directorio_compilaciones
    );

    // Crear directorio base
    if (mkdir(directorio_compilaciones, 0777) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr,
                    "❌ ERROR: No se pudo crear el directorio '%s'.\n",
                    directorio_compilaciones);
            exit(EXIT_FAILURE);
        }
    }

    // Crear directorio de instrucciones
    if (mkdir(directorio_instrucciones, 0777) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr,
                    "❌ ERROR: No se pudo crear el directorio '%s'.\n",
                    directorio_instrucciones);
            exit(EXIT_FAILURE);
        }
    }

    char ruta_completa[MAX_RUTA] = {0};

    snprintf(
        ruta_completa,
        sizeof(ruta_completa),
        "%s/%s",
        directorio_instrucciones,
        nombre_archivo
    );

    FILE* archivo = fopen(ruta_completa, "w");

    if (archivo == NULL) {
        fprintf(stderr,
                "❌ ERROR: No se pudo crear el archivo '%s'.\n",
                ruta_completa);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < cantidad_instrucciones; i++) {
        fprintf(archivo, "*%016" PRIX64 "\n", palabras[i]);
    }

    fclose(archivo);

    printf(
        "💾 Compilación exportada a: %s\n",
        ruta_completa
    );
}