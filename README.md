# Milo

Ecosistema para el diseño de una arquitectura de procesador propia, su ISA, compilador, GPU y herramientas de desarrollo.

Milo busca construir una plataforma completa donde hardware y software evolucionen conjuntamente. El proyecto incluye el diseño del procesador, una arquitectura gráfica 2D, un lenguaje ensamblador propio (MILO ASM), un compilador y la documentación necesaria para comprender y extender todo el ecosistema.

Licencia: MIT

## Descripción

Milo Alpha es un ecosistema compuesto por una arquitectura de CPU propia, un lenguaje ensamblador (MILO ASM), un compilador y la documentación necesaria para comprender y extender el sistema.

A diferencia de muchas arquitecturas tradicionales, el desarrollo del proyecto comienza por la implementación física del procesador. Sobre esa base se construyen posteriormente el ISA, el compilador y el resto del ecosistema de software.

Actualmente el hardware se implementa utilizando **Logisim Evolution**, mmientras que el compilador está desarrollado en **C**, siguiendo una arquitectura modular compuesta por lexer, parser, encoder y herramientas auxiliares.

## Arquitectura

Actualmente la arquitectura estable de Milo utiliza:

- CPU de 24 bits.
- Program Counter de 24 bits.
- 16 registros de propósito general.
- Tres buses internos.
- Unidad de control cableada (Hardwired Control Unit).
- GPU 2D integrada mediante registros especiales.
- ISA propia (MILO ASM).

> **Nota:** Las primeras versiones del proyecto fueron diseñadas sobre una arquitectura de 32 bits. A partir de la versión `1.0.0`, el desarrollo continúa sobre una arquitectura de 24 bits con el objetivo de optimizar el uso de recursos de FPGA y simplificar el datapath sin modificar la filosofía del ISA.

## Características

- Arquitectura CPU de 24 bits.
- Program Counter de 24 bits.
- 16 registros de propósito general.
- ISA propia (MILO ASM).
- Unidad de control cableada.
- Compilador modular en C.
- GPU 2D integrada.
- Tile Buffer.
- Scroll por hardware.
- Sincronización CPU-GPU mediante WAITV y WAITH.
- Milo Studio para edición de recursos gráficos.
- Documentación técnica completa.

## Componentes

Actualmente el ecosistema Milo está compuesto por:

### CPU

Arquitectura propia basada en una CPU de 24 bits con ISA personalizada, unidad de control cableada y soporte para ejecución de programas escritos en MILO ASM.

- ✅ Banco de registros.
- ✅ ALU.
- ✅ Unidad de control.
- ✅ Program Counter.
- ✅ Stack Pointer.
- ✅ Instrucciones aritméticas.
- ✅ Instrucciones de memoria.
- ✅ Saltos.
- ✅ CALL / RET.
- ✅ Registros especiales.
- ✅ Sincronización WAITV.
- ✅ Manejo de entradas.

### GPU

Procesador gráfico 2D orientado a sistemas embebidos con soporte para Tile Buffer, scroll por hardware y futura incorporación de sprites (OAM).

✅ Tile Buffer.
✅ Scroll por hardware.
✅ Escritura desde MOV, MOVI, LOAD y ALU.
🚧 OAM.
🚧 Sprites.

### Toolchain

Cadena completa de compilación escrita en C:

✅ Lexer.
✅ Parser.
✅ Encoder.
✅ Exportación a codigo en hexadecimal.
✅ Recuperación de errores.
✅ Documentación del ISA.
✅ Sistema de pruebas
🚧 Resolución automática de etiquetas.
🚧 Librerías estándar.

### Milo Studio

SDK gráfico para el desarrollo de recursos visuales del sistema.

Actualmente incluye:

✅ Editor de tiles 8×8
✅ Editor de paletas LUT
✅ Gestión de múltiples tiles
✅ Exportación de tilesets
✅ Importación de tiles indexados
✅ Carga de paletas RGB565
🚧 Editor de sprites
🚧 Editor de mapas

## Documentación

Para conocer la arquitectura y el funcionamiento interno del proyecto consulte:

- [**Developer Guide**](./docs/DeveloperGuide.md): Visión general de la arquitectura y del flujo de desarrollo.

## Filosofía del proyecto

Milo Alpha separa claramente tres niveles de abstracción:

```text
Programador
      │
      ▼
ISA
      │
      ▼
Instruction Encoding
      │
      ▼
Hardware
```

El procesador ejecuta únicamente palabras de control binarias.

El compilador traduce el ISA hacia esa representación física, permitiendo que el lenguaje ensamblador evolucione independientemente de la implementación interna del hardware.

Una misma instrucción del ISA puede traducirse en una o varias palabras de control sin modificar la interfaz visible para el programador.

## Estado del proyecto

Milo ha alcanzado su primera versión estable de arquitectura (`v1.0.0`).

La CPU, la GPU básica, el compilador y el flujo completo de compilación se encuentran completamente funcionales.

El desarrollo futuro se centrará en:

- Migración del hardware desde Logisim Evolution hacia VHDL.
- Sistema de sprites (OAM).
- Controlador de audio.
- DMA.
- Sistema operativo.
- SDK gráfico.
- Compiladores de alto nivel.