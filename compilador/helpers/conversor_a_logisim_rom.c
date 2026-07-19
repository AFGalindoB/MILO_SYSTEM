#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PALABRAS_POR_LINEA 32
#define BUFFER_SIZE 64

int main(int argc, char *argv[]) {
    // Validar argumentos de la línea de comandos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_indices.txt> <archivo_logisim.txt>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s archivo.txt prueba\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ruta_origen = argv[1];
    char *ruta_salida = argv[2];

    FILE *archivo_txt = fopen(ruta_origen, "r");
    if (archivo_txt == NULL) {
        perror("Error al abrir el archivo de origen");
        return EXIT_FAILURE;
    }

    FILE *archivo_logisim = fopen(ruta_salida, "w");
    if (archivo_logisim == NULL) {
        perror("Error al crear el archivo de salida para Logisim");
        fclose(archivo_txt);
        return EXIT_FAILURE;
    }

    // 1. Escribir la cabecera obligatoria que exige Logisim Evolution v3.0
    fprintf(archivo_logisim, "v3.0 hex words addressed\n");

    char buffer[BUFFER_SIZE];
    unsigned int direccion_actual = 0;
    unsigned int palabras_en_linea_actual = 0;
    
    // Almacenamos temporalmente las 32 palabras de la línea actual
    char palabras_buffer[PALABRAS_POR_LINEA][16]; 

    // Leer cada línea del archivo exportado por la web (flujo continuo de datos hex)
    while (fgets(buffer, sizeof(buffer), archivo_txt) != NULL) {
        // Limpiar los saltos de línea (\n y \r)
        size_t longitud = strlen(buffer);
        while (longitud > 0 && (buffer[longitud - 1] == '\n' || buffer[longitud - 1] == '\r')) {
            buffer[longitud - 1] = '\0';
            longitud--;
        }

        // Ignorar líneas vacías por seguridad
        if (longitud == 0) {
            continue;
        }

        // Guardar el token/palabra hexadecimal en nuestro acumulador de línea
        strncpy(palabras_buffer[palabras_en_linea_actual], buffer, 15);
        palabras_buffer[palabras_en_linea_actual][15] = '\0'; // Asegurar terminador nulo
        palabras_en_linea_actual++;

        // Cuando acumulamos 32 palabras, escribimos una línea completa con formato Logisim
        if (palabras_en_linea_actual == PALABRAS_POR_LINEA) {
            fprintf(archivo_logisim, "%04x:", direccion_actual);
            for (int i = 0; i < PALABRAS_POR_LINEA; i++) {
                fprintf(archivo_logisim, " %s", palabras_buffer[i]);
            }
            fprintf(archivo_logisim, "\n");

            // Avanzar la dirección base física (+32 en decimal, que equivale a +0x20 en hex)
            direccion_actual += PALABRAS_POR_LINEA;
            palabras_en_linea_actual = 0;
        }
    }

    // 2. Manejo del residuo: Si el archivo terminó pero quedaron palabras colgadas sin completar el bloque de 32
    if (palabras_en_linea_actual > 0) {
        fprintf(archivo_logisim, "%04x:", direccion_actual);
        // Escribir las palabras válidas leídas
        for (unsigned int i = 0; i < palabras_en_linea_actual; i++) {
            fprintf(archivo_logisim, " %s", palabras_buffer[i]);
        }
        // Rellenar con ceros el espacio restante hasta completar las 32 exigidas por el formato visual
        for (unsigned int i = palabras_en_linea_actual; i < PALABRAS_POR_LINEA; i++) {
            fprintf(archivo_logisim, " 0");
        }
        fprintf(archivo_logisim, "\n");
        direccion_actual += PALABRAS_POR_LINEA;
    }

    // Cerrar archivos
    fclose(archivo_txt);
    fclose(archivo_logisim);

    printf("✨ Formateo completado. Archivo '%s' listo para cargar en Logisim ROM/RAM.\n", ruta_salida);
    printf("📊 Direcciones generadas desde 0000 hasta %04x.\n", direccion_actual - 1);

    return EXIT_SUCCESS;
}