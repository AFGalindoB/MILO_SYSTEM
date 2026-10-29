#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ETIQUETAS 200
#define TAMANO_NOMBRE_ETIQUETA 64

// Estructura para representar una etiqueta individual
typedef struct {
    char nombre[TAMANO_NOMBRE_ETIQUETA];
    uint32_t direccion;
} Etiqueta;

// Estructura para la tabla de símbolos/etiquetas
typedef struct {
    Etiqueta lista[MAX_ETIQUETAS];
    int cantidad;
} TablaSimbolos;

/**
 * @brief Reinicia la tabla de símbolos (cantidad = 0).
 */
void inicializar_tag_manager(void);

/**
 * @brief Registra una nueva etiqueta con su dirección correspondiente.
 * @param nombre Nombre de la etiqueta.
 * @param direccion Dirección asociada a la etiqueta en la ROM.
 * @return true si se registró exitosamente, false si hubo duplicado o se excedió el límite.
 */
bool registrar_etiqueta(const char* nombre, uint32_t direccion);

/**
 * @brief Busca una etiqueta por su nombre y devuelve su dirección.
 * @param nombre Nombre de la etiqueta a buscar.
 * @return int32_t Dirección de la etiqueta o -1 si no existe.
 */
int32_t buscar_etiqueta(const char* nombre);

/**
 * @brief Escanea el código fuente antes de la compilación principal para
 *        identificar y registrar todas las etiquetas y sus direcciones físicas.
 * @param codigo_fuente Cadena de texto que contiene el código ensamblador.
 */
void pre_escaneo_etiquetas(const char* codigo_fuente);

#endif // TAG_MANAGER_H