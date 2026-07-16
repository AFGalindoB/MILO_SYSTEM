# Encoder Details - Milo ASM

## Representacion Final

La salida del codificador corresponde a la estructura `PalabraROM`.

```c
typedef struct {
    uint32_t control_word;
    uint32_t immediate;
} PalabraROM;
```

Esta estructura representa exactamente el formato físico de una posición de la memoria de programa definido por la arquitectura.

## Filosofía de diseño

El codificador fue diseñado siguiendo una filosofía de traducción directa entre la representación del ISA y las señales físicas del procesador.

Cada instrucción reconocida por el parser genera una configuración específica de:

- Opcode
- Selección de registros
- Selector del Bus C
- Fine Control
- Immediate

El resultado no es una secuencia de microinstrucciones ni una representación intermedia adicional.

El resultado es directamente la palabra de control que posteriormente interpretará la unidad de control del procesador.

Esta organización mantiene un fuerte paralelismo entre el software y el hardware, reduciendo la lógica necesaria durante la decodificación de instrucciones.

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
Construir Control Word
        │
        ▼
Generar PalabraROM
```

Durante este proceso desaparece completamente la representación del ISA.

A partir de este punto únicamente existe la representación física utilizada por el procesador.

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

Finalmente todos los campos son combinados mediante operaciones OR para construir la Control Word definitiva.

Esta metodología mantiene completamente desacoplada la lógica de codificación de la distribución física de los bits.

### Relación con el hardware

El codificador constituye el único componente del compilador que conoce la representación binaria del procesador.

Mientras el lexer y el parser únicamente conocen el lenguaje ensamblador, el codificador conoce la organización física descrita en Instruction Encoding Specification.

Esto implica que cualquier modificación sobre el formato de instrucción o sobre las señales de control del procesador afecta únicamente a esta etapa del compilador.

Por este motivo, el codificador actúa como la frontera entre el mundo del software y la implementación física del hardware.