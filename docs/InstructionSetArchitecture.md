# Milo Alpha ISA
Versión Alpha 0.1

## Introducción

Este documento describe el conjunto de instrucciones (Instruction Set Architecture, ISA) de la arquitectura Milo Alpha.

El ISA constituye la interfaz visible para el programador y define la semántica de cada instrucción, su sintaxis en lenguaje ensamblador y el comportamiento esperado durante su ejecución.

La representación binaria de las instrucciones y su codificación física se describen en el documento [Instruction Encoding Specification](./InstructionEncoding.md).

## Registros

### Registros de propósito general

La arquitectura dispone de dieciséis registros de propósito general de 32 bits.

| Registro |
| -------- |
| R0       |
| R1       |
| ...      |
| R15      |

Estos registros pueden utilizarse como operandos fuente y destino en las instrucciones del ISA.

### Registros especiales

Además del banco de registros, la arquitectura define un conjunto de registros especiales asociados a la GPU.

| Registro | Descripción                                                         |
| -------- | ------------------------------------------------------------------- |
| `TBUF`   | Escribe un tile en el Tile Buffer.                                  |
| `PIXOFF` | Actualiza el registro Pixel Offset.                                 |
| `TLOFF`  | Actualiza el registro Tile Offset.                                  |
| `SCROLL` | Actualiza simultáneamente los registros Pixel Offset y Tile Offset. |

A diferencia de los registros de propósito general, estos registros únicamente pueden utilizarse como destinos de escritura.

## Notación

En este documento se utiliza la siguiente convención.

| Símbolo | Significado                                     |
| ------- | ----------------------------------------------- |
| `Rn`    | Registro de propósito general.                  |
| `#n`    | Valor inmediato de 32 bits.                     |
| `[Rn]`  | Dirección de memoria almacenada en un registro. |

- Rd = Registro destino
- Rs = Registro fuente

## Transferencia de datos

Las instrucciones de transferencia permiten mover información entre registros de propósito general, memoria y registros especiales de la GPU.

### Destinos de escritura

El resultado de una instrucción de transferencia puede escribirse en:

- Un registro de propósito general.
- Un registro especial de la GPU.
- Ambos simultáneamente.

Cuando una instrucción especifica ambos destinos, el mismo dato es escrito sobre el banco de registros y sobre la GPU durante el mismo ciclo de ejecución.

Cada instrucción admite como máximo:

- Un registro de propósito general como destino.
- Un registro especial como destino.

Los registros especiales disponibles son:

- `PIXOFF`
- `TLOFF`
- `SCROLL`
- `TBUF`

En la sintaxis del ensamblador, cuando se utilizan ambos destinos, el registro especial siempre aparece antes del registro de propósito general.

---

### MOV / MOVI

Las instrucciones `MOV` y `MOVI` copian un dato hacia uno o más destinos.

La diferencia entre ambas radica únicamente en el origen del dato.

- `MOV` toma el dato desde un registro.
- `MOVI` toma el dato desde un valor inmediato.

#### Sintaxis

Origen desde registro

```asm
MOV Rd, Rs
MOV GPU, Rs
MOV GPU, Rd, Rs
```

Origen inmediato

```asm
MOVI Rd, #n
MOVI GPU, #n
MOVI GPU, Rd, #n
```

Donde:

- `Rd` representa un registro de propósito general.
- `Rs` representa un registro fuente.
- `GPU` representa cualquiera de los registros especiales (`PIXOFF`, `TLOFF`, `SCROLL` o `TBUF`).

**Ejemplos:**

```asm
MOV R1, R0
MOV PIXOFF, R0
MOV PIXOFF, R1, R0
```

```asm
MOVI R2, #15
MOVI SCROLL, #0x53
MOVI SCROLL, R2, #0x53
```

## Acceso a memoria

Las instrucciones de acceso a memoria permiten transferir datos entre la memoria RAM, el banco de registros y los registros especiales de la GPU.

Al igual que las instrucciones de transferencia de datos, `LOAD` puede escribir el dato leído en:

- Un registro de propósito general.
- Un registro especial de la GPU.
- Ambos simultáneamente.

Por el contrario, `STORE` siempre escribe sobre la memoria RAM.

### LOAD

Lee una palabra desde la memoria RAM.

#### Sintaxis

```asm
LOAD Rd, [Rs]
LOAD GPU, [Rs]
LOAD GPU, Rd, [Rs]
```

Donde:

