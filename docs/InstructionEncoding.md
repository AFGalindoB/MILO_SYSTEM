# Instruction Encoding Specification - Milo

**Versión:** 2.0.0

# 1. Introducción

Milo utiliza una arquitectura de ejecución determinista en la que cada instrucción define explícitamente las operaciones y rutas de datos que deben activarse durante un ciclo de reloj.

La representación binaria de una instrucción constituye la interfaz entre el programa almacenado en memoria y la lógica de control del procesador. Por esta razón, el formato de instrucción debe permitir seleccionar la operación a ejecutar, sus operandos, las fuentes de datos, los destinos y los mecanismos de acceso a memoria.

Cada instrucción de Milo ocupa exactamente **64 bits**.

La palabra de instrucción se divide en campos destinados a diferentes funciones del datapath. Estos campos permiten seleccionar la operación principal, determinar la fuente de la dirección de memoria, seleccionar registros y rutas de datos, configurar el comportamiento funcional de la unidad correspondiente y transportar datos auxiliares.

La codificación está diseñada para ser interpretada mediante lógica de control cableada (*hardwired control*), utilizando decodificadores, multiplexores, demultiplexores y señales de habilitación.

# 2. Formato general de instrucción

Una instrucción de Milo ocupa una palabra de **64 bits**. Por lo tanto, cada posición de la memoria ROM almacena una palabra de instrucción de 64 bits, distribuida de la siguiente manera:

| Campo                   |     Bits |      Tamaño |
| ----------------------- | -------: | ----------: |
| OPCODE                  |    63:60 |      4 bits |
| MAR Control             |    59:56 |      4 bits |
| Register Bank Selection |    55:44 |     12 bits |
| Bus Control             |    43:36 |      8 bits |
| Fine-Control            |    35:24 |     12 bits |
| Immediate / Payload     |     23:0 |     24 bits |
| **Total**               | **63:0** | **64 bits** |

Para facilitar la interpretación del formato, los campos pueden agruparse según su función dentro de la arquitectura:

* **OPCODE:** identifica la operación principal.
* **MAR Control:** determina la fuente utilizada para obtener la dirección de memoria.
* **Register Bank Selection:** contiene los selectores de los registros utilizados como fuentes y destino.
* **Bus Control:** determina las fuentes seleccionadas para los buses A, B y C.
* **Fine-Control:** configura las señales específicas de la operación seleccionada.
* **Immediate / Payload:** transporta un valor inmediato o información adicional asociada a la instrucción.

## 2.1. OPCODE

El campo `OPCODE` identifica la operación principal que será ejecutada por el procesador.

La unidad de control utiliza este campo para habilitar el bloque funcional correspondiente y determinar la interpretación del campo `Fine-Control`.

| Código | Operación |
| ------ | --------- |
| `0000` | NOP       |
| `0001` | ALU       |
| `0010` | MOV       |
| `0011` | PC        |

Los códigos restantes se encuentran reservados para futuras extensiones de la arquitectura.

## 2.2. Direccionamiento de memoria

El campo `MAR Control` ocupa los bits `59:56` y tiene un tamaño de 4 bits.

En Milo, **MAR no corresponde a un registro físico independiente**. El campo MAR controla la selección de la fuente utilizada para obtener la dirección de memoria.

Esta decisión permite mantener separado el camino de direccionamiento del camino de datos.

El campo MAR se divide conceptualmente en dos partes:

```text
┌─────────┬───────────┐
│ MAR_SRC │  MAR_REG  │
│   1 bit │   3 bits  │
└─────────┴───────────┘
```

### 2.2.1. MAR_SRC

`MAR_SRC` corresponde al bit más significativo del campo MAR:

```text
MAR[3]
```

Determina la fuente utilizada para obtener la dirección de memoria.

| MAR_SRC | Fuente de dirección |
| ------- | ------------------- |
| `0`     | Banco de registros  |
| `1`     | Immediate           |

### 2.2.2. MAR_REG

`MAR_REG` corresponde a los tres bits restantes:

```text
MAR[2:0]
```

Cuando `MAR_SRC = 0`, estos bits seleccionan uno de los ocho registros inferiores del banco de registros:

| MAR_REG | Registro |
| ------- | -------- |
| `000`   | R0       |
| `001`   | R1       |
| `010`   | R2       |
| `011`   | R3       |
| `100`   | R4       |
| `101`   | R5       |
| `110`   | R6       |
| `111`   | R7       |

Por lo tanto, cuando la dirección proviene del banco de registros:

```text
MAR[3]   = 0
MAR[2:0] = registro de dirección
```

