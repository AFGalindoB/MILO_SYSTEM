# Milo Alpha

Ecosistema experimental para el diseño de una arquitectura de procesador de 32 bits, su ISA, compilador y herramientas de desarrollo.

Licencia: MIT

## Descripción

Milo Alpha es un ecosistema compuesto por una arquitectura de CPU propia, un lenguaje ensamblador (MILO ASM), un compilador y la documentación necesaria para comprender y extender el sistema.

A diferencia de muchas arquitecturas tradicionales, el desarrollo del proyecto comienza por la implementación física del procesador. Sobre esa base se construyen posteriormente el ISA, el compilador y el resto del ecosistema de software.

Actualmente el hardware se implementa utilizando **Logisim Evolution**, mmientras que el compilador está desarrollado en **C**, siguiendo una arquitectura modular compuesta por lexer, parser, encoder y herramientas auxiliares.

## Características

- Arquitectura de 32 bits.
- 16 registros de propósito general.
- Tres buses internos.
- Unidad de control cableada (Hardwired Control Unit).
- ISA propia (MILO ASM).
- Compilador modular desarrollado en C.
- Program Counter y soporte para flujo de control.
- Registro de banderas (N, Z, C y V).
- Implementación completa en Logisim Evolution.
- Documentación técnica del hardware y del compilador.
- Sistema de pruebas para el compilador y la CPU.

## Estado actual

### Hardware

- ✅ Banco de registros.
- ✅ ALU.
- ✅ Registro de banderas (Status Register).
- ✅ Program Counter.
- ✅ Stack Pointer.
- ✅ Instrucciones MOV.
- ✅ Instrucciones LOAD / STORE.
- ✅ Valores inmediatos.
- ✅ Comparación (CMP).
- ✅ Saltos incondicionales (JMP).
- ✅ Llamadas a subrutinas (CALL).
- ✅ Retorno de subrutinas (RET).
- ✅ Saltos condicionales mediante banderas (JZ, JNZ, JC, JNC, JN, JNN, JV, JNV).
- 🚧 Etiquetas del ensamblador.
- 🚧 GPU.
- 🚧 DMA.

### Software

- ✅ Lexer.
- ✅ Parser con recuperación ante errores (panic mode).
- ✅ Reportes de error con línea y columna.
- ✅ Encoder.
- ✅ Expansión de instrucciones del ISA hacia múltiples palabras de control.
- ✅ Exportación de programas hacia ROM.
- ✅ Sistema de pruebas.
- 🚧 Resolución automática de etiquetas.
- 🚧 Expansión del ISA.

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

Actualmente Milo Alpha se encuentra en una etapa **Alpha**, centrada en el desarrollo y validación de la arquitectura base del procesador y de su ecosistema de herramientas.

Las próximas etapas del proyecto incluyen la incorporación de etiquetas en el ensamblador, nuevas instrucciones del ISA, GPU, DMA, sistema operativo, compiladores de alto nivel y otros componentes necesarios para evolucionar hacia un ecosistema de propósito general.

Milo Alpha no busca únicamente implementar un procesador; busca desarrollar un ecosistema completo donde hardware, ISA, compilador y herramientas evolucionen de forma coordinada.