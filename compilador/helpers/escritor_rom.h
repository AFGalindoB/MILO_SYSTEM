#ifndef ESCRITOR_ROM_H
#define ESCRITOR_ROM_H

#include <stdint.h>

void establecer_directorio_compilaciones(const char* directorio);

void exportar_a_txt_rom(
    const char* nombre_archivo,
    uint64_t* palabras,
    uint32_t cantidad_instrucciones
);

#endif