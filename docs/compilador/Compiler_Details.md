# Compiler_Details

## Introducción

El compilador de Milo traduce programas escritos en lenguaje ensamblador (MILO ASM) hacia la representación física utilizada por el procesador. Su implementación sigue una arquitectura modular organizada en etapas especializadas que incluyen el análisis léxico, el análisis sintáctico, la resolución de símbolos, la codificación de instrucciones y la generación de la imagen final de la ROM.

Esta organización mantiene desacopladas las distintas responsabilidades del proceso de compilación, permitiendo que el ISA evolucione independientemente de su implementación física.

Este documento describe exclusivamente la organización interna del compilador y la función de cada uno de sus módulos.

## Organización del compilador

```text
compilador/
│
├── lexer/
├── parser/
├── encoder/
├── helpers/
├── tests/
├── makefiles/
└── Makefile
```

Cada directorio implementa una etapa específica del proceso de compilación.

## Flujo general

Durante la compilación existen distintos niveles de representación de un mismo programa.

```text
  Código Fuente
        │
        ▼
Preescaneo de etiquetas
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
    Imagen ROM
        │
        ▼
    Archivo ROM
```

Cada etapa consume la representación producida por la etapa anterior y genera una representación de menor nivel hasta obtener la codificación física que será interpretada por el procesador.

Es importante destacar que no existe una correspondencia obligatoria entre una instrucción del ISA y una única palabra de control y que hay cosas que no pasan directamente a las palabras de control como las etiquetas donde estas mismas son administradas por el compilador convirtiendolas durante el proceso de parseo en una direccion fisica.

Durante la etapa de codificación, una instrucción del lenguaje ensamblador puede traducirse en una o varias palabras de control físicas dependiendo de la implementación de la arquitectura.

Por ejemplo, una instrucción como `RET` requiere actualmente dos palabras de control consecutivas para restaurar el estado del Stack Pointer y posteriormente cargar el Program Counter desde la pila.

Esta expansión es completamente transparente para el resto del compilador y constituye una responsabilidad exclusiva del encoder.

## Compilación en dos pasadas

El compilador utiliza una estrategia de dos pasadas para permitir referencias hacia etiquetas definidas posteriormente en el código fuente.

Durante la primera pasada se recorren todas las instrucciones con el único objetivo de construir la tabla de símbolos y calcular la dirección física asociada a cada etiqueta.


```text
Código Fuente
      │
      ▼
Primera pasada
      │
      ▼
Tabla de símbolos
```

Una vez conocida la posición de todas las etiquetas, el compilador reinicia el lexer y ejecuta una segunda pasada completa.

Durante esta etapa las instrucciones son analizadas normalmente por el parser, el cual puede resolver inmediatamente cualquier referencia simbólica consultando la tabla de símbolos.

```text
Tabla de símbolos
       │
       ▼
Segunda pasada
       │
       ▼
     Lexer
       │
       ▼
     Parser
       │
       ▼
    Encoder
```

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

## Encoder

El codificador constituye la última etapa del proceso de compilación.

Su responsabilidad consiste en transformar la representación intermedia producida por el parser en la representación física utilizada por el procesador.

En esta etapa todas las instrucciones del ISA ya fueron reconocidas y validadas. El encoder decide cómo implementar físicamente cada una de ellas mediante una o varias palabras de control compatibles con la CPU.

Como consecuencia, no existe una relación obligatoria de uno a uno entre el ISA y el Instruction Encoding. Algunas instrucciones generan una única palabra de control, mientras que otras pueden expandirse en varias microinstrucciones físicas cuando así lo requiere la arquitectura.

Para mas detalles revisar: [Encoder Details](./Encoder_Details.md)

## Helpers

Además de coordinar las distintas etapas del compilador, este módulo administra el ciclo completo de compilación. Entre sus responsabilidades se encuentran la construcción de la tabla de símbolos, la ejecución de ambas pasadas, la expansión de instrucciones, la construcción de la imagen final de la ROM, la emisión de diagnósticos y la exportación del programa.

Para mas detalles revisar: [Helpers Details](./Helpers_Details.md)

### Resolución de símbolos

El compilador mantiene una tabla global de símbolos utilizada para asociar cada etiqueta del programa con su dirección física dentro de la memoria ROM.

Cada entrada almacena:

- Nombre de la etiqueta.
- Dirección física.

Durante la primera pasada todas las etiquetas son registradas.

Posteriormente el parser consulta esta tabla para reemplazar automáticamente los identificadores simbólicos por direcciones inmediatas durante la segunda pasada.

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
 Test Etiquetas
      │
      ▼
 Test Encoder
      │
      ▼
 Imagen ROM
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
         Compilador
             │
 ┌───────────┴───────────┐
 │                       │
 ▼                       ▼
Tabla símbolos       Pipeline
                         │
                         ▼
               Lexer → Parser → Encoder
                         │
                         ▼
                     Imagen ROM
                         │
                         ▼
                     Archivo ROM
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