Los registros `R0–R7` **continúan siendo registros de propósito general**. Su inclusión en esta selección únicamente significa que pueden ser utilizados como fuente de dirección de memoria mediante el mecanismo de direccionamiento de MAR.

Los registros `R8–R15` continúan disponibles para cualquier función de propósito general.

### 2.2.3. Direccionamiento mediante Immediate

Cuando:

```text
MAR_SRC = 1
```

la dirección de RAM se obtiene directamente del campo `Immediate / Payload` de 24 bits.

En este modo, `MAR[2:0]` no participa en la selección de la dirección.

Conceptualmente:

```text
                         ┌──→ R0–R7
                         │
MAR_SRC = 0 ── MAR_REG ──┤
                         │
                         └──────────→ Dirección RAM

MAR_SRC = 1 ────────────────────────→ Immediate
                                      │
                                      ▼
                                Dirección RAM
```

La incorporación de esta segunda fuente permite acceder directamente a posiciones de memoria sin requerir que una dirección sea cargada previamente en uno de los registros de propósito general.

Esto reduce la presión sobre el banco de registros y evita instrucciones adicionales cuando una operación necesita trabajar directamente con una dirección conocida.

Por ejemplo, una operación que necesite modificar una posición concreta de RAM puede utilizar una dirección inmediata en lugar de reservar un registro exclusivamente para almacenar dicha dirección.

La sintaxis utilizada para expresar esta operación pertenece a la especificación del ISA y no a la codificación binaria.

## 2.3. Selección de registros

El campo `Register Bank Selection` ocupa los bits `55:44` y contiene tres selectores independientes de 4 bits:

* `RegSrcA`
* `RegSrcB`
* `RegDest`

Cada selector permite seleccionar uno de los dieciséis registros de propósito general del banco de registros.

```text
Register Bank Selection
┌──────────┬──────────┬──────────┐
│ RegSrcA  │ RegSrcB  │ RegDest  │
│  4 bits  │  4 bits  │  4 bits  │
└──────────┴──────────┴──────────┘
```

### 2.3.1. RegSrcA

`RegSrcA` ocupa los bits `55:52`.

Selecciona el registro utilizado como fuente cuando el selector del Bus A selecciona el banco de registros.

### 2.3.2. RegSrcB

`RegSrcB` ocupa los bits `51:48`.

Selecciona el registro utilizado como fuente cuando el selector del Bus B selecciona el banco de registros.

### 2.3.3. RegDest

`RegDest` ocupa los bits `47:44`.

Selecciona el registro de destino sobre el cual puede realizarse una escritura cuando la operación correspondiente habilita el banco de registros.

Los tres selectores utilizan la misma codificación:

| Código | Registro |
| ------ | -------- |
| `0000` | R0       |
| `0001` | R1       |
| `0010` | R2       |
| `0011` | R3       |
| `0100` | R4       |
| `0101` | R5       |
| `0110` | R6       |
| `0111` | R7       |
| `1000` | R8       |
| `1001` | R9       |
| `1010` | R10      |
| `1011` | R11      |
| `1100` | R12      |
| `1101` | R13      |
| `1110` | R14      |
| `1111` | R15      |

La selección de un registro no implica por sí misma que dicho registro sea conectado a un bus o que se produzca una escritura. La participación efectiva de cada registro está determinada por los respectivos controles de los buses y por las señales de habilitación de la operación.

## 2.4. Control de buses

El campo `Bus Control` ocupa los bits `43:36` y contiene los selectores correspondientes a los tres buses principales del datapath:

* `Bus A Source Selector`: 2 bits
* `Bus B Source Selector`: 2 bits
* `Bus C Source Selector`: 4 bits

```text
Bus Control
┌────────────────┬────────────────┬──────────────────────┐
│ Bus C Selector │ Bus B Selector │ Bus A Selector       │
│    4 bits      │    2 bits      │    2 bits            │
└────────────────┴────────────────┴──────────────────────┘
```

Milo utiliza tres buses principales para transportar datos dentro del datapath:

* Bus A
* Bus B
* Bus C

Los buses A y B proporcionan operandos a la ALU, mientras que el Bus C distribuye el dato seleccionado hacia los destinos que tengan habilitada su escritura.

### 2.4.1. Bus A Source Selector

El `Bus A Source Selector` corresponde a los bits `37:36` de la instrucción. Dentro del splitter del campo `Bus Control` (`43:36`), corresponde a las posiciones `1:0`.

El Bus A es un bus de entrada de la ALU. Su función es proporcionar el primer operando utilizado por la unidad aritmético-lógica.

