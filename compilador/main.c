#include "helpers/compilador_helper.h"
#include "helpers/escritor_rom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MILO_COMPILADOR_DIR
#error "MILO_COMPILADOR_DIR no ha sido definido por el Makefile"
#endif


static char* cargar_archivo_fuente(const char* nombre_archivo)
{
    FILE* archivo = fopen(nombre_archivo, "rb");

    if (archivo == NULL) {
        fprintf(
            stderr,
            "❌ ERROR: No se pudo abrir el archivo fuente '%s'.\n",
            nombre_archivo
        );
        return NULL;
    }

    if (fseek(archivo, 0, SEEK_END) != 0) {
        fprintf(
            stderr,
            "❌ ERROR: No se pudo determinar el tamaño de '%s'.\n",
            nombre_archivo
        );
        fclose(archivo);
        return NULL;
    }

    long tamaño = ftell(archivo);

    if (tamaño < 0) {
        fprintf(
            stderr,
            "❌ ERROR: No se pudo determinar el tamaño de '%s'.\n",
            nombre_archivo
        );
        fclose(archivo);
        return NULL;
    }

    rewind(archivo);

    char* codigo_fuente = malloc((size_t)tamaño + 1);

    if (codigo_fuente == NULL) {
        fprintf(
            stderr,
            "❌ ERROR: No se pudo reservar memoria para '%s'.\n",
            nombre_archivo
        );
        fclose(archivo);
        return NULL;
    }

    size_t bytes_leidos = fread(
        codigo_fuente,
        1,
        (size_t)tamaño,
        archivo
    );

    fclose(archivo);

    if (bytes_leidos != (size_t)tamaño) {
        fprintf(
            stderr,
            "❌ ERROR: No se pudo leer completamente '%s'.\n",
            nombre_archivo
        );
        free(codigo_fuente);
        return NULL;
    }

    codigo_fuente[tamaño] = '\0';

    return codigo_fuente;
}


static const char* obtener_nombre_archivo(const char* ruta)
{
    const char* nombre = strrchr(ruta, '/');

    if (nombre != NULL) {
        return nombre + 1;
    }

    return ruta;
}


static void construir_nombre_salida(
    const char* archivo_fuente,
    char* nombre_salida,
    size_t tamaño_salida
)
{
    const char* nombre = obtener_nombre_archivo(archivo_fuente);

    snprintf(
        nombre_salida,
        tamaño_salida,
        "%s",
        nombre
    );

    char* extension = strrchr(nombre_salida, '.');

    if (extension != NULL) {
        *extension = '\0';
    }

    /*
     * El escritor ROM espera solamente el nombre del archivo,
     * no la ruta completa.
     */
    size_t longitud = strlen(nombre_salida);

    if (longitud + 4 < tamaño_salida) {
        strcat(nombre_salida, ".txt");
    }
}


int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(
            stderr,
            "Uso: milo <archivo.milo>\n"
        );
        return EXIT_FAILURE;
    }

    const char* archivo_fuente = argv[1];

    /*
     * La ruta es proporcionada por el Makefile durante la compilación.
     *
     * Ejemplo:
     *
     * /home/julieth/Documentos/MILO_CPU/compilador
     *
     * Desde ahí, las compilaciones se almacenan en:
     *
     * /home/julieth/Documentos/MILO_CPU/compilaciones
     */
    char directorio_compilaciones[512];

    int resultado_ruta = snprintf(
        directorio_compilaciones,
        sizeof(directorio_compilaciones),
        "%s/../compilaciones",
        MILO_COMPILADOR_DIR
    );

    if (
        resultado_ruta < 0 ||
        (size_t)resultado_ruta >= sizeof(directorio_compilaciones)
    ) {
        fprintf(
            stderr,
            "❌ ERROR: La ruta del directorio de compilaciones es demasiado larga.\n"
        );
        return EXIT_FAILURE;
    }

    establecer_directorio_compilaciones(
        directorio_compilaciones
    );

    char* codigo_fuente = cargar_archivo_fuente(
        archivo_fuente
    );

    if (codigo_fuente == NULL) {
        return EXIT_FAILURE;
    }

    char nombre_salida[256];

    construir_nombre_salida(
        archivo_fuente,
        nombre_salida,
        sizeof(nombre_salida)
    );

    int resultado = compilar_y_exportar(
        nombre_salida,
        codigo_fuente
    );

    free(codigo_fuente);

    return resultado;
}