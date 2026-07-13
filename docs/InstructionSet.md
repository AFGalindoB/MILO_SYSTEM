# Instruction Encoding Specification - Milo Alpha

Versión Alpha 0.1

## Introducción

Este documento describe la codificación binaria de las instrucciones utilizadas por la arquitectura Milo Alpha.

Su objetivo es definir la representación física de una instrucción dentro de la memoria de programa y el significado de cada uno de sus campos, permitiendo implementar ensambladores, desensambladores y herramientas de depuración compatibles con la arquitectura.

La semántica de cada instrucción y el comportamiento del conjunto de instrucciones (ISA) se describen en documentos independientes.

## Formato general de instrucción

| Opcode | RegDest | RegSrc2 | RegSrc1 | Bus C Source Selector | Fine-Control | Inmediate | 
| ------ | --------| ------- | ------- | --------------------- | ------------ | --------- |
| 6 bits | 4 bits  | 4 bits  | 4 bits  | 5 bits                | 9 bits       | 32 bits   |

## Palabra de ROM

Cada posición de la memoria ROM almacena una palabra de 64 bits dividida en dos campos independientes.

- Bits 63:32 — Control Word
- Bits 31:0 — Immediate / Payload

El campo Control Word contiene toda la información necesaria para controlar el datapath del procesador.

El campo Immediate almacena datos adicionales utilizados por instrucciones que requieren operandos inmediatos o información auxiliar.

## OPCODE

El campo OPCODE identifica la operación principal que será ejecutada por el procesador.

La unidad de control utiliza este campo para habilitar el bloque funcional correspondiente y determinar la interpretación del campo Fine Control.

|        |     |
| ------ | --- |
| 000000 | NOP |
| 000001 | ALU |
| 000010 | MOV |

## Campo Register Control

El campo Register Control ocupa 12 bits y permite seleccionar simultáneamente tres registros del banco de registros.

Cada selección corresponde a un índice dentro de los dieciséis registros disponibles.

RegDest: Registro destino sobre el cual será escrito el resultado presente en el Bus C.

RegSrc1: Primer registro fuente conectado al Bus A.

RegSrc2: Segundo registro fuente conectado al Bus B.

|      | RegDest | RegSrc2 | RegSrc1 |
| ---- | ------- | ------- | ------- |
| 0000 | R0      | R0      | R0      |
| 0001 | R1      | R1      | R1      |
| ...  | ...     | ...     | ...     |
| 1111 | R15     | R15     | R15     |

## Bus C Source Selector

Este campo controla el multiplexor que alimenta el Bus C y determina desde qué unidad funcional será obtenido el dato que se escribirá en el banco de registros.

|       |                 |
| ----- | --------------- |
| 00000 | ALU             |
| 00001 | RegSrc1         |
| 00010 | RAM             |
| 00011 | ROM (Inmediate) |

El Bus C constituye el único camino de escritura hacia el banco de registros.

El campo Bus C Source Selector controla el multiplexor asociado a este bus, determinando cuál unidad funcional tendrá permiso para colocar información sobre él durante la ejecución de la instrucción.

## Fine Control

El campo Fine Control contiene señales de control específicas del bloque funcional seleccionado por el campo OPCODE.

Por este motivo, un mismo patrón binario puede representar señales completamente distintas dependiendo de la instrucción ejecutada.

### NOP

Fine Control ignorado.

---

### ALU

| Bits  | Nombre de la Señal  | Tipo      | Descripción Física                                                       |
| ----- | ------------------- | --------- | ------------------------------------------------------------------------ |
| `8-6` | `RESERVED`          |           | Bits libres para futuras ampliaciones.                                   |
| `5-3` | `ALU_OP`            | Seleccion | Selecciona la operación ejecutada por la Unidad Aritmético Lógica.       |
| `2`   | `CARRY_IN`          | Control   | Permite tomar el acarreo guardado a las operaciones de suma y resta.     |
| `1`   | `UPDATE_FLAGS`      | Control   | Permite actualizar todas las FLAGS.                                      |
| `0`   | `ALU_&_REGS_ENABLE` | Control   | Habilita las entradas de datos a la ALU y la carga del registro destino. |

---

### MOV

| Bits  | Nombre de la Señal | Tipo        | Descripción Física                                  |
| ----- | ------------------ | ----------- | --------------------------------------------------- |
| `8-3` | `RESERVED`         |             | Bits libres para parámetros del sistema.            |
| `2`   | `ENABLE_MDR_&_MAR` | Habilitador | Habilita el modo de acceso a memoria.               |
| `1`   | `RAM_WE`           | Habilitador | Habilita la escritura en la RAM                     |
| `0`   | `ENABLE_REGS`      | Habilitador | Habilita la escritura en el banco de registros.     |

#### Acceso a memoria

La arquitectura Milo Alpha no implementa registros dedicados MAR (Memory Address Register) ni MDR (Memory Data Register).

Durante las operaciones de acceso a memoria, dos registros de propósito general asumen temporalmente estas funciones mediante la activación de la señal `ENABLE_MDR_&_MAR`.

En este modo de operación:

- RegSrc1 se conecta al bus de direcciones de la memoria.
- RegSrc2 se conecta al bus de datos de la memoria.

Una vez finalizada la operación ambos registros recuperan inmediatamente su comportamiento habitual, sin alterar su naturaleza como registros de propósito general.

## Decodificación de instrucciones

La unidad de control realiza la decodificación de cada instrucción en dos etapas.

1. El campo OPCODE selecciona el bloque funcional responsable de ejecutar la instrucción.

2. El bloque seleccionado interpreta el contenido del campo Fine Control y genera las señales físicas necesarias para controlar el datapath.

De esta manera, un mismo conjunto de bits puede adquirir distintos significados dependiendo de la operación principal seleccionada.

## Consideraciones de diseño

La codificación de instrucciones de Milo Alpha fue diseñada para mantener una correspondencia directa entre la representación binaria de una instrucción y las señales físicas del procesador.

En consecuencia, gran parte del proceso de decodificación consiste únicamente en distribuir los diferentes campos de la palabra de control hacia los bloques funcionales correspondientes, reduciendo la lógica combinacional necesaria dentro de la unidad de control.

Esta organización simplifica la implementación del hardware y facilita el desarrollo de herramientas como ensambladores, simuladores y depuradores.