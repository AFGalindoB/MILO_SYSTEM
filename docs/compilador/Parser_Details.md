# Parser Details - Milo ASM

## Representación Final

El parser de Milo ASM traduce cada línea del programa a una representación intermedia (Intermediate Representation o **IR**) independiente de la codificación binaria utilizada posteriormente por el encoder.

Esta representación describe la semántica de la instrucción mediante un conjunto fijo de campos, separando explícitamente el tipo de operación, su formato sintáctico y los operandos involucrados.

```c
typedef struct {
    TipoInstruccion tipo;
    FormatoInstruccion formato;

    uint32_t linea;

    uint8_t tiene_modificador_f;

    Operando destino[2];
    Operando fuente[2];
} InstruccionIR;
```

Los operandos son representados mediante una estructura uniforme.

```c
typedef struct {
    TipoOperando tipo;
    uint32_t valor;
} Operando;
```

Cada operando identifica tanto el recurso de hardware utilizado como el valor asociado.

Actualmente existen los siguientes tipos:

- Registro de propósito general (RW).
- Registro de solo lectura (RO).
- Registro de solo escritura (WO).
- Valor inmediato.
- Operando nulo.

Gracias a esta organización, todas las instrucciones del ISA son descritas mediante una estructura común, independientemente de su sintaxis particular.

## Filosofía de diseño

El parser fue diseñado como una etapa de traducción entre el lenguaje ensamblador y una representación intermedia independiente del hardware.

Su responsabilidad no consiste en generar código máquina, sino en interpretar la sintaxis del programa, validar su gramática y construir una representación uniforme que posteriormente será consumida por el encoder.

Para ello el proceso de análisis se divide en dos niveles.

1. Clasificación de la instrucción mediante su formato sintáctico.
2. Construcción de una representación intermedia tipada.

Esta separación permite desacoplar completamente el análisis sintáctico de la codificación física del procesador, facilitando la evolución independiente del parser y del encoder.

### Reconocimiento de instrucciones

El reconocimiento de una instrucción se realiza de forma secuencial.

```text
parsear_linea_tokens()
            │
            ▼
Reconocer mnemónico
            │
            ▼
Determinar TipoInstruccion
            │
            ▼
Clasificar FormatoInstruccion
            │
            ▼
Invocar parser especializado
            │
            ▼
Construir InstruccionIR
```

Cada mnemónico es asociado primero a un `TipoInstruccion`, el cual identifica la operación que ejecutará la CPU.

Posteriormente se determina su `FormatoInstruccion`, encargado de describir la gramática que deberá seguir dicha instrucción.

Actualmente existen los siguientes formatos:

- Sin operandos.
- Salto.
- MOV.
- MOVI.
- LOAD.
- STORE.
- ALU unaria.
- ALU binaria.

Esta clasificación permite reutilizar una misma gramática entre múltiples instrucciones.

Por ejemplo, las instrucciones:

- ADD
- SUB
- ADC
- SBC
- AND
- OR
- XOR
- SHL
- SHR
- CMP

comparten exactamente el mismo procedimiento de análisis sintáctico, diferenciándose únicamente por el valor almacenado en `TipoInstruccion`.

Una vez determinado el formato, el parser especializado resuelve automáticamente:

- Modificadores (`.F`).
- Destinos.
- Operandos fuente.
- Referencias a etiquetas cuando corresponda.

Construyendo finalmente la estructura `InstruccionIR`.

### Operandos tipados

Todos los operandos son representados mediante la estructura `Operando`.

Cada operando almacena dos piezas de información:

- El tipo de recurso al que hace referencia.
- El identificador o valor asociado.

Actualmente existen los siguientes tipos:

| Tipo               | Descripción                   |
| ------------------ | ----------------------------- |
| OPERANDO_REG_RW    | Registro de propósito general |
| OPERANDO_REG_RO    | Registro de solo lectura      |
| OPERANDO_REG_WO    | Registro de solo escritura    |
| OPERANDO_INMEDIATO | Valor inmediato               |
| OPERANDO_NULO      | Operando inexistente          |

Gracias a esta representación el parser deja de depender de nombres concretos como `Rd`, `Rs` o `GPU`, delegando esa interpretación al encoder.

## Recuperación ante errores

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

## Destinos ortogonales

Antes de analizar los operandos fuente, el parser resuelve automáticamente los destinos de escritura.

Cada instrucción puede escribir simultáneamente sobre:

Un registro de propósito general.
Un registro de solo escritura.

Los destinos se almacenan en:

- Registro RW.
  ```c
  destino[0]
  ```

- Registro RO.
  ```c
  destino[1]
  ```

Actualmente los registros WO implementados son:

- TBUF
- SCROLL

Este mecanismo permite extender el hardware agregando nuevos periféricos sin modificar la gramática del lenguaje ensamblador.

## Parsers especializados

Cada formato posee una rutina independiente encargada de validar exclusivamente la gramática correspondiente.

Actualmente se implementan:

| Función                    | Formato                                         |
| -------------------------- | ----------------------------------------------- |
| parsear_formato_mov        | MOV                                             |
| parsear_formato_mov_inm    | MOVI                                            |
| parsear_formato_load       | LOAD                                            |
| parsear_formato_store      | STORE                                           |
| parsear_formato_binario    | ADD, SUB, ADC, SBC, AND, OR, XOR, SHL, SHR, CMP |
| parsear_formato_alu_unario | NOT                                             |
| parsear_formato_salto      | JMP, CALL, JZ, etc.                             |

Esta organización evita grandes bloques condicionales y mantiene desacopladas las distintas gramáticas del lenguaje.

## Resolución de etiquetas

Las instrucciones de salto aceptan tanto direcciones inmediatas como etiquetas simbólicas.

Cuando el parser encuentra un identificador en una instrucción de salto:

```asm
JMP LOOP
```

Consulta la tabla global de etiquetas.

Si la etiqueta existe, el identificador es reemplazado por su dirección correspondiente y el operando pasa a ser tratado como un inmediato.

### Extensibilidad

La incorporación de nuevas instrucciones continúa siendo un proceso localizado.

Actualmente incorporar una nueva instrucción suele requerir únicamente:

1. Añadir el nuevo TipoInstruccion.
2. Asociar el mnemónico con un FormatoInstruccion.
3. Reutilizar un parser existente o implementar uno nuevo.
4. Incorporar la traducción correspondiente en el encoder.

La incorporación de nuevos registros especiales normalmente requiere únicamente ampliar el reconocimiento léxico y el tipo de operando correspondiente, manteniendo intacta la lógica del parser.