- `Rd` representa un registro de propósito general.
- `Rs` contiene la dirección de memoria a leer.
- `GPU` representa cualquiera de los registros especiales (`PIXOFF`, `TLOFF`, `SCROLL` o `TBUF`).

Los corchetes (`[]`) indican que el contenido del registro debe interpretarse como una dirección de memoria y no como un dato.

**Ejemplos:**

```asm
LOAD R0, [R5]
LOAD PIXOFF, [R5]
LOAD PIXOFF, R0, [R5]
```

Durante la ejecución:

- `Rs` actúa temporalmente como **MAR (Memory Address Register)**.

---

### STORE

Escribe una palabra en la memoria RAM.

#### Sintaxis

```asm
STORE Rs, [Ra]
```

Donde:

- `Rs` contiene el dato que será escrito.
- `Ra` contiene la dirección de memoria de destino.

**Ejemplo:**

```asm
STORE R2, [R4]
```

Durante la ejecución:

- `Ra` actúa temporalmente como **MAR (Memory Address Register)**.
- `Rs` actúa temporalmente como **MDR (Memory Data Register)**.

### Consideraciones

La arquitectura Milo Alpha no implementa registros dedicados para MAR y MDR.

Durante las operaciones de acceso a memoria, dos registros de propósito general asumen temporalmente estas funciones, reduciendo la cantidad de hardware dedicado y manteniendo un banco de registros uniforme.

En consecuencia, antes de ejecutar un `STORE`, tanto la dirección de memoria como el dato a escribir deben encontrarse previamente cargados en registros de propósito general.

## Instrucciones Aritmético-Lógicas

Las instrucciones aritmético-lógicas realizan operaciones sobre uno o dos operandos provenientes del banco de registros.

El resultado de la operación puede escribirse en:

- Un registro de propósito general.
- Un registro especial de la GPU (`SCROLL` o `TBUF`).
- Ambos simultáneamente.

A diferencia de las instrucciones de transferencia de datos, las operaciones de la ALU únicamente pueden escribir sobre los registros especiales `SCROLL` y `TBUF`.

Cuando una instrucción especifica ambos destinos, el resultado generado por la ALU es escrito simultáneamente sobre el banco de registros y sobre la GPU.

---

### Operaciones binarias

Las operaciones binarias utilizan uno de los siguientes formatos.

```asm
MNEMONICO Rd, Ra, Rb

MNEMONICO GPU, Ra, Rb

MNEMONICO GPU, Rd, Ra, Rb
```

Donde:

- `Rd` representa el registro destino.
- `Ra` representa el primer operando.
- `Rb` representa el segundo operando.
- `GPU` puede ser `SCROLL` o `TBUF`.

Las instrucciones disponibles son:

- ADD
- ADC
- SUB
- SBC
- AND
- OR
- XOR
- SHL
- SHR
- CMP

**Ejemplos:**

```asm
ADD R0, R1, R2
ADD SCROLL, R1, R2
ADD SCROLL R0, R1, R2
```

---

### NOT

La instrucción `NOT` realiza una negación bit a bit sobre un único operando.

#### Sintaxis

```asm
NOT Rd, Rs

NOT GPU, Rs

NOT GPU, Rd, Rs
```

Donde:

- `Rd` representa un registro de propósito general.
- `Rs` representa el operando fuente.
- `GPU` puede ser `SCROLL` o `TBUF`.

**Ejemplos:**

```asm
NOT R1, R0
NOT TBUF, R0
NOT TBUF, R1, R0
```

---

# Actualización de Flags

Las instrucciones aritméticas pueden incorporar el modificador `.F`.

Cuando este modificador está presente, el procesador actualiza el registro de banderas utilizando el resultado de la operación.

Ejemplo

```asm
ADD.F R0, R1, R2
```

Sin el modificador `.F`, la operación no altera las banderas.

---

# Subrutinas

## CALL

Realiza una llamada a una subrutina.

```asm
CALL #direccion
```

La dirección de retorno se almacena automáticamente en la pila.

---

## RET

Retorna desde una subrutina.

```asm
RET
```

La dirección de retorno es obtenida desde la pila.

---

# Saltos Condicionales

La arquitectura dispone de saltos basados en el estado de las banderas.

## Zero

```asm
JZ #direccion
JNZ #direccion
```

---

## Carry

```asm
JC #direccion
JNC #direccion
```

---

## Negative

```asm
JN #direccion
JNN #direccion
```

---

## Overflow

```asm
JV #direccion
JNV #direccion
```

Todos los saltos utilizan una dirección inmediata como destino.