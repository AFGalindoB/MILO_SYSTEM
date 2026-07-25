# Testing Specification

Versión 1.0.0

## Introducción

El sistema de pruebas de Milo tiene como objetivo verificar el correcto funcionamiento tanto del compilador como de las herramientas que conforman el ecosistema de desarrollo.

Las pruebas se encuentran organizadas siguiendo la misma separación de responsabilidades utilizada durante el diseño del compilador. Cada conjunto de tests valida una etapa específica del pipeline de compilación o un conjunto concreto de funcionalidades del ISA.

Esta organización permite detectar errores de forma temprana, aislar la causa de un fallo y validar la evolución del proyecto sin necesidad de inspeccionar manualmente la codificación generada.

## Organización de las pruebas

Las pruebas de Milo se organizan según el componente de la arquitectura que validan.

```text
tests/
│
├── lexer.c
│
├── parser.c
│
└── compilacion/
```

Actualmente existen dos categorías principales.

### Pruebas del compilador

Verifican el correcto funcionamiento de las distintas etapas del proceso de compilación.

Estas pruebas se ejecutan de forma aislada sobre módulos específicos, permitiendo validar el análisis léxico y sintáctico sin depender del hardware.

Su objetivo es garantizar que el ISA sea interpretado correctamente antes de generar la codificación física.

### Pruebas de compilación

Las pruebas de compilación validan el funcionamiento completo del flujo de traducción del ISA.

Estas pruebas utilizan programas escritos en lenguaje ensamblador que son procesados por el lexer, el parser y el encoder para producir las palabras de control que posteriormente ejecutará el procesador.

En consecuencia, dependen del funcionamiento correcto de todas las etapas anteriores del compilador.

Además de verificar que la traducción hacia el Instruction Encoding sea correcta, estas pruebas constituyen una herramienta de validación del propio hardware, ya que las palabras de control generadas son las mismas que posteriormente serán ejecutadas por el procesador.

Esto permite utilizar un mismo programa de prueba tanto para comprobar el comportamiento del compilador como para validar que la implementación hardware responde correctamente ante las instrucciones generadas.

> Cada prueba es completamente independiente y puede ejecutarse de forma individual.

## Compilación de las pruebas y ejecucion

Cada prueba del proyecto constituye un ejecutable independiente que puede compilarse y ejecutarse de forma aislada.

Para simplificar el flujo de trabajo, Milo utiliza un conjunto de Makefiles especializados organizados por categoría de pruebas. El Makefile principal actúa como punto de entrada y delega la compilación hacia los Makefiles correspondientes.

### Compilar todas las pruebas

Compila todos los testbench disponibles del proyecto.

```bash
make
```

o de forma equivalente:

```bash
make all
```

Este comando genera todos los ejecutables de pruebas del compilador y de compilación.

### Compilar únicamente las pruebas del compilador

Compila los test encargados de validar el análisis léxico y sintáctico.

```bash
make tests
```

También es posible compilar cada prueba de forma individual.

```bash
make test_lexer
make test_parser
```

### Compilar únicamente las pruebas de compilación

Compila los test encargados de generar programas para la ROM del procesador.

```bash
make compilacion
```

También pueden compilarse individualmente.

```bash
make test_movi
make test_movimiento_general
make test_alu
make test_flags
make test_pc
make test_gpu
make test_inputs
make test_etiquetas
```

### Limpiar los ejecutables generados

El siguiente comando elimina todos los ejecutables creados durante la compilación de las pruebas.

```bash
make clean
```

### Ejecución de las pruebas

La compilación únicamente genera los ejecutables correspondientes. Cada prueba debe ejecutarse manualmente para analizar sus resultados.

**Pruebas del compilador:**

Las pruebas del compilador muestran directamente su resultado por la salida estándar (terminal).

Su objetivo es facilitar la inspección del comportamiento del lexer y del parser, mostrando los tokens reconocidos, las instrucciones parseadas o los errores detectados durante el análisis.

**Pruebas de compilación:**

Las pruebas de compilación generan programas compilados compatibles con la arquitectura Milo.

Cuando la compilación finaliza correctamente, el resultado se exporta como un archivo de texto dentro del directorio: `compilaciones/instrucciones`

Cada archivo contiene la representación binaria de las palabras de control generadas por el compilador, lista para ser utilizada por la memoria ROM del procesador o por los test hardware correspondientes.

### Flujo general

```text
Seleccionar prueba
        │
        ▼
Compilar (make ...)
        │
        ▼
Ejecutar ejecutable
        │
        ├──────────────┐
        ▼              ▼
Salida por       Archivo ROM
terminal         compilaciones/*.txt
```

## Tipos de pruebas

La documentación detallada de cada conjunto de pruebas se mantiene en documentos independientes.

- [Pruebas del análisis léxico del lenguaje ensamblador.](./Test_Lexer.md)
- [Pruebas del análisis sintáctico y reconocimiento del ISA.](./Test_Parser.md)
- Pruebas de Compilacion:
  - [Pruebas de compilación para operaciones aritmético-lógicas.](./Test_ALU.md)
  - [Pruebas de compilación para instrucciones con operandos inmediatos.](./Test_MOVI.md)
  - [Pruebas de compilación para transferencias entre registros y memoria.](./Test_Movimiento_General.md)
  - [Pruebas de compilación relacionadas con la actualización de banderas.](./Test_Flags.md)
  - [Pruebas de compilación relacionadas con control de flujo.](./Test_PC.md)
  - [Pruebas de compilación relacionadas con el renderizado de la gpu.](./Test_GPU.md)
  - [Pruebas de compilación relacionadas con manejo de entradas.](./Test_Input.md)
  - [Pruebas de compilación relacionadas con manejo de etiquetas.](./Test_Etiquetas.md)

Cada documento describe los objetivos de la prueba, los casos cubiertos y los criterios utilizados para validar su funcionamiento.

## Filosofía del sistema de pruebas

El sistema de pruebas de Milo sigue la misma organización modular utilizada por el compilador.

Cada conjunto de pruebas valida una etapa concreta del proceso de compilación. Sin embargo, estas etapas no son independientes entre sí, sino que forman una cadena de dependencias donde cada nivel utiliza el resultado producido por el anterior.

```text
Código Fuente
      │
      ▼
Tests del Lexer
      │
      ▼
Tests del Parser
      │
      ▼
Tests de Compilación
      │
      ▼
Instruction Encoding
      │
      ▼
Hardware
```

En consecuencia, una etapa solo puede validarse correctamente si las anteriores funcionan como se espera.

- Si el lexer reconoce incorrectamente los tokens, el parser recibirá información inválida
- Si el parser interpreta incorrectamente el ISA, el encoder generará palabras de control erróneas.
- Si el encoder produce una codificación incorrecta, las pruebas de compilación generarán programas incompatibles con el hardware.

Por este motivo, las pruebas suelen ejecutarse de forma progresiva, comenzando por los niveles más bajos del compilador y avanzando hacia las pruebas de compilación, que validan el funcionamiento completo del pipeline de traducción.
