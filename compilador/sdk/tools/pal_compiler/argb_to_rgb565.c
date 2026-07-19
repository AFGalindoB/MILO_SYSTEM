#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PALETTE_COLORS 16

// ---- 1. CONVERSIÓN MATEMÁTICA PURA ----
uint16_t convertir_argb_a_rgb565(uint32_t argb) {
    uint8_t r8 = (argb >> 16) & 0xFF;
    uint8_t g8 = (argb >> 8)  & 0xFF;
    uint8_t b8 = (argb)       & 0xFF;

    uint16_t r5 = (r8 >> 3) & 0x1F;
    uint16_t g6 = (g8 >> 2) & 0x3F;
    uint16_t b5 = (b8 >> 3) & 0x1F;

    return (r5 << 11) | (g6 << 5) | b5;
}

// ---- 2. GENERADOR DE ARCHIVO E INFRAESTRUCTURA ----
// Esta función solo se preocupa por la persistencia. Recibe el buffer limpio.
void guardar_buffer_a_archivo(const char* nombre_salida, uint16_t* buffer_colores, int cantidad) {
    // Asegurar la existencia de las carpetas
    if (mkdir("../../compilaciones", 0777) == -1 && errno != EEXIST) {
        printf("❌ ERROR CRÍTICO: No se pudo crear el directorio base '../../compilaciones'.\n");
        exit(EXIT_FAILURE);
    }

    if (mkdir("../../compilaciones/paletas_de_color", 0777) == -1 && errno != EEXIST) {
        printf("❌ ERROR CRÍTICO: No se pudo crear el subdirectorio '../../compilaciones/paletas_de_color'.\n");
        exit(EXIT_FAILURE);
    }

    // Construir ruta y escribir
    char ruta_completa[512] = {0};
    sprintf(ruta_completa, "../../compilaciones/paletas_de_color/%s", nombre_salida);

    FILE* salida = fopen(ruta_completa, "w");
    if (salida == NULL) {
        printf("❌ ERROR CRÍTICO: No se pudo crear el archivo de salida en '%s'.\n", ruta_completa);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cantidad; i++) {
        fprintf(salida, "%04X\n", buffer_colores[i]);
    }

    fclose(salida);
    printf("💾 Paleta guardada exitosamente en: %s (%d colores)\n", ruta_completa, cantidad);
}

// ---- 3. PUNTO DE ENTRADA Y LOGICA DE PARSEO ----
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <archivo_origen.txt> <nombre_salida.txt>\n", argv[0]);
        return 1;
    }

    FILE* entrada = fopen(argv[1], "r");
    if (entrada == NULL) {
        printf("❌ ERROR: No se pudo abrir el archivo de entrada '%s'.\n", argv[1]);
        return 1;
    }

    // Buffer intermedio para aislar la lectura de la escritura
    uint16_t buffer_paleta[MAX_PALETTE_COLORS];
    int colores_procesados = 0;
    char linea[32];

    // Bucle de procesamiento y validación estricta del formato hex
    while (fgets(linea, sizeof(linea), entrada) && colores_procesados < MAX_PALETTE_COLORS) {
        linea[strcspn(linea, "\r\n")] = 0; // Limpiar saltos de línea
        if (strlen(linea) == 0) continue;

        // Validar longitud exacta de 8 caracteres hexadecimales
        if (strlen(linea) != 8) {
            printf("Error: El color '%s' debe tener exactamente 8 caracteres hexadecimales (AARRGGBB).\n", linea);
            continue; 
        }

        // Parsear la cadena a un entero de 32 bits
        uint32_t color_argb = 0;
        if (sscanf(linea, "%X", &color_argb) != 1) {
            printf("Error: Formato hexadecimal inválido '%s'.\n", linea);
            continue;
        }

        // Convertir usando la función pura e inyectar al buffer
        buffer_paleta[colores_procesados] = convertir_argb_a_rgb565(color_argb);
        colores_procesados++;
    }
    fclose(entrada);

    if(colores_procesados == MAX_PALETTE_COLORS){
        printf("Advertencia: la paleta puede contiener más de %d colores.\n", MAX_PALETTE_COLORS);
    }

    // Si logramos procesar colores válidos, mandamos el buffer completo a escribir
    if (colores_procesados > 0) {
        guardar_buffer_a_archivo(argv[2], buffer_paleta, colores_procesados);
    } else {
        printf("⚠️ No se encontraron colores válidos para exportar.\n");
    }

    return 0;
}