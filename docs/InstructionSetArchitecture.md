# Milo ISA

Versión: 1.0.0

## Introducción

Este documento describe el conjunto de instrucciones (Instruction Set Architecture, ISA) de la arquitectura Milo.

El ISA constituye la interfaz visible para el programador y define la semántica de cada instrucción, su sintaxis en lenguaje ensamblador y el comportamiento esperado durante su ejecución.

La representación binaria de las instrucciones y su codificación física se describen en el documento [Instruction Encoding Specification](./InstructionEncoding.md).

## Sintaxis del ensamblador

El compilador de Milo procesa el código fuente instrucción por instrucción.

Cada instrucción posee una sintaxis bien definida que especifica la cantidad y el tipo de operandos requeridos.

Una vez que el compilador ha reconocido todos los operandos necesarios para una instrucción, cualquier token adicional presente en la misma línea será ignorado.

Esto permite que el analizador sintáctico mantenga un comportamiento tolerante frente a texto adicional después de una instrucción válida.

Por ejemplo, las siguientes líneas son equivalentes para el compilador:

```asm
MOVI R2, #2

MOVI R2, #2 ADD SUB XOR
```

En ambos casos únicamente se procesa la instrucción `MOVI R2, #2`.

Del mismo modo, una etiqueta finaliza inmediatamente después del carácter :.

```asm
FIN:

FIN: NOP ADD MOV R0, R1
```

Representan exactamente la misma definición de etiqueta.

Todo el contenido situado después del carácter : será ignorado.

Este comportamiento forma parte del funcionamiento del compilador y no modifica la semántica del programa generado.

## Registros

La arquitectura Milo clasifica los registros visibles para el programador según el tipo de acceso permitido por el hardware.

Actualmente existen tres categorías:

- Registros de lectura y escritura (RW).
- Registros de solo lectura (RO).
- Registros de solo escritura (WO).

| Tipo de registro | Leer | Escribir |
| ---------------- | ---- | -------- |
| General (RW)     |  ✅  |    ✅    |
| Entrada (RO)     |  ✅  |    ❌    |
| GPU (WO)         |  ❌  |    ✅    |


### Registros de lectura y escritura (RW)

Corresponden al banco de registros de propósito general.

Estos registros pueden utilizarse tanto como operandos fuente como destinos de escritura.

Actualmente existen dieciséis registros:

| Registro |
| -------- |
| R0       |
| R1       |
| ...      |
| R15      |

### Registros de solo lectura (RO)

Representan información generada por el hardware.

Actualmente la arquitectura implementa:

| Registro | Descripción                      |
| -------- | -------------------------------- |
| RINPT    | Registro de entrada del sistema. |

Estos registros únicamente pueden utilizarse como operandos fuente.

Intentar escribir sobre ellos constituye un error de compilación.

### Registros de solo escritura (WO)

Representan periféricos controlados por el procesador. Actualmente existen:

| Registro | Descripción                           |
| -------- | ------------------------------------- |
| TBUF     | Escritura del Tile Buffer de la GPU.  |
| SCROLL   | Registro de desplazamiento de la GPU. |

Estos registros únicamente pueden utilizarse como destinos.

No es posible leer su contenido desde el software.

## Notación

En este documento se utiliza la siguiente convención.

| Símbolo | Significado                                     |
| ------- | ----------------------------------------------- |
| `Rn`    | Registro de propósito general.                  |
| `#n`    | Valor inmediato de 24 bits.                     |
| `[Rn]`  | Dirección de memoria almacenada en un registro. |

- Rd = Registro destino
- Rs = Registro fuente

## Etiquetas

Las etiquetas permiten asignar un nombre simbólico a una dirección del programa.

Su objetivo es facilitar la escritura de saltos, llamadas a subrutinas y referencias dentro del código sin depender de direcciones numéricas.

Una etiqueta se define escribiendo un identificador seguido del carácter `:`.

Ejemplo:

```asm
INICIO:
```

Una vez definida, puede utilizarse como operando en cualquier instrucción que espere una dirección.

```asm
JMP INICIO

CALL MI_FUNCION
```

Las etiquetas no generan código máquina.

Durante la compilación son reemplazadas por la dirección correspondiente de la instrucción asociada.


### Reglas sintácticas de una etiqueta

Una etiqueta:

- Debe finalizar con el carácter `:`.
- Debe poseer un nombre único dentro del programa.
- Puede declararse antes o después de ser utilizada.
- No ocupa espacio en la ROM.
- No modifica el contador de programa.

## Transferencia de datos

Las instrucciones de transferencia permiten mover información entre registros de propósito general, memoria y registros especiales de la arquitectura.

### Destinos de escritura

El resultado de una instrucción de transferencia puede escribirse en:

- Un registro de propósito general (RW).
- Un registro especial de solo escritura (WO).
- Ambos simultáneamente.

Cuando una instrucción especifica ambos destinos, el mismo dato es escrito sobre el banco de registros y sobre el periférico correspondiente durante el mismo ciclo de ejecución.

Cada instrucción admite como máximo:

- Un registro de propósito general como destino.
- Un registro especial de solo escritura como destino.

Actualmente los registros especiales de solo escritura disponibles son:

- `TBUF`
- `SCROLL`

Los registros de solo lectura, como `RINPT`, únicamente pueden utilizarse como operandos fuente y nunca como destino de escritura.

En la sintaxis del ensamblador, cuando una instrucción utiliza ambos destinos, el registro especial siempre aparece antes del registro de propósito general.

**Ejemplos:**

```asm
MOV R1, R0
MOV TBUF, R0
MOV TBUF, R1, R0
MOV R2, RINPT
```

> Nota: Los registros especiales no forman parte del banco de registros de propósito general. Cada uno representa un periférico o recurso de hardware específico y su comportamiento (solo lectura o solo escritura) es definido por la arquitectura.

---

### MOV / MOVI

Las instrucciones `MOV` y `MOVI` permiten transferir información entre registros de propósito general, registros especiales y valores inmediatos, respetando las restricciones de acceso de cada tipo de registro.

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

- `Rd` representa un registro de propósito general o un registro de solo lectura.
- `Rs` representa un registro fuente.
- `GPU` representa cualquiera de los registros especiales (`SCROLL` o `TBUF`).

**Ejemplos:**

```asm
MOV R1, R0
MOV R1, RINPT
MOV TBUF, R0
MOV TBUF, R1, R0
```

```asm
MOVI R2, #15
MOVI SCROLL, #0x53
MOVI SCROLL, R2, #0x53
```

## Acceso a memoria

Las instrucciones de acceso a memoria permiten transferir datos entre la memoria RAM, el banco de registros y los registros de solo escritura.

Al igual que las instrucciones de transferencia de datos, `LOAD` puede escribir el dato leído en:

- Un registro de propósito general.
- Un Registro de solo escritura.
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
- `GPU` representa cualquiera de los registros especiales (`SCROLL` o `TBUF`).

Los corchetes (`[]`) indican que el contenido del registro debe interpretarse como una dirección de memoria y no como un dato.

**Ejemplos:**

```asm
LOAD R0, [R5]
LOAD TBUF, [R5]
LOAD TBUF, R0, [R5]
```

Durante la ejecución:

- `Rs` actúa temporalmente como **MAR (Memory Address Register)**.

---

### STORE

Escribe una palabra en la memoria RAM.

#### Sintaxis

```asm
STORE Rs, [Ra]
STORE RINPT, [Ra]
STORE #n, [Ra]
```

Donde:

- `Rs` contiene el dato que será escrito.
- `Ra` contiene la dirección de memoria de destino.
- `RINPT` permite escribir directamente el contenido del registro de entrada.
- `#n` representa un valor inmediato de 24 bits.

**Ejemplo:**

```asm
STORE R2, [R4]

STORE RINPT, [R4]

STORE #0x1a, [R4]
```

**Funcionamiento**

Durante la ejecución de `STORE`:

- `Ra` actúa temporalmente como **Memory Address Register (MAR)** y suministra la dirección de memoria.
- El dato a escribir proviene del **Bus C**, cuyo origen es seleccionado por el campo **Bus C Source Selector** de la instrucción.

Dependiendo de la configuración del Bus C, la memoria puede recibir datos provenientes de:

- Un registro de propósito general.
- El registro de entrada (`RINPT`).
- Un valor inmediato almacenado en la ROM.

Esta organización permite escribir distintos tipos de datos en memoria utilizando una única instrucción, sin necesidad de cargar previamente toda la información en el banco de registros.

### Consideraciones

La arquitectura Milo no implementa registros dedicados para **MAR (Memory Address Register)** ni **MDR (Memory Data Register)**.

Durante una operación `STORE`, el registro utilizado como dirección actúa temporalmente como **MAR**, mientras que el dato es suministrado directamente por el **Bus C**.

Esto permite que una operación de escritura obtenga el dato desde distintas fuentes sin requerir transferencias adicionales al banco de registros.

Actualmente pueden escribirse en memoria valores provenientes de:

- Registros de propósito general.
- El registro de entrada (`RINPT`).
- Valores inmediatos almacenados en la ROM.

Gracias a este diseño, muchas operaciones de escritura pueden realizarse en un único ciclo menos respecto a implementaciones donde el dato debe copiarse previamente a un registro temporal.

## Instrucciones Aritmético-Lógicas

Las instrucciones aritmético-lógicas realizan operaciones sobre uno o dos operandos provenientes del banco de registros.

El resultado de la operación puede escribirse en:

- Un registro de propósito general.
- Un Registro de solo escritura.
- Ambos simultáneamente.

A diferencia de las instrucciones de transferencia de datos, las operaciones de la ALU únicamente puede escribirse en registros de propósito general y registros especiales de solo escritura.

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
CALL #120
CALL MI_FUNCION
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

JZ MI_FUNCION
JNZ MI_FUNCION
```

---

## Carry

```asm
JC #direccion
JNC #direccion

JC MI_FUNCION
JNC MI_FUNCION
```

---

## Negative

```asm
JN #direccion
JNN #direccion

JN MI_FUNCION
JNN MI_FUNCION
```

---

## Overflow

```asm
JV #direccion
JNV #direccion

JV MI_FUNCION
JNV MI_FUNCION
```

Todos los saltos utilizan una dirección inmediata como destino.