| Código | Fuente                              |
| ------ | ----------------------------------- |
| `00`   | Registro seleccionado por `RegSrcA` |
| `01`   | RAM                                 |
| `10`   | ROM (Immediate)                     |
| `11`   | Reserved                            |

Cuando se selecciona `00`, el registro indicado mediante `RegSrcA` es conectado al Bus A.

Cuando se selecciona otra fuente, `RegSrcA` no participa en la selección del dato que alimenta el Bus A.

Por tanto, el camino lógico del Bus A puede representarse como:

```text
RegSrcA ──────────────┐
RAM ──────────────────┼──► MUX A ───► Bus A ───► ALU
Immediate ────────────┘
```

El Bus A no constituye un camino general de distribución de datos. Su función es proporcionar uno de los operandos de entrada de la ALU.

### 2.4.2. Bus B Source Selector

El `Bus B Source Selector` corresponde a los bits `39:38` de la instrucción. Dentro del splitter del campo `Bus Control` (`43:36`), corresponde a las posiciones `3:2`.

El Bus B es un bus de entrada de la ALU. Su función es proporcionar el segundo operando utilizado por la unidad aritmético-lógica.

| Código | Fuente                              |
| ------ | ----------------------------------- |
| `00`   | Registro seleccionado por `RegSrcB` |
| `01`   | RAM                                 |
| `10`   | ROM (Immediate)                     |
| `11`   | Reserved                            |

Cuando se selecciona `00`, el registro indicado mediante `RegSrcB` es conectado al Bus B.

Cuando se selecciona otra fuente, `RegSrcB` no participa en la selección del dato que alimenta el Bus B.

Por tanto, el camino lógico del Bus B puede representarse como:

```text
RegSrcB ──────────────┐
RAM ──────────────────┼──► MUX B ───► Bus B ───► ALU
Immediate ────────────┘
```

El Bus B no constituye un camino general de distribución de datos. Su función es proporcionar uno de los operandos de entrada de la ALU.

Los buses A y B poseen selección de fuente independiente. Por lo tanto, una misma instrucción puede proporcionar a la ALU dos fuentes diferentes.

Por ejemplo:

```text
Bus A = RegSrcA
Bus B = Immediate
```

permite ejecutar una operación de la ALU utilizando simultáneamente un registro y un valor inmediato.

### 2.4.3. Bus C Source Selector

El `Bus C Source Selector` corresponde a los bits `43:40` de la instrucción. Dentro del splitter del campo `Bus Control` (`43:36`), corresponde a las posiciones `7:4`.

A diferencia de los buses A y B, el Bus C constituye el principal camino de distribución de datos hacia el resto del datapath.

El Bus C puede recibir datos provenientes de la ALU o de las fuentes explícitamente conectadas a su multiplexor.

| Código      | Fuente          |
| ----------- | --------------- |
| `0000`      | ALU             |
| `0001`      | RegSrc B        |
| `0010`      | RAM             |
| `0011`      | ROM (Immediate) |
| `0100–1111` | Reserved        |

El camino lógico del Bus C puede representarse como:

```text
ALU ────────────────┐
RegSrc B ───────────┤
RAM ────────────────┤
Immediate ──────────┼──► MUX C ───► Bus C
RINPT ──────────────┘
```

El Bus C puede distribuir su valor hacia diferentes destinos del procesador dependiendo de las señales de control de la instrucción.

Entre sus posibles destinos se encuentran:

* Banco de registros.
* RAM.
* GPU.
* Registros de periféricos.

Por tanto, los buses tienen responsabilidades diferenciadas:

* **Bus A:** entrada del primer operando de la ALU.
* **Bus B:** entrada del segundo operando de la ALU.
* **Bus C:** distribución de datos hacia los destinos del datapath.

Los buses A y B no alimentan directamente los destinos de escritura del procesador.

## 2.5. Fine-Control

El campo `Fine-Control` ocupa los bits `35:24` y contiene las señales de control específicas del bloque funcional seleccionado mediante `OPCODE`.

Por este motivo, la interpretación de este campo depende de la operación principal seleccionada.

Los bits que no son utilizados por una operación se encuentran reservados para dicha operación.

### 2.5.1. NOP

El campo `Fine-Control` no tiene una función asociada a `NOP`.

### 2.5.2. ALU

