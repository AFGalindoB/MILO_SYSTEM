# Lexer Details - Milo ASM

## Flujo de funcionamiento

El lexer constituye la primera etapa del compilador.

Su responsabilidad consiste en recorrer el código fuente carácter por carácter, reconocer los elementos léxicos definidos por el lenguaje y transformarlos en una secuencia de tokens que posteriormente será consumida por el parser.

```text
  Código Fuente
        │
        ▼
Inicialización del Lexer
        │
        ▼
Lectura carácter por carácter
        │
        ▼
Reconocimiento del patrón léxico
        │
        ▼
Generación del Token
        │
        ▼
Agrupación por línea
        │
        ▼
      Parser
```

A diferencia de versiones anteriores, el lexer no entrega un flujo continuo de tokens.

El compilador solicita una línea completa mediante:

```c
obtener_linea_de_tokens()
```

Esta función consume internamente tantos tokens como sean necesarios hasta encontrar un salto de línea o el final del archivo, permitiendo que el parser procese una instrucción completa en cada iteración.

## Token

La salida del lexer se representa mediante la estructura `Token`.

```c
typedef struct {
    TipoToken tipo;
    char lexema[32];
    uint32_t valor;
    uint32_t linea;
    uint32_t columna;
} Token;
```

Cada token contiene toda la información necesaria para las etapas posteriores del compilador.

| Campo     | Descripción                                          |
| --------- | ---------------------------------------------------- |
| `tipo`    | Categoría léxica reconocida.                         |
| `lexema`  | Texto original encontrado en el código fuente.       |
| `valor`   | Valor numérico asociado al token cuando corresponde. |
| `linea`   | Línea donde inicia el token.                         |
| `columna` | Columna donde inicia el token.                       |

Gracias al campo `valor`, el parser no necesita volver a convertir registros o literales numéricos.

## Clasificación de tokens

El lexer reconoce actualmente las siguientes categorías léxicas.

### Identificadores

Representan símbolos que posteriormente serán interpretados por el parser. Ejemplo:

```asm
ADD
MOV
LOOP
WAITV
```

El lexer no diferencia si un identificador corresponde a un mnemónico o una etiqueta; esa decisión pertenece al parser.

### Registros

El lexer distingue tres clases diferentes de registros.

#### Registros de lectura/escritura (RW)

Corresponden al banco general del procesador. Ejemplo:

```asm
R0
R7
R15
```

Su índice se almacena directamente en `valor`.

#### Registros de solo lectura (RO)

Representan recursos hardware que únicamente pueden utilizarse como fuente.

Actualmente existe: `RINPT`

#### Registros de solo escritura (WO)

Representan periféricos del hardware que únicamente aceptan escritura.

Actualmente existen: 

- `TBUF`
- `SCROLL`

El lexer convierte automáticamente estos registros en identificadores internos utilizados posteriormente por el parser y el encoder.

### Valores inmediatos y Números

Se reconocen literales numéricos sin prefijo y los inmediatos utilizan el prefijo #. Ejemplos:

```asm
15
0x100

#25
#0xFF
```

Durante el análisis léxico se convierten automáticamente al entero correspondiente.

Se soportan:

- Decimal
- Hexadecimal

### Modificadores

Representan modificadores que alteran el comportamiento de un mnemónico. Ejemplo: `ADD.F`

Durante el análisis se generan dos tokens independientes:

- `ADD` corresponde `IDENTIFICADOR`.
- `.F` corresponde `MODIFICADOR`.

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

## Reconocimiento léxico

El reconocimiento de un token sigue siempre la misma estrategia.

```text
Leer carácter
       │
       ▼
Determinar categoría
       │
       ├───────────────┐
       ▼               ▼
Consumir patrón    Error léxico
       │
       ▼
Construir Token
       │
       ▼
Continuar análisis
```

Cada categoría posee su propio autómata de reconocimiento, permitiendo que el análisis permanezca completamente desacoplado del parser.

## Filosofía de diseño

El lexer fue diseñado para realizar exclusivamente reconocimiento léxico.

No interpreta instrucciones, no valida operandos, no conoce formatos de instrucciones ni participa en la generación del microcódigo.

Su única responsabilidad consiste en transformar texto en tokens tipados.

Esta separación permite que el parser se concentre únicamente en la gramática del lenguaje, mientras que el encoder permanece completamente desacoplado de la representación textual del programa.