# Instruction Encoding Specification - Milo

Versión: 1.0.0

## Introducción

Este documento describe la codificación binaria de las instrucciones utilizadas por la arquitectura Milo Alpha.

Su objetivo es definir la representación física de una instrucción dentro de la memoria de programa y el significado de cada uno de sus campos, permitiendo implementar ensambladores, desensambladores y herramientas de depuración compatibles con la arquitectura.

La semántica de cada instrucción y el comportamiento del conjunto de instrucciones (ISA) se describen en documentos independientes.

## Formato general de instrucción

| Opcode | RegDest | RegSrc2 | RegSrc1 | Bus C Source Selector | Fine-Control | Inmediate | 
| ------ | --------| ------- | ------- | --------------------- | ------------ | --------- |
| 6 bits | 4 bits  | 4 bits  | 4 bits  | 5 bits                | 9 bits       | 32 bits   |

> Nota: Dependiendo de la version el inmediato puede ser de 24 bits usando los bits del 0-24

## Palabra de ROM

Cada posición de la memoria ROM almacena una palabra de 64 bits dividida en dos campos independientes.

- Bits 63:32 — Control Word
- Bits 31:0 — Immediate / Payload

El campo Control Word contiene toda la información necesaria para controlar el datapath del procesador.

El campo Immediate almacena datos adicionales utilizados por instrucciones que requieren operandos inmediatos o información auxiliar.

## OPCODE

El campo OPCODE identifica la operación principal que será ejecutada por el procesador.

La unidad de control utiliza este campo para habilitar el bloque funcional correspondiente y determinar la interpretación del campo Fine Control.

|        |      |
| ------ | ---- |
| 000000 | NOP  |
| 000001 | ALU  |
| 000010 | MOV  |
| 000011 | PC   |
| 000100 | STOP |

## Selección de operandos

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

## Modelo de registros

La arquitectura Milo Alpha clasifica los registros visibles para el programador según el tipo de acceso permitido por el hardware.

Actualmente existen tres categorías de registros:

- Registros de lectura y escritura (RW).
- Registros de solo escritura (WO).
- Registros de solo lectura (RO).

Esta clasificación permite integrar periféricos dentro del espacio de registros sin introducir nuevas instrucciones específicas para cada dispositivo.

### Registros de lectura y escritura (RW)

Corresponden al banco de registros de propósito general del procesador.

Estos registros pueden utilizarse libremente como operandos fuente y como destinos de escritura.

Actualmente existen dieciséis registros de propósito general:

| Código | Registro |
| ------ | -------- |
| 0000   | R0       |
| 0001   | R1       |
| ...    | ...      |
| 1111   | R15      |

Estos registros son utilizados por todas las operaciones aritméticas, lógicas, de memoria y control de flujo.

### Registros de solo escritura (WO)

Los registros de solo escritura representan periféricos controlados directamente por el procesador.

Su contenido no puede leerse desde el software.

Actualmente se implementan:

| Registro | Función                                                                  |
| -------- | ------------------------------------------------------------------------ |
| TBUF     | Escribe un índice de tile dentro del Tile Buffer de la GPU.              |
| SCROLL   | Actualiza el registro de desplazamiento horizontal y vertical de la GPU. |

Internamente estos registros no forman parte del banco de registros.

Cuando una instrucción utiliza uno de ellos como destino, la unidad de control habilita la señal correspondiente (WE_TILE_BUFFER, SCROLL, etc.) para cargar el dato presente sobre el Bus C.

### Registros de solo lectura (RO)

Los registros de solo lectura representan entradas provenientes del hardware externo.

Actualmente se implementa:

| Registro | Función                          |
| -------- | -------------------------------- |
| RINPT    | Registro de entrada del sistema. |

Su contenido únicamente puede ser producido por el hardware.

Las instrucciones pueden utilizar este registro como operando fuente, pero nunca como destino.

### Integración con el Bus C

Todos los registros especiales se integran utilizando el mismo datapath del procesador.

Los registros de solo lectura pueden actuar como fuente del Bus C mediante el campo **Bus C Source Selector**.

Los registros de solo escritura reciben el valor presente sobre el Bus C cuando la unidad de control habilita la señal correspondiente.

De esta manera, la CPU mantiene un único mecanismo de transferencia de datos tanto para el banco de registros como para los periféricos.

## Bus C Source Selector

Este campo controla el multiplexor que alimenta el Bus C y determina desde qué unidad funcional será obtenido el dato que será distribuido hacia los bloques habilitados durante la ejecución de la instrucción.

