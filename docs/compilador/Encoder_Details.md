# Encoder Details - Milo ASM

## Representacion Final

La salida del codificador corresponde a la estructura `PalabraROM`.

```c
typedef struct {
    uint32_t control_word;
    uint32_t immediate;
} PalabraROM;
```

Cada instancia representa una palabra física almacenada en la memoria ROM del procesador.

La palabra de control contiene todas las señales necesarias para configurar el hardware durante un ciclo de ejecución, mientras que el campo `immediate` almacena el operando inmediato asociado cuando la instrucción lo requiere.

Dependiendo de la instrucción recibida, el encoder puede generar una o varias estructuras `PalabraROM`, las cuales serán ejecutadas secuencialmente por la CPU.

## Filosofía de diseño

El encoder constituye la frontera entre el ISA y la implementación física del procesador.

Recibe una representación completamente independiente del hardware (`InstruccionIR`) y la transforma en una o varias palabras de control compatibles con la arquitectura Milo Alpha.

```text
InstruccionIR
        │
        ▼
Interpretación de operandos
        │
        ▼
Configuración de señales físicas
        │
        ▼
Construcción de PalabraROM
```

A partir de esta etapa desaparecen conceptos propios del lenguaje ensamblador como:

- MOV
- ADD
- LOAD
- registros RW
- registros WO

Y únicamente permanecen señales físicas que posteriormente serán interpretadas por la CPU.

## Flujo de codificación

Todas las instrucciones siguen el mismo flujo general.

```text
InstruccionIR
        │
        ▼
Seleccionar instrucción
        │
        ▼
Resolver operandos
        │
        ▼
Seleccionar señales hardware
        │
        ▼
Empaquetar palabra física
        │
        ▼
PalabraROM[]
```

El encoder nunca analiza texto.

Toda la información necesaria ya fue resuelta previamente por el lexer y el parser.

## Organización interna

La función principal del módulo es: `codificar_instruccion()`.

Internamente la codificación se organiza mediante un `switch` sobre `TipoInstruccion`.

```text
InstruccionIR
        │
        ▼
switch(instr->tipo)
        │
 ┌──────┼─────────────┐
 │      │             │
 ▼      ▼             ▼
ALU   Movimiento   Control de Flujo
 │      │             │
 └──────┴──────┬──────┘
               ▼
      Construcción física
```

Cada familia configura únicamente las señales necesarias para implementar esa operación.

### Empaquetado de la palabra de control

Una vez determinadas las señales de hardware, todas son empaquetadas mediante la función:

```c
empaquetar_campos()
```

Esta función recibe los distintos campos físicos:

- Opcode
- Registro destino
- Registro fuente 1
- Registro fuente 2
- Selector del Bus C
- Fine Control
- Immediate

Posteriormente cada campo es desplazado hasta su posición correspondiente dentro de la palabra de control.

```text
┌──────────┬──────────┬──────────┬──────────┬────────────┬──────────────┐
│ Opcode   │ RegDest  │ RegSrc2  │ RegSrc1  │ Bus C Sel. │ Fine Control │
└──────────┴──────────┴──────────┴──────────┴────────────┴──────────────┘
```

Finalmente todos los campos se combinan mediante operaciones OR para construir la palabra física definitiva.

Gracias a este mecanismo, cualquier modificación del formato binario del procesador queda localizada únicamente dentro del encoder.

## Interpretación de operandos

El encoder no trabaja directamente con registros o inmediatos escritos por el programador.

Toda esa información llega encapsulada mediante la estructura `Operando`.

Durante la codificación el encoder interpreta el tipo de cada operando para decidir cómo debe configurarse el hardware.

Por ejemplo:

- Un registro RW habilita escritura sobre el banco de registros.
- Un registro RO selecciona una fuente especial para el Bus C.
- Un registro WO activa señales específicas del hardware.
- Un inmediato se almacena en el campo `immediate`.

Esta separación mantiene completamente desacoplados el lenguaje ensamblador y la representación física.

## Señales de hardware

Además de seleccionar la operación principal, el encoder configura diversas señales auxiliares del procesador.

Entre ellas se encuentran:

- Escritura sobre registros generales.
- Actualización de flags.
- Selección de operación de la ALU.
- Carry de entrada.
- Escritura sobre Tile Buffer.
- Actualización del registro Scroll.
- Escritura en memoria.
- Habilitación del Stack Pointer.
- Selección de saltos condicionales.
- Selección del Bus C.

Estas señales se agrupan dentro del campo `Fine Control`, cuya interpretación depende del opcode ejecutado.

### Relación con el hardware

El encoder constituye el único módulo del compilador que conoce la codificación física del procesador.

Mientras que:

- El lexer trabaja únicamente con caracteres.
- El parser trabaja con gramática
- El compilador coordina el pipeline.

El encoder conoce:

- Distribución de bits.
- Opcodes físicos.
- Señales de control.
- Funcionamiento interno de la CPU.

Por este motivo cualquier modificación del hardware normalmente sólo requiere cambios dentro de este módulo.

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

## Independencia entre ISA y hardware

Una de las características más importantes del encoder es que no existe una correspondencia obligatoria entre una instrucción del ISA y una palabra física.

```text
            ISA
             │
             ▼
        InstruccionIR
             │
             ▼
          Encoder
             │
      ┌──────┴──────┐
      ▼             ▼
1 PalabraROM   N PalabrasROM
      │             │
      └──────┬──────┘
             ▼
        Programa Final
```

Esta arquitectura permite que el lenguaje ensamblador permanezca estable incluso cuando la implementación física del procesador evoluciona, ya que cualquier cambio en las señales de control o en la cantidad de microoperaciones necesarias queda encapsulado exclusivamente dentro del encoder.