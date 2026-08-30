#ifndef COMPILE_TEXT_H
#define COMPILE_TEXT_H

/**
 * Genera instrucciones ASM para escribir un texto
 * en el Tile Buffer de la GPU.
 *
 * Cada carácter del texto se busca dentro de la tabla
 * de sprites válida. La posición del carácter dentro
 * del texto determina la posición del tile.
 *
 * Formato del valor escrito en TBUF:
 *
 * Bits  0-11  -> posición
 * Bits 12-23  -> índice del tile
 *
 * Retorna:
 *     String con las instrucciones ASM generadas.
 *     NULL si ocurre un error.
 *
 * El resultado debe liberarse con free().
 */
char* generar_tbuf_texto(const char* texto);

#endif