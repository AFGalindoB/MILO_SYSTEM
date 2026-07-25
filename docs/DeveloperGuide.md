# Developer Guide - MILO SYSTEM

Versión: 1.0.0

## Introducción

Milo System es un proyecto de investigación cuyo objetivo es desarrollar un ecosistema completo alrededor de una arquitectura de procesador propia.

A diferencia de muchas arquitecturas tradicionales, el desarrollo de Milo parte de la implementación física del procesador. El datapath, las señales de control y la organización interna del hardware constituyen la base sobre la que posteriormente se construyen las herramientas de programación.

En consecuencia, el conjunto de instrucciones (ISA) no define el funcionamiento interno del procesador, sino que actúa como una capa de abstracción destinada a facilitar su programación. La CPU ejecuta directamente palabras de control, mientras que el ensamblador traduce las instrucciones del ISA hacia dicha representación física.

El proyecto no se limita únicamente al diseño del hardware. Cada nueva capacidad incorporada al procesador requiere también el desarrollo de las herramientas de software necesarias para programarlo, validarlo y facilitar su evolución.

Por este motivo, Milo evoluciona mediante dos líneas de trabajo estrechamente relacionadas: la implementación del hardware y el desarrollo de su ecosistema de software.

Este documento presenta una visión general del proyecto y sirve como punto de partida para comprender cómo se relacionan sus distintos componentes antes de profundizar en la documentación técnica de cada uno de ellos.

## Arquitectura del proyecto

El desarrollo de Milo se organiza actualmente en dos componentes principales que evolucionan de forma coordinada.

- **Hardware**, responsable de implementar físicamente el procesador mediante su datapath, unidad de control y bloques funcionales.
- **Software**, responsable de proporcionar las herramientas necesarias para programar, compilar y validar dicha implementación hardware.

```text
                 Milo
                   │
      ┌────────────┴────────────┐
      │                         │
  Hardware                  Software
      │                         │
      ▼                         ▼
   CPU.circ                MILO ASM
```

Aunque ambos componentes evolucionan conjuntamente, cumplen responsabilidades claramente diferenciadas.

El hardware define las capacidades reales del procesador y las señales físicas que pueden ejecutarse.

El software construye las abstracciones necesarias para utilizar dichas capacidades, proporcionando un lenguaje ensamblador (ISA), un compilador y herramientas de desarrollo que ocultan los detalles de la codificación binaria.

Esta separación permite que el ISA evolucione independientemente de la representación física de las instrucciones y facilita que una misma operación visible para el programador pueda implementarse mediante distintas estrategias de codificación sin modificar la interfaz expuesta al usuario.

## Software

La capa de software proporciona las herramientas necesarias para programar la arquitectura Milo.

Actualmente esta capa está compuesta por el compilador MILO ASM, cuya función consiste en traducir programas escritos utilizando el ISA de Milo hacia la representación binaria que comprende el procesador.

Durante este proceso el compilador interpreta el lenguaje ensamblador, valida su sintaxis resuelve etiquetas y finalmente genera la codificación física que será almacenada en la memoria ROM del procesador.

Dependiendo de la instrucción, una operación del ISA puede traducirse en una o varias palabras de control físicas. Esto permite que el compilador implemente abstracciones de mayor nivel utilizando múltiples microinstrucciones sin modificar la interfaz ofrecida al programador.

Internamente el compilador se encuentra dividido en etapas independientes encargadas del análisis léxico, análisis sintáctico y generación de código.

La organización interna del compilador se documenta en:

- [Detalles del Compilador](./compilador/Compiler_Details.md)

## Hardware

La implementación hardware corresponde al procesador Milo desarrollado utilizando Logisim Evolution.

A diferencia de muchas arquitecturas tradicionales, el procesador no ejecuta directamente instrucciones del ISA. Su funcionamiento se basa en una palabra de control que gobierna el datapath mediante señales distribuidas hacia los distintos bloques funcionales.

La CPU implementa el banco de registros, la ALU, la unidad de control, el sistema de memoria y el resto de componentes necesarios para ejecutar dichas palabras de control.

Su organización interna se describe en:

- [Detalles del CPU](./CPU_Details.md)
- [Detalles de la GPU](./GPU_Details.md)

La estructura física de las instrucciones se documenta por separado mediante la especificación de codificación.

Este documento define cómo se organiza cada palabra de control dentro de la memoria ROM y el significado de cada uno de sus campos.

- [Instruction Encoding Specification](./InstructionEncoding.md)

## Niveles de abstracción

Uno de los conceptos fundamentales de Milo consiste en distinguir claramente el ISA de la codificación física de las instrucciones.

Este mecanismo permite introducir instrucciones complejas sin incrementar necesariamente la complejidad del hardware. Cuando resulta conveniente, el encoder puede expandir una instrucción del ISA en una secuencia de microinstrucciones equivalentes.

Por ejemplo, la instrucción `RET` actualmente se implementa mediante dos palabras de control consecutivas: la primera actualiza el `Stack Pointer` y la segunda carga el `Program Counter` desde la pila. Para el programador continúa existiendo una única instrucción RET, mientras que el procesador únicamente ejecuta la secuencia física generada por el compilador.

El ISA constituye una interfaz orientada al programador. Describe las instrucciones disponibles, su sintaxis y su comportamiento lógico.

La codificación de instrucciones, por el contrario, representa la forma física en la que dichas operaciones son almacenadas dentro de la memoria ROM y posteriormente interpretadas por la unidad de control del procesador.

En consecuencia:

- El programador trabaja sobre el ISA.
- El compilador traduce el ISA hacia la codificación física definida por la arquitectura.
- La CPU únicamente interpreta palabras de control binarias.

```text
Programa
    │
    ▼
Lenguaje Ensamblador (ISA)
    │
    ▼
Compilador
    │
    ▼
Instruction Encoding
    │
    ▼
   CPU
```

Aunque ambas capas se encuentran estrechamente relacionadas, no representan el mismo concepto.

Una única instrucción del ISA puede traducirse en una o varias palabras de control dependiendo de la implementación del compilador, mientras que la CPU permanece completamente ajena a dicha abstracción y únicamente ejecuta la secuencia de instrucciones codificadas almacenadas en memoria.

## Flujo de desarrollo

El procedimiento para incorporar nuevas funcionalidades depende del nivel de la arquitectura que resulte afectado.

### Modelo mental de Milo

Antes de modificar cualquier componente del proyecto es importante identificar en qué nivel de abstracción se encuentra el cambio que se desea realizar.

En Milo existen niveles claramente diferenciados:


```text
Programador
      │
      ▼
ISA
      │
      ▼
Instruction Encoding
      │
      ▼
Hardware
```

Cada nivel posee una responsabilidad distinta.

| Nivel                | Responsabilidad                                                                      |
| -------------------- | ------------------------------------------------------------------------------------ |
| Hardware             | Ejecutar señales de control.                                                         |
| Instruction Encoding | Representar dichas señales mediante palabras binarias.                               |
| ISA                  | Proporcionar una interfaz de programación independiente de la implementación física. |


Antes de realizar cualquier modificación conviene preguntarse:

- ¿Estoy cambiando el comportamiento físico del procesador?
- ¿Estoy cambiando únicamente la representación binaria?
- ¿Estoy modificando únicamente la interfaz que utiliza el programador?

Responder estas preguntas permite identificar inmediatamente qué componentes deberán actualizarse.

### Modificaciones sobre el hardware

Cuando una modificación altera el funcionamiento interno del procesador y, especialmente, la representación binaria utilizada para controlarlo, es necesario actualizar la especificación de codificación y adaptar el encoder para que continúe generando palabras de control compatibles con el hardware.

```text
Modificar Hardware
        │
        ▼
Actualizar la especificación del Instruction Encoding
        │
        ▼
Actualizar Encoder
        │
        ▼
Ejecutar Tests
```

> Los tests continúan siendo válidos porque se encuentran escritos utilizando el ISA de Milo y no la codificación binaria. Mientras el comportamiento del ISA permanezca inalterado, únicamente el encoder debe adaptarse para generar la nueva representación física de las instrucciones.

### Modificaciones sobre el ISA

Las modificaciones del lenguaje ensamblador afectan únicamente a la interfaz ofrecida al programador.

Una vez definida la nueva instrucción, el compilador decide cómo traducirla hacia una o varias palabras de control compatibles con el hardware existente.

```
Diseñar o modificar el ISA
          │
          ▼
Actualizar Lexer
          │
          ▼
Actualizar Parser
          │
          ▼
Definir su traducción al Instruction Encoding
          │
          ▼
Actualizar Encoder
          │
          ▼
Actualizar Tests
```

No existe una relación obligatoria de uno a uno entre el ISA y la codificación de instrucciones.

El ISA constituye una abstracción de programación, mientras que el Instruction Encoding representa la interfaz física utilizada por el procesador.

## Validación de la arquitectura

Toda modificación realizada sobre Milo debe validarse mediante el sistema de pruebas del proyecto.

Actualmente la arquitectura dispone de un conjunto de testbench que permiten verificar tanto las distintas etapas del compilador como la generación de la codificación binaria utilizada por el procesador.

Estas pruebas permiten comprobar que las modificaciones introducidas no alteren el comportamiento esperado del ISA, del compilador ni de la interfaz física utilizada por el hardware.

Algunas instrucciones del ISA pueden expandirse internamente en múltiples palabras de control durante la etapa de generación de código. Por este motivo, las pruebas de compilación verifican la secuencia física completa generada por el encoder y no únicamente una correspondencia uno a uno entre instrucciones del ISA y palabras almacenadas en la ROM.

En función del tipo de cambio realizado, puede ser suficiente ejecutar un subconjunto de las pruebas o resultar conveniente ejecutar la batería completa de testbench del proyecto.

La organización del sistema de pruebas, el procedimiento para compilar cada test y la descripción de las distintas categorías disponibles se documentan en:

- [Testing Specification](./tests/TestingSpecification.md)

Como regla general:

- Modificaciones del lexer requieren validar, al menos, las pruebas del lexer y del parser.
- Modificaciones del parser requieren validar el parser y las pruebas de compilación.
- Modificaciones del encoder o del Instruction Encoding requieren ejecutar las pruebas de compilación.
- Modificaciones del hardware deben verificarse utilizando las pruebas de compilación, ya que estas generan exactamente las palabras de control que posteriormente ejecutará el procesador.

## Principio de diseño

En Milo el hardware constituye la fuente de verdad (source of truth) de la arquitectura.

El Instruction Encoding documenta cómo controlar físicamente dicho hardware y el ISA proporciona una interfaz de programación construida sobre esa representación.

En consecuencia, las herramientas de software se adaptan a la arquitectura física del procesador y no al contrario.

El compilador constituye el único componente responsable de cerrar la brecha entre el nivel lógico del ISA y la implementación física del procesador. Esto permite que el hardware permanezca relativamente simple mientras que las abstracciones de programación evolucionan mediante transformaciones realizadas durante la compilación.