| Bits   | Nombre de la señal | Tipo        | Descripción física                                                         |
| ------ | ------------------ | ----------- | -------------------------------------------------------------------------- |
| `11:8` | `RESERVED`         | —           | Bits reservados para futuras extensiones.                                  |
| `7`    | `RAM_WE`           | Habilitador | Habilita la escritura en RAM.                                              |
| `6:4`  | `ALU_OP`           | Selección   | Selecciona la operación ejecutada por la ALU.                              |
| `3`    | `CARRY_IN`         | Control     | Permite utilizar el acarreo almacenado en las operaciones de suma y resta. |
| `2`    | `UPDATE_FLAGS`     | Control     | Habilita la actualización de las FLAGS.                                    |
| `1`    | `ENABLE_ALU`       | Control     | Habilita la operación de la ALU.                                           |
| `0`    | `REGS_ENABLE`      | Habilitador | Habilita la escritura en el registro destino.                              |

#### 2.5.2.1. ALU_OP

Este campo determina la operación lógica o aritmética ejecutada por la ALU sobre los operandos presentes en `Bus A` y `Bus B`.

| `ALU_OP` | Operación     | Descripción                                                                      |
| -------- | ------------- | -------------------------------------------------------------------------------- |
| `000`    | `SUM`         | Suma `Bus A + Bus B`.                                                            |
| `001`    | `SUB`         | Resta `Bus A - Bus B`.                                                           |
| `010`    | `AND`         | Operación AND bit a bit entre `Bus A` y `Bus B`.                                 |
| `011`    | `OR`          | Operación OR bit a bit entre `Bus A` y `Bus B`.                                  |
| `100`    | `XOR`         | Operación XOR bit a bit entre `Bus A` y `Bus B`.                                 |
| `101`    | `NOT`         | Complemento bit a bit del operando seleccionado en `Bus A`.                      |
| `110`    | `SHIFT_LEFT`  | Desplazamiento a la izquierda de `Bus A` según la cantidad indicada por `Bus B`. |
| `111`    | `SHIFT_RIGHT` | Desplazamiento a la derecha de `Bus A` según la cantidad indicada por `Bus B`.   |


### 2.5.3. MOV

| Bits   | Nombre de la señal | Tipo        | Descripción física                              |
| ------ | ------------------ | ----------- | ----------------------------------------------- |
| `11:2` | `RESERVED`         | —           | Bits reservados para futuras extensiones.       |
| `1`    | `ENABLE_REGS`      | Habilitador | Habilita la escritura en el banco de registros. |
| `0`    | `RAM_WE`           | Habilitador | Habilita la escritura en RAM.                   |

### 2.5.4. PC

| Bits    | Nombre de la señal     | Tipo        | Descripción física                                         |
| ------- | ---------------------- | ----------- | ---------------------------------------------------------- |
| `11:10` | `RESERVED`             | —           | Bits reservados para futuras extensiones.                  |
| `9`     | `ENABLE_WAIT`          | Habilitador | Habilita la espera del Program Counter.                    |
| `8:7`   | `SEL_FLAG`             | Control     | Selecciona una de las FLAGS.                               |
| `6`     | `NEGATE_FLAG`          | Control     | `0`: FLAGS normales; `1`: FLAGS negadas.                   |
| `5`     | `JMP_CONDITIONAL`      | Habilitador | Habilita el salto condicionado por FLAGS.                  |
| `4`     | `ENABLE_SP`            | Habilitador | Permite modificar el Stack Pointer.                        |
| `3`     | `UPDOWN_STACK_POINTER` | Control     | `1`: incrementa el Stack Pointer; `0`: lo decrementa.      |
| `2`     | `WE_STACK`             | Habilitador | Permite cargar un valor en el Stack.                       |
| `1`     | `SEL_PC`               | Control     | `0`: ROM; `1`: Stack.                                      |
| `0`     | `PC_JUMP`              | Habilitador | Habilita la carga del Program Counter.                     |

## 2.6. Codificación de las FLAGS

Milo Alpha utiliza cuatro flags de estado para representar condiciones producidas por las operaciones de la ALU. Cada flag ocupa un bit independiente y puede inspeccionarse durante la simulación para verificar el comportamiento del hardware.

La codificación de las FLAGS sigue el siguiente orden:

| Bit | Nombre             | Descripción |
| --- | ------------------ | ----------- |
| `0` | `Negative (N)`     | Indica que el resultado de la operación es negativo según la interpretación de números con signo. |
| `1` | `Zero (Z)`         | Indica que el resultado de la operación es igual a cero. |
| `2` | `Carry/Borrow (C)` | Representa la condición de acarreo en operaciones aritméticas o de préstamo en operaciones de resta, según la convención implementada por la ALU. |
| `3` | `Overflow (V)`     | Indica que se produjo un desbordamiento aritmético en una operación con signo. |

Por tanto, el estado de las cuatro FLAGS puede representarse mediante un vector de 4 bits:

