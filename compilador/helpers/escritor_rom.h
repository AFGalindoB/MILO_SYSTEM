#ifndef ESCRITOR_ROM_H
#define ESCRITOR_ROM_H

#include "../codificador/codificador.h"
#include <stdint.h>

void exportar_a_txt_rom(const char* nombre_archivo, PalabraROM* palabras, uint32_t cantidad_instrucciones);

#endif