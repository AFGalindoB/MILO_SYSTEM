# Milo ISA

Versión: 2.0.0

## Introducción

Este documento describe el conjunto de instrucciones (*Instruction Set Architecture*, ISA) de la arquitectura Milo.

El ISA constituye la interfaz visible para el programador y define la semántica de cada instrucción, su sintaxis en lenguaje ensamblador y el comportamiento esperado durante su ejecución.

La representación binaria de las instrucciones y su codificación física se describen en el documento [Instruction Encoding Specification](./InstructionEncoding.md).

La arquitectura Milo utiliza un modelo de ejecución basado en un datapath sincronizado por ciclos de reloj. Las operaciones se configuran durante un ciclo y sus efectos de escritura se hacen efectivos en el flanco de reloj correspondiente.

# Notación

| Símbolo | Significado                                                                                                        |
| ------- | ------------------------------------------------------------------------------------------------------------------ |
| `Rn`    | Registro de propósito general.                                                                                     |
| `#n`    | Valor inmediato de 24 bits.                                                                                        |
| `[#n]`  | Acceso a la palabra de RAM cuya dirección corresponde al valor inmediato `#n`.                                     |
| `[Rn]`  | Acceso a la palabra de RAM cuya dirección corresponde al contenido de `Rn`.                                        |
| `RAM`   | Palabra de memoria correspondiente a la dirección establecida por la referencia de acceso a RAM de la instrucción. |
| `Rd`    | Registro destino de propósito general.                                                                             |
| `Ra`    | Primer operando fuente de propósito general.                                                                       |
| `Rb`    | Segundo operando fuente de propósito general.                                                                      |

