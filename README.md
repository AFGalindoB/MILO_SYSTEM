# Milo Alpha

Ecosistema experimental para el diseño de una arquitectura de procesador de 32 bits, su ISA, compilador y herramientas de desarrollo.

Licencia: MIT

## Descripción

Milo Alpha es un ecosistema compuesto por una arquitectura de CPU propia, un lenguaje ensamblador (MILO ASM), un compilador y la documentación necesaria para comprender y extender el sistema.

A diferencia de muchas arquitecturas tradicionales, el desarrollo del proyecto parte del diseño físico del procesador. Sobre esta implementación hardware se construyen posteriormente el ISA, el compilador y el resto de herramientas de desarrollo.

Actualmente el procesador se implementa utilizando **Logisim Evolution**, mientras que el compilador se encuentra desarrollado en **C**.

## Características

- Arquitectura de 32 bits.
- 16 registros de propósito general.
- Tres buses internos.
- Unidad de control cableada (*Hardwired Control Unit*).
- ISA (Instruction Set Architecture) propia.
- Compilador ensamblador propio (MILO ASM).
- Implementación completa en Logisim Evolution.
- Documentación técnica del hardware y del compilador.
- Sistema de pruebas para el compilador y la CPU.

## Estado actual

### Hardware

- ✅ Banco de registros.
- ✅ ALU.
- ✅ Registro de banderas (Status Register).
- ✅ Instrucciones MOV.
- ✅ Instrucciones LOAD / STORE.
- ✅ Valores inmediatos.
- ✅ Program Counter.
- 🚧 Instrucciones de salto.
- 🚧 Stack (Pila)
- 🚧 GPU.
- 🚧 DMA.

### Software

- ✅ Lexer.
- ✅ Parser.
- ✅ Codificador de instrucciones.
- ✅ Exportación de programas hacia ROM.
- ✅ Sistema de pruebas.
- 🚧 Expansión del ISA.

## Documentación

En caso de desear conocer mas a profundidad el sistema revisar el documento:

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

El procesador únicamente ejecuta palabras de control binarias.

El compilador se encarga de traducir el ISA hacia dicha representación física, permitiendo que ambas capas evolucionen de forma relativamente independiente.

## Estado del proyecto

Actualmente Milo Alpha se encuentra en una etapa **Alpha**, enfocada en el desarrollo y validación de la arquitectura base del procesador y de su ecosistema de herramientas.