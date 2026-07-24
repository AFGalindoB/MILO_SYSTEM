# Parser Details - Milo ASM

## Representación Final

La representación intermedia utilizada durante esta etapa se encuentra definida por la estructura `InstruccionParseada`.

A diferencia de versiones anteriores, esta estructura ya no se organiza únicamente alrededor del tipo de instrucción, sino que separa explícitamente destinos, fuentes y unidades funcionales, permitiendo que una misma operación pueda escribir simultáneamente sobre registros generales y registros especiales del hardware.

```c
typedef struct {
    TipoInstruccion tipo;
    uint32_t linea;

    uint8_t reg_gpu;

    int tiene_rd;
    uint8_t rd;

    union {
        ...
    } fuentes;
} InstruccionParseada;
```

Esta representación constituye una capa de abstracción entre el ISA y el encoder, permitiendo describir qué recursos del procesador participan en una instrucción sin depender todavía de la codificación física de la palabra de control.

## Filosofía de diseño

El parser fue diseñado siguiendo una arquitectura basada en especialización por familias de instrucciones.

La función principal identifica el mnemónico correspondiente y delega el análisis sintáctico al módulo especializado encargado de esa categoría de instrucciones.

```text
parsear_linea_tokens()
            │
 ┌──────────┼──────────┐
 │          │          │
 ▼          ▼          ▼
Movimiento  ALU   Control de Flujo
```

Actualmente existen tres familias principales:

- Movimiento de datos
- Operaciones ALU
- Control de flujo
- Sincronización con GPU

Cada módulo conoce exclusivamente la gramática de su propia familia y Cada familia utiliza exclusivamente la información necesaria para su unidad funcional.

Esto mantiene desacopladas las distintas categorías del ISA y facilita la incorporación de nuevas instrucciones sin modificar el resto del parser.

### Reconocimiento de instrucciones

El proceso de análisis sintáctico de cada línea sigue siempre la misma secuencia.

```text
Leer mnemónico
        │
        ▼
Seleccionar familia
        │
        ▼
Resolver destinos ortogonales
        │
        ▼
Analizar operandos fuente
        │
        ▼
Construir InstruccionParseada
```

Esta separación permite reutilizar el mismo mecanismo de destinos para todas las familias del ISA.

### Recuperación ante errores

El parser implementa un mecanismo de recuperación conocido como panic mode.

Cuando una instrucción contiene un error sintáctico, únicamente dicha línea es descartada.

```text
Error
   │
   ▼
Registrar diagnóstico
   │
   ▼
Continuar con la siguiente línea
```

Este mecanismo evita la propagación de errores en cascada y permite que una única compilación detecte múltiples errores sintácticos antes de finalizar.

Además de evitar errores en cascada, el parser conserva información suficiente para generar un diagnóstico detallado al finalizar la compilación.

Cada error registrado almacena:

- Línea.
- Columna.
- Línea reconstruida.
- Tipo de token esperado.
- Mensaje descriptivo.

Posteriormente el compilador genera un informe indicando visualmente la posición del error.

Por ejemplo:

```text
[1] Error sintáctico en Línea 8, Columna 12

Detalle:
Se esperaba una coma.

     8 | ADD R1 R2 R3
                ^
```

Este mecanismo evita errores en cascada y permite detectar múltiples problemas durante una misma compilación.

### Destinos ortogonales

Una de las principales características del parser actual es el soporte para destinos ortogonales.

Antes de interpretar los operandos propios de cada instrucción, el parser identifica automáticamente el destino físico sobre el cual deberá escribirse el resultado.

Actualmente existen dos tipos de destino.

- **Registros generales:** El resultado será almacenado en el banco de registros.
- **Registros especiales:** En estos casos el resultado no se escribe en un registro general sino directamente sobre un periférico del sistema.

Actualmente se encuentran implementados:

- TBUF
- SCROLL

El parser detecta automáticamente estos registros especiales y almacena dicha información mediante el campo: `reg_gpu`

Esta organización permite que una instrucción pueda escribir directamente sobre hardware sin introducir instrucciones especiales dentro del ISA.


### Extensibilidad

La incorporación de nuevas instrucciones continúa siendo un proceso localizado.

Generalmente basta con:

1. Añadir el nuevo valor en TipoInstruccion.
2. Incorporar su reconocimiento en parsear_linea_tokens().
3. Implementar la gramática correspondiente dentro de una familia existente o crear una nueva
4. Añadir la traducción correspondiente en el encoder.

La incorporación de nuevos registros especiales únicamente requiere ampliar el mecanismo de resolución de destinos ortogonales, sin modificar el resto del parser.