```text
┌───────┬───────┬──────────────┬────────────┐
│ Bit 3 │ Bit 2 │    Bit 1     │   Bit 0    │
├───────┼───────┼──────────────┼────────────┤
│   V   │   C   │      Z       │     N      │
└───────┴───────┴──────────────┴────────────┘
```

La representación binaria se expresa como `VCZN`, donde el bit menos significativo corresponde a `N` y el más significativo a `V`.

Estas flags permiten observar las condiciones resultantes de las operaciones aritméticas y lógicas y sirven como referencia para verificar el comportamiento de la ALU y de las instrucciones de control de flujo que dependen del estado del procesador.

Aunque las FLAGS no se seleccionan directamente mediante un campo independiente de la palabra de instrucción, su actualización depende de las señales de control de la operación correspondiente. En particular, el campo `UPDATE_FLAGS` del bloque `ALU` determina si se habilita la actualización del estado de las FLAGS.

Durante la simulación de `CPU.circ`, los cuatro bits pueden inspeccionarse para comprobar que el hardware produce el estado esperado después de ejecutar una instrucción.

## 3. Decodificación de instrucciones

La interpretación de una instrucción se realiza mediante lógica de control cableada.

El `OPCODE` constituye la primera etapa de selección y determina qué lógica funcional debe interpretar la instrucción.

Los campos restantes proporcionan los parámetros necesarios para configurar multiplexores, demultiplexores, habilitadores y unidades funcionales.

Conceptualmente:

```text
                Instruction Word
                       │
                       ▼
                  ┌─────────┐
                  │ OPCODE  │
                  └────┬────┘
                       │
              ┌────────┼────────┐
              ▼        ▼        ▼
             NOP       ALU      MOV ...
              │        │
              ▼        ▼
        Control Logic  Control Logic
              │        │
              └────┬───┘
                   ▼
                 Datapath
```

La decodificación no requiere una memoria de microinstrucciones. Cada combinación válida de `OPCODE` y campos de control activa directamente las señales correspondientes del hardware.

---

# 4. Consideraciones de diseño

El formato de 64 bits mantiene una longitud fija para todas las instrucciones, simplificando la lectura desde ROM y la lógica de decodificación.

La modificación del campo MAR no aumenta el tamaño de la instrucción. El mismo campo de 4 bits se divide internamente entre un bit destinado a seleccionar la fuente de dirección y tres bits destinados a seleccionar un registro cuando la dirección proviene del banco de registros.

Esta organización permite añadir direccionamiento inmediato sin incrementar el ancho de la palabra de instrucción.

El direccionamiento mediante registros conserva ocho posibles fuentes (`R0–R7`), mientras que `R8–R15` permanecen completamente disponibles para operaciones generales.

La existencia de un único campo `Immediate / Payload` de 24 bits mantiene una representación compacta. Cuando una instrucción utiliza el inmediato para más de una función, todas esas funciones hacen referencia al mismo valor inmediato.

La sintaxis utilizada para expresar estas operaciones pertenece al ISA y debe mantenerse separada de la codificación física descrita en este documento.

---

> **Nota de evolución del encoding**
>
> El formato de instrucciones actual reemplaza deliberadamente el encoding utilizado en versiones anteriores de Milo. La modificación surgió a partir de las limitaciones observadas en el acceso a memoria y en el control del datapath del diseño anterior.
>
> En la versión anterior, `RegSrc1` y `RegSrc2` estaban directamente asociados con `Bus A` y `Bus B`, respectivamente. Esto hacía que la selección de los buses dependiera necesariamente del banco de registros y, en particular, que `RegSrc1` quedara comprometido como fuente de la dirección de memoria durante las operaciones de acceso a RAM.
>
> El diseño actual desacopla la selección de los buses de los registros fuente. `Bus A` y `Bus B` poseen ahora selectores independientes que permiten seleccionar registros, RAM o el campo `Immediate / Payload` de manera independiente. Esto permite utilizar ambos buses sin que su fuente tenga que estar determinada por `RegSrcA` o `RegSrcB`, aumentando la flexibilidad del datapath y permitiendo operaciones que no eran representables directamente en el encoding anterior.
>
> Paralelamente, el direccionamiento de memoria se separó de la selección de operandos mediante `MAR Control`. La dirección puede provenir de un registro seleccionado mediante `MAR_REG` o directamente del campo `Immediate / Payload`, evitando que `RegSrc1` deba reservarse para este propósito.
>
> Como consecuencia, el nuevo encoding rompe intencionalmente la compatibilidad con el formato anterior. Esta ruptura permite eliminar el acoplamiento entre registros, buses y direccionamiento de memoria, reducir dependencias innecesarias del banco de registros y aprovechar de forma más flexible los recursos del datapath.