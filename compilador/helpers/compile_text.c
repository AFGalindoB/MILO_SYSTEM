#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* SPRITE_CHARS =
    " abcdefghijklmnopqrstuvwxyz";

char* generar_tbuf_texto(const char* texto) {

    // Validación de entrada
    if (!texto)
        return NULL;

    // Calcular la capacidad necesaria para el código generado
    size_t capacidad = strlen(texto) * 64 + 1;
    char* codigo = malloc(capacidad);

    // Validación de memoria
    if (!codigo)
        return NULL;

    codigo[0] = '\0';

    for (size_t posicion = 0; texto[posicion] != '\0'; posicion++) {

        const char* sprite = strchr(
            SPRITE_CHARS,
            texto[posicion]
        );

        if (!sprite) {
            fprintf(
                stderr,
                "ERROR: caracter '%c' no tiene un sprite valido.\n",
                texto[posicion]
            );

            free(codigo);
            return NULL;
        }

        unsigned int tile =
            (unsigned int)(sprite - SPRITE_CHARS);

        unsigned int valor =
            (tile << 12) | posicion;

        char linea[128];

        snprintf(
            linea,
            sizeof(linea),
            "MOVI TBUF, #%u ; '%c' -> Tile %02X, Pos %03zX\n",
            valor,
            texto[posicion],
            tile,
            posicion
        );

        strcat(codigo, linea);
    }

    return codigo;
}