|       |                 |
| ----- | --------------- |
| 00000 | ALU             |
| 00001 | RegSrc1         |
| 00010 | RAM             |
| 00011 | ROM (Immediate) |
| 00100 | RINPT           |

El Bus C constituye el camino principal de distribución de datos dentro del procesador.

Dependiendo de las señales de control activadas por la instrucción ejecutada, el valor presente sobre este bus puede ser utilizado por distintos bloques funcionales, entre ellos:

- Banco de registros.
- La GPU.
- El registro de entradas de usuario

El campo **Bus C Source Selector** controla el multiplexor asociado a este bus, determinando qué unidad funcional tendrá permiso para colocar información sobre él durante la ejecución de la instrucción.

## Fine Control

El campo Fine Control contiene señales de control específicas del bloque funcional seleccionado por el campo OPCODE.

Por este motivo, un mismo patrón binario puede representar señales completamente distintas dependiendo de la instrucción ejecutada.

### NOP

Fine Control ignorado.

---

### ALU

| Bits  | Nombre de la Señal | Tipo        | Descripción Física                                                       |
| ----- | ------------------ | ----------- | ------------------------------------------------------------------------ |
| `8`   | `WE_TILE_BUFFER`   | Habilitador | Permite cargar un valor en el buffer de tileset de la gpu.               |
| `7`   | `SCROLL`           | Habilitador | Permite cargar un valor en el registro de pixel y tile offset de la gpu. |
| `6-4` | `ALU_OP`           | Seleccion   | Selecciona la operación ejecutada por la Unidad Aritmético Lógica.       |
| `3`   | `CARRY_IN`         | Control     | Permite tomar el acarreo guardado a las operaciones de suma y resta.     |
| `2`   | `UPDATE_FLAGS`     | Control     | Permite actualizar todas las FLAGS.                                      |
| `1`   | `ENABLE_ALU`       | Control     | Permite la entradade datos a la ALU                                      |
| `0`   | `REGS_ENABLE`      | Control     | Habilita la carga del registro destino.                                  |

---

### MOV

| Bits  | Nombre de la Señal | Tipo        | Descripción Física                                                       |
| ----- | ------------------ | ----------- | ------------------------------------------------------------------------ |
| `8-5` | `RESERVED`         |             | Bits libres para parámetros del sistema.                                 |
| `4`   | `WE_TILE_BUFFER`   | Habilitador | Permite cargar un valor en el buffer de tileset de la gpu.               |
| `3`   | `SCROLL`           | Habilitador | Permite cargar un valor en el registro de pixel y tile offset de la gpu. |
| `2`   | `ENABLE_MDR_&_MAR` | Habilitador | Habilita el modo de acceso a memoria.                                    |
| `1`   | `RAM_WE`           | Habilitador | Habilita la escritura en la RAM                                          |
| `0`   | `ENABLE_REGS`      | Habilitador | Habilita la escritura en el banco de registros.                          |

---

### PC

| Bits  | Nombre de la Señal     | Tipo        | Descripción Física                                                   |
| ----- | ---------------------- | ----------- | -------------------------------------------------------------------- |
| `8-7` | `SEL_FLAG`             | Control     | Selecciona una de las flags                                          |
| `6`   | `NEGATE_FLAG`          | Control     | 0: Flags normales; 1: Flags Negadas                                  |
| `5`   | `JMP_CONDITIONAL`      | Habilitador | Habilita el salto con flags                                          |
| `4`   | `ENABLE_SP`            | Habilitador | Cuando esta a 1 permite subir o bajar stack pointer                  |
| `3`   | `UPDOWN_STACK_POINTER` | Control     | 1: Sube stack pointer, 0: Baja Stack pointer.                        |
| `2`   | `WE_STACK`             | Habilitador | Permite cargar un valor en el STACK.                                 |
| `1`   | `SEL_PC`               | Control     | 0: ROM; 1 STACK                                                      |
| `0`   | `PC_JUMP`              | Habilitador | Habilita la carga del program counter.                               |

---

### STOP

| Bits  | Nombre de la Señal | Tipo        | Descripción Física                                                   |
| ----- | ------------------ | ----------- | -------------------------------------------------------------------- |
| `1`   | `SEL_WAIT`         | Control     | 0: HBlank; 1 VBlank                                                  |
| `0`   | `ENABLE_WAIT`      | Habilitador | Habilita la espera del program counter.                              |

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