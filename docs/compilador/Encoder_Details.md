# Encoder Details - Milo ASM

## Representacion Final

La salida del codificador corresponde a la estructura `PalabraROM`.

```c
typedef struct {
    uint32_t control_word;
    uint32_t immediate;
} PalabraROM;
```

Cada instancia de esta estructura representa una palabra de control física almacenada en la memoria ROM del procesador.

Dependiendo de la instrucción del ISA, el codificador puede generar una o varias estructuras `PalabraROM`, las cuales serán ejecutadas secuencialmente por la CPU.

## Filosofía de diseño

El codificador fue diseñado siguiendo una filosofía de traducción entre el ISA y la representación física utilizada por el procesador.

Para cada instrucción reconocida por el parser, el encoder decide cómo implementarla mediante una o varias palabras de control compatibles con la arquitectura.

Cada palabra de control define explícitamente la configuración física del hardware mediante los siguientes campos:

- Opcode
- Selección de registros
- Selector del Bus C
- Fine Control
- Immediate

En la mayoría de los casos una instrucción del ISA produce una única palabra de control. Sin embargo, determinadas operaciones pueden expandirse en varias palabras físicas cuando así lo requiere la implementación del procesador.

Esta organización mantiene un fuerte paralelismo entre el software y el hardware, permitiendo que el ISA permanezca independiente de la cantidad de operaciones físicas necesarias para ejecutar cada instrucción.

## Flujo de codificación

Cada instrucción sigue el mismo proceso de transformación.

```text
InstruccionParseada
        │
        ▼
Seleccionar operación
        │
        ▼
Asignar campos físicos
        │
        ▼
Construir una o varias
Palabras de Control
        │
        ▼
Generar PalabraROM[]
```

Durante este proceso desaparece completamente la representación del ISA.

A partir de este punto únicamente existe la representación física que será almacenada en la memoria ROM y posteriormente interpretada por la CPU.

## Organización interna

La función principal del módulo es `codificar_instruccion()`.

Internamente la codificación se organiza mediante un `switch` sobre el tipo de instrucción recibido.

```text
InstruccionParseada
        │
        ▼
switch(instr->tipo)
        │
        ├───────────────┐
        │               │
        ▼               ▼
 Operaciones ALU    Transferencias
        │               │
        └───────┬───────┘
                ▼
      Construcción Control Word
```

Cada caso del `switch` define explícitamente cómo debe configurarse el hardware para ejecutar esa instrucción.

### Construcción de la palabra de control

El proceso de codificación se realiza en dos etapas.

En primer lugar se construyen de forma independiente todos los campos que forman la instrucción física.

```text
Opcode
RegDest
RegSrc1
RegSrc2
Bus C Selector
Fine Control
Immediate
```

Una vez calculados estos valores, cada campo se desplaza hasta su posición correspondiente dentro de la palabra de 32 bits.

```text
┌──────────┬──────────┬──────────┬──────────┬────────────┬──────────────┐
│ Opcode   │ RegDest  │ RegSrc2  │ RegSrc1  │ Bus C Sel. │ Fine Control │
└──────────┴──────────┴──────────┴──────────┴────────────┴──────────────┘
```

Finalmente todos los campos son combinados mediante operaciones OR para construir una palabra de control física.

Cuando una instrucción requiere varias operaciones físicas, este procedimiento se repite para cada una de las palabras de control generadas.

Esta metodología mantiene completamente desacoplada la lógica de codificación de la distribución física de los bits.

### Relación con el hardware

El codificador constituye el único componente del compilador que conoce la representación binaria del procesador.

Mientras el lexer y el parser únicamente conocen el lenguaje ensamblador, el codificador conoce la organización física descrita en Instruction Encoding Specification.

Esto implica que cualquier modificación sobre el formato de instrucción o sobre las señales de control del procesador afecta únicamente a esta etapa del compilador.

Por este motivo, el codificador actúa como la frontera entre el mundo del software y la implementación física del hardware.

El encoder constituye además el único componente del compilador autorizado para decidir cómo una instrucción del ISA se traduce físicamente dentro de la ROM.

Esto permite que el hardware evolucione sin modificar la sintaxis del lenguaje ensamblador. Cambios en la cantidad de palabras de control necesarias para implementar una instrucción afectan únicamente al encoder, manteniendo estable el resto del pipeline de compilación.

### Expansión de instrucciones

Aunque la mayoría de instrucciones del ISA generan una única palabra de control, el encoder permite expandir una instrucción en varias operaciones físicas cuando resulta necesario.

Por ejemplo, la instrucción `RET` actualmente se implementa mediante dos palabras de control consecutivas:

```text
RET
 │
 ├────────► SP = SP - 1
 │
 └────────► PC = STACK[SP]
```

La responsabilidad de realizar esta expansión pertenece exclusivamente al encoder. El parser continúa trabajando con una única instrucción del ISA y el resto del compilador permanece completamente ajeno a esta implementación.