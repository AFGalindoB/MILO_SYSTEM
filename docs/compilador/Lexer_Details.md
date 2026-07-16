# Parser Details - Milo ASM

## Flujo de funcionamiento

El lexer procesa el código fuente carácter por carácter.

Durante este recorrido identifica el comienzo de un elemento léxico, consume los caracteres necesarios para reconocerlo y finalmente produce un token que será entregado al parser.

```text
Código Fuente
      │
      ▼
Lectura carácter por carácter
      │
      ▼
Reconocimiento del patrón
      │
      ▼
Generación del Token
      │
      ▼
Parser
```

El lexer no interpreta el significado de las instrucciones ni verifica que la secuencia de tokens sea válida desde el punto de vista sintáctico. Su única responsabilidad consiste en reconocer correctamente los elementos léxicos definidos por el lenguaje.

## Token

La salida del lexer se encuentra representada mediante la estructura Token.

```c
typedef struct {
    TipoToken tipo;
    char lexema[32];
    uint32_t valor;
    uint32_t linea;
    uint32_t columna;
} Token;
```

Cada token almacena la información necesaria para que las etapas posteriores puedan interpretar correctamente el programa.

| Campo     | Descripción                                                                        |
| --------- | ---------------------------------------------------------------------------------- |
| `tipo`    | Clasificación del token reconocido.                                                |
| `lexema`  | Representación textual encontrada en el código fuente.                             |
| `valor`   | Valor numérico asociado cuando el token representa un dato numérico o un registro. |
| `linea`   | Línea donde comienza el token.                                                     |
| `columna` | Columna donde comienza el token.                                                   |

## Clasificación de tokens

El lexer clasifica cada elemento del lenguaje mediante la enumeración `TipoToken`. Actualmente el compilador reconoce las siguientes categorías léxicas.

### Identificadores

Representan palabras definidas por el lenguaje que posteriormente serán interpretadas por el parser. Ej:

- ADD
- MOV
- LOAD
- STORE

### Registros

Representan registros de propósito general. Ej:

- R0
- R5
- R12

Además del texto original, el lexer almacena directamente el índice numérico del registro dentro del campo valor.

### Valores inmediatos

Representan constantes inmediatas precedidas por el prefijo correspondiente. Ej: #25

El lexer convierte automáticamente la representación textual al valor numérico asociado.

### Modificadores

Representan modificadores que alteran el comportamiento de un mnemónico. Ej:

`ADD.F`

En este caso:

- `ADD` corresponde al identificador.
- `.F` corresponde al modificador.

Esta separación permite que un mismo mnemónico admita múltiples variantes sin incrementar el número de instrucciones del ISA.

### Símbolos de puntuación

El lexer reconoce además distintos símbolos utilizados por la gramática del lenguaje.

| Token              | Símbolo |
| ------------------ | ------- |
| `TOKEN_COMA`       | `,`     |
| `TOKEN_CORCHETE_I` | `[`     |
| `TOKEN_CORCHETE_D` | `]`     |
| `TOKEN_DOSPUNTOS`  | `:`     |

### Control del archivo

El lexer genera además tokens especiales utilizados para controlar el flujo del análisis.

| Token               | Descripción                             |
| ------------------- | --------------------------------------- |
| `TOKEN_SALTO_LINEA` | Final de una instrucción.               |
| `TOKEN_EOF`         | Fin del archivo fuente.                 |
| `TOKEN_ERROR`       | Error léxico durante el reconocimiento. |

## Filosofía de diseño

El lexer fue diseñado para realizar únicamente reconocimiento léxico.

No interpreta instrucciones, no valida operandos y tampoco posee conocimiento del formato físico de las instrucciones ejecutadas por el procesador.

Su única responsabilidad consiste en transformar texto en tokens.

Esta separación permite que modificaciones en el ISA relacionadas con la gramática puedan implementarse únicamente en el lexer y el parser, sin afectar al encoder encargado de generar la codificación física.