> **Nota:** El funcionamiento del acceso a memoria RAM, la selección de la dirección mediante `[#n]` o `[Rn]` y las operaciones permitidas sobre memoria se explican al final de este documento, en la sección [Acceso a memoria RAM](#acceso-a-memoria-ram).

## Acceso a memoria RAM

Milo integra la memoria RAM directamente en el datapath, permitiendo que los datos almacenados en memoria participen de forma directa en las operaciones de la arquitectura.

La arquitectura permite establecer la dirección a partir de un registro de propósito general o directamente mediante un valor inmediato.

La RAM utiliza lectura asíncrona y escritura síncrona. Por lo tanto, los datos pueden estar disponibles durante el ciclo de ejecución, mientras que las modificaciones sobre memoria se hacen efectivas en el flanco de reloj correspondiente.

> **Nota:** Nota: Las referencias [Rn] y [#n] establecen la dirección utilizada para el acceso a RAM. Una vez establecida la dirección, el identificador RAM representa el contenido de dicha posición durante la instrucción.
> 
> El uso concreto de la memoria como fuente, destino o dirección de una operación se especificará posteriormente al describir las operaciones disponibles en el ISA.

# Transferencia de datos (MOV)

La instrucción `MOV` permite transferir un valor desde una fuente hacia un destino.

Su sintaxis general es:

```asm
MOV destino, fuente
```

El destino determina dónde se almacenará el valor, mientras que la fuente determina de dónde se obtiene.

La sintaxis oficial de `MOV` es:

```asm
MOV Rn, Rn
MOV Rn, #n
MOV Rn, []
MOV [], Rn
MOV [], #n
```

Las referencias `[]` representan un acceso a RAM. La dirección utilizada por dicho acceso puede especificarse mediante un valor inmediato o mediante un registro, de acuerdo con las reglas de acceso a memoria definidas por la instrucción.

Las combinaciones disponibles son:

| Destino | Fuente | Descripción          |
| ------- | ------ | -------------------- |
| `Rn`    | `Rn`   | Registro → registro  |
| `Rn`    | `#n`   | Inmediato → registro |
| `Rn`    | `[]`   | RAM → registro       |
| `[]`    | `Rn`   | Registro → RAM       |
| `[]`    | `#n`   | Inmediato → RAM      |

> **Nota:** La sintaxis de `MOV` permite utilizar `[]` tanto como destino como fuente, incluyendo la forma `MOV [], RAM`. Debido a que Milo utiliza una RAM de un solo puerto y todas las referencias `RAM` de una misma instrucción corresponden a la posición de memoria establecida por `[]`, esta operación vuelve a escribir en la misma posición el valor que acaba de ser leído. Aunque no tiene una utilidad práctica como transferencia de datos, se mantiene como una combinación sintácticamente válida para conservar la uniformidad de la gramática de la instrucción.


> **Nota:** La instrucción `MOV` reemplaza las instrucciones independientes `MOV` y `MOVI` de versiones anteriores del ISA.

## Transferencia entre registros

Permite copiar el contenido de un registro de propósito general hacia otro.

Ejemplo:

```asm
MOV R1, R0
```

Conceptualmente:

```text
R1 ← R0
```

Se copia el dato contenido en `R0` hacia `R1`. El contenido de `R0` permanece sin modificaciones.

## Valor inmediato a registro

Un valor inmediato puede utilizarse para cargar directamente un valor en un registro de propósito general.

Ejemplo:

```asm
MOV R1, #15
```

Conceptualmente:

```text
R1 ← 15
```

En este caso, `#15` representa directamente el valor que será transferido al registro.

## Lectura desde memoria a registro

La instrucción `MOV` permite transferir el valor almacenado en una posición de RAM hacia un registro de propósito general.

La forma general es:

```asm
MOV Rn, []
```

La referencia `[]` indica que la fuente corresponde a una palabra de RAM. La dirección de memoria puede especificarse mediante un valor inmediato o mediante el contenido de un registro.

### Dirección mediante valor inmediato

La dirección puede indicarse directamente mediante un valor inmediato.

Ejemplo:

```asm
MOV R1, [#0x01]
```

Conceptualmente:

```text
R1 ← RAM[0x01]
```

En este caso, `#0x01` representa directamente la dirección de memoria que se desea consultar.

Por ejemplo:

* Si `RAM[0x01] = 0x0A`.
* Al ejecutar `MOV R1, [#0x01]`.
* `R1` pasará a contener el valor `0x0A`.

### Dirección mediante registro

La dirección también puede obtenerse a partir del contenido de un registro de propósito general.

Ejemplo:

```asm
MOV R1, [R0]
```

Conceptualmente:

```text
R1 ← RAM[R0]
```

En este caso, `R0` no representa el dato que se desea leer, sino la dirección de memoria que se utilizará para realizar el acceso.

Por ejemplo:

* Si `R0 = 0x01`, la referencia `[R0]` corresponde a `RAM[0x01]`.
* Si `RAM[0x01] = 0x0A`.
* Al ejecutar `MOV R1, [R0]`.
* `R1` pasará a contener el valor `0x0A`.

Por lo tanto, `[Rn]` debe interpretarse como la palabra de RAM cuya dirección corresponde al contenido de `Rn`.

> 💡 **Ayuda:** Para interpretar una lectura desde memoria, puede pensarse como: “el registro `Rn` recibirá el valor almacenado en la posición de memoria indicada por `[#n]` o `[Rn]`.

## Escritura desde registro a memoria

La instrucción `MOV` también permite transferir el contenido de un registro de propósito general hacia una posición de RAM.

La forma general es:

```asm
MOV [], Rn
```

La dirección de memoria puede especificarse mediante un valor inmediato o mediante el contenido de un registro.

### Dirección mediante valor inmediato

La posición de memoria puede indicarse directamente mediante un valor inmediato.

Ejemplo:

```asm
MOV [#0x01], R1
```

Conceptualmente:

```text
RAM[0x01] ← R1
```

En este caso, `#0x01` representa directamente la dirección de memoria donde se almacenará el contenido de `R1`.

Por ejemplo:

* Si `R1 = 0x0A`.
* Al ejecutar `MOV [#0x01], R1`.
* `RAM[0x01]` pasará a contener el valor `0x0A`.

El contenido de `R1` no se modifica.

### Dirección mediante registro

La dirección también puede obtenerse a partir del contenido de un registro de propósito general.

Ejemplo:

```asm
MOV [R0], R1
```

Conceptualmente:

```text
RAM[R0] ← R1
```

En este caso, `R0` contiene la dirección de memoria donde se almacenará el contenido de `R1`.

Por ejemplo:

* Si `R0 = 0x01`, la referencia `[R0]` corresponde a `RAM[0x01]`.
* Si `R1 = 0x0A`.
* Al ejecutar `MOV [R0], R1`.
* `RAM[0x01]` pasará a contener el valor `0x0A`.

Los contenidos de `R0` y `R1` no se modifican.

> 💡 **Ayuda:** Para interpretar una escritura en memoria, puede pensarse como: “el valor del registro se almacenará en la posición de memoria indicada por `[#n]` o `[Rn]`”.

## Valor inmediato a memoria

La instrucción `MOV` permite almacenar directamente un valor inmediato en una posición de RAM.

La forma general es:

```asm
MOV [], #n
```

La referencia `[]` puede utilizar un registro de propósito general o un valor inmediato para establecer la dirección de memoria.

### Dirección mediante registro

```asm
MOV [R0], #15
```

Conceptualmente:

```text
RAM[R0] ← 15
```

En este caso, `R0` contiene la dirección de memoria y `#15` corresponde al valor que será almacenado.

El contenido de `R0` no se modifica.

### Dirección mediante inmediato

También es posible utilizar un valor inmediato para establecer directamente la dirección:

```asm
MOV [#15], #15
```

Conceptualmente:

```text
RAM[15] ← 15
```

Esta forma es válida debido a que el formato de instrucción dispone de un único campo `Immediate/Payload` y las dos referencias inmediatas utilizan el mismo valor.

### Regla de reutilización del inmediato

Una misma instrucción puede contener múltiples referencias a `#n`. Todas ellas deben corresponder al mismo valor inmediato, ya que la instrucción dispone de un único campo `Immediate/Payload`.

Por ejemplo:

```asm
ADD R0, #5, #5
```

es válida, mientras que:

```asm
ADD R0, #5, #10
```

no es válida, debido a que requeriría representar dos valores inmediatos diferentes en un único campo.

La misma regla se aplica a las referencias utilizadas para establecer direcciones de memoria y valores de datos.

Por ejemplo:

```asm
MOV [#15], #15
```

es válida porque ambas referencias utilizan el valor `15`.

En cambio:

```asm
MOV [#1], #15
```

no es válida, ya que requiere simultáneamente los valores inmediatos `1` y `15`.

Cuando una instrucción contiene varias referencias a un inmediato, el ensamblador debe verificar que todas correspondan al mismo valor antes de realizar el encoding.

# Instrucciones Aritmético-Lógicas

Las instrucciones aritmético-lógicas utilizan la **Unidad Aritmético-Lógica (ALU)** para realizar operaciones sobre uno o dos operandos.

A diferencia de las instrucciones de transferencia de datos, estas instrucciones no se limitan a copiar un valor entre una fuente y un destino. La ALU recibe los operandos indicados por la instrucción, realiza la operación correspondiente y genera un resultado que puede ser almacenado en un registro de propósito general o en memoria.

Las operaciones binarias utilizan la siguiente estructura:

```asm
MNEMONICO destino, operando A, operando B
```

Donde:

* `MNEMONICO` representa la operación que realizará la ALU.
* `destino` indica dónde se almacenará el resultado.
* `operando A` representa el primer valor utilizado por la operación.
* `operando B` representa el segundo valor utilizado por la operación.

Los operandos pueden provenir de registros de propósito general, memoria o valores inmediatos. El destino puede ser un registro de propósito general o una posición de memoria.

Las combinaciones disponibles son:

| Destino | Operando A | Operando B |
| ------- | ---------- | ---------- |
| `Rd`    | `Ra`       | `Rb`       |
| `Rd`    | `#n`       | `Rb`       |
| `Rd`    | `Ra`       | `#n`       |
| `Rd`    | `#n`       | `#n`       |
| `Rd`    | `Ra`       | `[]`       |
| `Rd`    | `[]`       | `Ra`       |
| `Rd`    | `[]`       | `RAM`      |
| `Rd`    | `#n`       | `[]`       |
| `Rd`    | `[]`       | `#n`       |
| `[]`    | `Ra`       | `Rb`       |
| `[]`    | `#n`       | `Rb`       |
| `[]`    | `Ra`       | `#n`       |
| `[]`    | `#n`       | `#n`       |
| `[]`    | `RAM`      | `Rb`       |
| `[]`    | `Ra`       | `RAM`      |
| `[]`    | `#n`       | `RAM`      |
| `[]`    | `RAM`      | `#n`       |
| `[]`    | `RAM`      | `RAM`      |

Donde:

* `Rd` representa el registro de propósito general donde se almacenará el resultado.
* `Ra` y `Rb` representan registros de propósito general utilizados como operandos.
* `RAM` representa la palabra de memoria correspondiente a la referencia de RAM establecida previamente en la instrucción.
* `#n` representa un valor inmediato de 24 bits.
* `[]` representa una referencia de acceso a RAM.

> **Nota:** Cuando una instrucción utiliza una referencia a RAM, la primera aparición de memoria debe establecer explícitamente la dirección mediante `[#n]` o `[Rn]`. Las apariciones posteriores de `RAM` dentro de la misma instrucción hacen referencia a la misma posición de memoria.

Por ejemplo, una operación que utiliza una posición de RAM más de una vez debe escribirse en ASM de la siguiente forma:

```asm
ADD [#address], #n, RAM
ADD [Rn], #n, RAM
ADD Rd, [#address], RAM
ADD Rd, [Rn], RAM
ADD [#address], RAM, RAM
ADD Rd, Ra, RAM
```

donde `#address` representa una dirección inmediata y `Rn` representa el registro cuyo contenido se utilizará como dirección.

Por lo tanto, una expresión como:

```asm
ADD RAM, RAM, RAM
```

no constituye una sintaxis válida para establecer el acceso a memoria, ya que ninguna de las referencias indica inicialmente la dirección.

La forma correcta sería, por ejemplo:

```asm
ADD [#0x10], RAM, RAM
```

o:

```asm
ADD [R0], RAM, RAM
```

En ambos casos, las tres apariciones de memoria hacen referencia a la misma posición de RAM.

---

## Operaciones binarias

Las operaciones binarias utilizan dos operandos y producen un único resultado.

Las operaciones disponibles son:

* `ADD`
* `ADC`
* `SUB`
* `SBC`
* `AND`
* `OR`
* `XOR`
* `SHL`
* `SHR`
* `CMP`

Por ejemplo:

```asm
ADD R0, R1, R2
```

realiza la suma de `R1` y `R2` y almacena el resultado en `R0`.

También pueden utilizarse operandos provenientes de memoria o valores inmediatos:

```asm
ADD R0, [R1], R2
ADD R0, R1, #5
ADD [R0], R1, RAM
```

En el último ejemplo, `[R0]` establece la dirección de RAM y la referencia posterior `RAM` corresponde a esa misma posición de memoria.

La estructura de la instrucción permanece siempre:

```text
Operación de la ALU, destino, operando A, operando B
```

Lo que cambia es el origen de cada operando y el lugar donde se almacena el resultado.

La interpretación de `[]` y `RAM` como referencias y datos de memoria sigue las reglas de acceso a memoria descritas anteriormente.

### ADC — Add with Carry

La instrucción `ADC` realiza una suma incorporando el estado actual de la bandera de acarreo (`Carry`) como parte de la operación.

Conceptualmente:

```text
Resultado ← Operando A + Operando B + Carry
```

Por ejemplo:

```asm
ADC R0, R1, R2
```

equivale conceptualmente a:

```text
R0 ← R1 + R2 + Carry
```

A diferencia de `ADD`, la instrucción `ADC` utiliza el `Carry` existente antes de realizar la operación. Esto permite realizar sumas de valores cuyo tamaño supera el ancho de los registros, procesando sus partes de menor y mayor peso de forma consecutiva.

### SBC — Subtract with Carry

La instrucción `SBC` realiza una resta incorporando el estado actual de la bandera de acarreo (`Carry`) como parte de la operación.

Conceptualmente:

```text
Resultado ← Operando A - Operando B - Carry
```

Por ejemplo:

```asm
SBC R0, R1, R2
```

equivale conceptualmente a:

```text
R0 ← R1 - R2 - Carry
```

Al igual que `ADC`, `SBC` permite utilizar el estado de `Carry` entre operaciones consecutivas. Esto resulta especialmente útil para realizar restas sobre valores cuyo tamaño supera el ancho de los registros.

### CMP — Compare

La instrucción `CMP` realiza una resta entre sus dos operandos, pero descarta el resultado. Su propósito es determinar la relación entre ambos valores mediante las banderas generadas por la operación.

Conceptualmente:

```text
CMP A, B
→ A - B
```

Por ejemplo:

```asm
CMP R1, R2
```

realiza conceptualmente:

```text
R1 - R2
```

pero el resultado no se almacena en ningún registro ni en memoria.

La utilidad de `CMP` es evaluar una condición sin modificar los operandos originales. Las banderas resultantes pueden utilizarse posteriormente para determinar el comportamiento de instrucciones de salto condicional.

---

## Operación unaria: NOT

La instrucción `NOT` realiza una negación bit a bit sobre un único operando.

A diferencia de las operaciones anteriores, `NOT` es una operación **unaria**, por lo que únicamente requiere un operando.

Su estructura es:

```asm
NOT destino, operando
```

Por ejemplo:

```asm
NOT R1, R0
```

Conceptualmente:

```text
R1 ← NOT R0
```

La instrucción puede utilizar un registro o una posición de memoria como fuente y almacenar el resultado en un registro o en memoria, de acuerdo con las combinaciones permitidas para la operación.

## Actualización de Flags

Las instrucciones aritméticas pueden incorporar el modificador `.F`.

Cuando este modificador está presente, el procesador actualiza el registro de banderas utilizando el resultado de la operación.

Ejemplo:

```asm
ADD.F R0, R1, R2
```

Sin el modificador `.F`, la operación no altera las banderas.

Las banderas generadas por una operación pueden ser utilizadas posteriormente por instrucciones de salto o retorno condicional.

---

# Subrutinas

## Etiquetas

Las etiquetas permiten asignar un nombre simbólico a una dirección del programa.

Su objetivo es facilitar la escritura de saltos, llamadas a subrutinas y referencias dentro del código sin depender directamente de direcciones numéricas.

Una etiqueta se define escribiendo un identificador seguido del carácter `:`.

Ejemplo:

```asm
INICIO:
```

Una vez definida, puede utilizarse como destino en las instrucciones que requieren una dirección.

```asm
JMP INICIO

CALL MI_FUNCION
```

Las etiquetas no generan código máquina. Durante la compilación son reemplazadas por la dirección correspondiente a la instrucción asociada.

## Reglas sintácticas de una etiqueta

* Toda etiqueta debe finalizar con el carácter `:`.
* Cada etiqueta debe poseer un nombre único dentro del programa.
* Una etiqueta puede declararse antes o después de ser utilizada.
* Las etiquetas no ocupan espacio en la ROM.
* Las etiquetas no modifican el contador de programa.

---

## CALL

La instrucción `CALL` realiza una llamada a una subrutina.

```asm
CALL #120
CALL MI_FUNCION
```

El destino puede indicarse directamente mediante una dirección o mediante una etiqueta.

Al ejecutar `CALL`, la dirección de la siguiente instrucción se almacena automáticamente en la pila. Esta dirección será utilizada posteriormente por `RET` para continuar la ejecución desde el punto donde se realizó la llamada.

La arquitectura no implementa variantes condicionales de `CALL`.

---

## RET

La instrucción `RET` finaliza la ejecución de una subrutina y retorna al punto desde el que fue llamada.

```asm
RET
```

La dirección de retorno se obtiene automáticamente desde la pila y se utiliza para continuar la ejecución del programa.

---

# Saltos condicionales

La arquitectura dispone de saltos condicionales basados en el estado de las banderas.

Los saltos condicionales utilizan una dirección como destino. Esta dirección puede indicarse directamente mediante un valor numérico o mediante una etiqueta.

---

## Zero

```asm
JZ #direccion

JNZ #direccion

JZ MI_FUNCION

JNZ MI_FUNCION
```

`JZ` realiza el salto cuando la bandera `Zero` está activa.

`JNZ` realiza el salto cuando la bandera `Zero` no está activa.

---

## Carry

```asm
JC #direccion

JNC #direccion

JC MI_FUNCION

JNC MI_FUNCION
```

`JC` realiza el salto cuando la bandera `Carry` está activa.

`JNC` realiza el salto cuando la bandera `Carry` no está activa.

---

## Negative

```asm
JN #direccion

JNN #direccion

JN MI_FUNCION

JNN MI_FUNCION
```

`JN` realiza el salto cuando la bandera `Negative` está activa.

`JNN` realiza el salto cuando la bandera `Negative` no está activa.

---

## Overflow

```asm
JV #direccion

JNV #direccion

JV MI_FUNCION

JNV MI_FUNCION
```

`JV` realiza el salto cuando la bandera `Overflow` está activa.

`JNV` realiza el salto cuando la bandera `Overflow` no está activa.

---

# Sintaxis del ensamblador

El ensamblador de Milo procesa el código fuente instrucción por instrucción. Cada instrucción posee una sintaxis definida que especifica la cantidad y el tipo de operandos requeridos.

Para que una instrucción sea válida, todos los tokens presentes en la misma línea deben corresponder a la sintaxis definida para dicha instrucción. Los tokens adicionales no forman parte de la instrucción y provocan un error de sintaxis.

Por ejemplo, la siguiente instrucción es válida:

```asm
MOV R2, #2
```

Mientras que la siguiente produce un error:

```asm
MOV R2, #2 ADD SUB XOR
```

La instrucción `MOV R2, #2` contiene todos los operandos requeridos, pero los tokens adicionales `ADD`, `SUB` y `XOR` no corresponden a la sintaxis de `MOV`, por lo que la línea es rechazada por el ensamblador.

Del mismo modo, una etiqueta debe finalizar inmediatamente después del carácter `:`.

La siguiente definición es válida:

```asm
FIN:
```

Mientras que:

```asm
FIN: NOP ADD MOV R0, R1
```

produce un error debido a que los tokens posteriores a `:` no forman parte de la definición de la etiqueta.

Este comportamiento garantiza que cada línea del código fuente corresponda exactamente a una construcción sintácticamente válida y evita que errores de escritura sean interpretados como contenido adicional ignorado por el ensamblador.
