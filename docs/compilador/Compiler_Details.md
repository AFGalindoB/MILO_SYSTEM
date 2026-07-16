# Compiler_Details

Versión Alpha 0.1

## Introducción

El compilador de Milo Alpha constituye el puente entre el ISA de la arquitectura y la representación física utilizada por el procesador.

Su responsabilidad consiste en traducir programas escritos en lenguaje ensamblador hacia la codificación binaria definida por el Instruction Encoding Specification, generando las palabras de control que posteriormente serán ejecutadas por la CPU.

La implementación del compilador sigue una arquitectura modular en la que cada etapa posee una única responsabilidad claramente definida. Esta organización facilita la incorporación de nuevas instrucciones al ISA, simplifica el mantenimiento del código y permite validar cada componente mediante pruebas independientes.

Este documento describe exclusivamente la organización interna del compilador y la función de cada uno de sus módulos.

## Organización del compilador

```text
compilador/
│
├── lexer/
├── parser/
├── codificador/
├── helpers/
├── tests/
├── makefiles/
└── Makefile
```

Cada directorio implementa una etapa específica del proceso de compilación.

## Flujo general

Durante la compilación existen distintos niveles de representación de un mismo programa.

```text
Código Fuente (ISA)
        │
        ▼
      Lexer
        │
        ▼
      Parser
        │
        ▼
     Encoder
        │
        ▼
Instruction Encoding
        │
        ▼
   Archivo ROM
```

Cada etapa consume la representación producida por la etapa anterior y genera una representación de menor nivel hasta obtener la codificación física que será interpretada por el procesador.

## Lexer

El analizador léxico constituye la primera etapa del compilador.

Su responsabilidad consiste en recorrer secuencialmente el código fuente y transformar una secuencia de caracteres en una secuencia de tokens, eliminando la necesidad de que las etapas posteriores trabajen directamente sobre texto.

Cada token representa una unidad léxica del lenguaje ensamblador y contiene tanto su clasificación como la información necesaria para localizarlo dentro del archivo fuente.

Para mas detalles revisar: [Lexer Details](./Lexer_Details.md)

## Parser

El parser constituye la segunda etapa del proceso de compilación.

Su responsabilidad consiste en interpretar la secuencia de tokens producida por el lexer, verificar que cada instrucción cumpla la gramática definida por el lenguaje ensamblador y construir una representación estructurada que posteriormente será utilizada por el codificador.

A diferencia del lexer, que únicamente reconoce elementos individuales del lenguaje, el parser comprende la relación existente entre dichos elementos y determina si forman una instrucción válida del ISA.

Para mas detalles revisar: [Parser Details](./Parser_Details.md)

## Codificador (Encoder)

El codificador constituye la última etapa del proceso de compilación.

Su responsabilidad consiste en transformar la representación intermedia producida por el parser en la representación binaria utilizada por el procesador.

A diferencia del parser, el codificador ya no trabaja con el lenguaje ensamblador. En esta etapa todas las instrucciones ya fueron reconocidas y validadas; únicamente resta convertirlas en la palabra de control que será almacenada en la memoria ROM.

Para mas detalles revisar: [Encoder Details](./Encoder_Details.md)

## Helpers

El módulo Helpers agrupa los componentes encargados de coordinar el funcionamiento del compilador y de materializar el resultado final del proceso de compilación.

A diferencia del lexer, parser y codificador, estos módulos no implementan una etapa específica del pipeline. Su responsabilidad consiste en integrar las distintas etapas y proporcionar los mecanismos necesarios para producir el archivo final compatible con la memoria de programa del procesador.

Para mas detalles revisar: [Helpers Details](./Helpers_Details.md)

## Tests

El sistema de pruebas del compilador se encuentra organizado siguiendo la misma separación de responsabilidades utilizada durante el proceso de compilación.

Cada conjunto de pruebas valida una etapa específica del pipeline, permitiendo detectar errores de forma aislada antes de que afecten a las etapas posteriores.

Esta organización facilita la evolución tanto del ISA como del codificador, ya que permite verificar independientemente cada nivel de abstracción.

### Filosofía de las pruebas

La arquitectura de pruebas de Milo Alpha sigue la misma separación de niveles utilizada por el resto del proyecto.

```text
Código Fuente
      │
      ▼
 Test Lexer
      │
      ▼
 Test Parser
      │
      ▼
 Test Codificador
      │
      ▼
Palabras de Control
```

Cada nivel verifica únicamente la responsabilidad que le corresponde.

De esta forma, un error léxico no afecta las pruebas sintácticas, un error sintáctico no compromete la validación del codificador y una modificación en la codificación del procesador únicamente requiere actualizar las pruebas relacionadas con el encoder.

## Filosofía de diseño

El compilador de Milo Alpha sigue una arquitectura por etapas (compilation pipeline), donde cada módulo posee una única responsabilidad claramente definida y opera sobre un nivel distinto de abstracción.

Cada etapa transforma la representación producida por la anterior sin depender de los detalles internos de las demás.


```text
Código Fuente
      │
      ▼
Lexer
(Tokens)
      │
      ▼
Parser
(Instrucción del ISA)
      │
      ▼
Encoder
(Palabra de Control)
      │
      ▼
ROM
```

Las responsabilidades de cada módulo son las siguientes:

- **Lexer:** transforma el código fuente en una secuencia de tokens.
- **Parser:** interpreta esos tokens según la gramática del lenguaje ensamblador y construye una representación estructurada de una instrucción del ISA.
- **Encoder:** traduce la representación del ISA hacia la codificación binaria utilizada por el procesador.

Esta separación refleja la filosofía general de Milo Alpha, donde el ISA constituye una abstracción destinada al programador, mientras que el procesador únicamente ejecuta palabras de control.

Como consecuencia, la evolución del lenguaje ensamblador y la evolución de la codificación física pueden mantenerse parcialmente desacopladas.

Por ejemplo:

- Una modificación en la codificación física de una instrucción suele requerir únicamente cambios en el encoder.
- Una modificación en la sintaxis del lenguaje ensamblador normalmente afecta al parser, sin necesidad de modificar la implementación hardware.
- La incorporación de una nueva instrucción requiere extender tanto el parser como el encoder y, cuando corresponda, añadir el soporte físico dentro del procesador.

Esta organización reduce el acoplamiento entre los distintos módulos del compilador, facilita el mantenimiento del proyecto y permite evolucionar el hardware y las herramientas de programación de forma coordinada.