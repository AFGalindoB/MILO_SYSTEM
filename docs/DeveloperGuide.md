# Developer Guide - MILO SYSTEM

Versión: 2.0.0

## Introducción

Milo System es un proyecto de investigación cuyo objetivo es desarrollar un ecosistema completo alrededor de una arquitectura de procesador propia.

A diferencia de muchas arquitecturas tradicionales, el desarrollo de Milo parte de la implementación física del procesador. El datapath, las señales de control y la organización interna del hardware constituyen la base sobre la que posteriormente se construyen las herramientas de programación.

En consecuencia, el conjunto de instrucciones (ISA) no define el funcionamiento interno del procesador, sino que actúa como una capa de abstracción destinada a facilitar su programación. La CPU ejecuta directamente palabras de control, mientras que el ensamblador traduce las instrucciones del ISA hacia dicha representación física.

El proyecto no se limita únicamente al diseño del hardware. Cada nueva capacidad incorporada al procesador requiere también el desarrollo de las herramientas de software necesarias para programarlo, validarlo y facilitar su evolución.

Por este motivo, Milo evoluciona mediante dos líneas de trabajo estrechamente relacionadas: la implementación del hardware y el desarrollo de su ecosistema de software.

Este documento presenta una visión general del proyecto y sirve como punto de partida para comprender cómo se relacionan sus distintos componentes antes de profundizar en la documentación técnica de cada uno de ellos.

---

## Arquitectura del proyecto

El desarrollo de Milo se organiza actualmente en dos componentes principales que evolucionan de forma coordinada:

* **Hardware**, responsable de implementar físicamente el procesador y los componentes que forman una implementación concreta de Milo.
* **Software**, responsable de proporcionar las herramientas necesarias para programar, compilar y validar dichas implementaciones.

```text
                         Milo System
                              │
                ┌─────────────┴─────────────┐
                │                           │
             Hardware                    Software
                │                           │
        ┌───────┴───────┐                   ▼
        │               │               MILO ASM
      Core           Console
```

Aunque ambos componentes evolucionan conjuntamente, cumplen responsabilidades claramente diferenciadas.

El hardware define las capacidades reales del procesador, su organización física y las señales que pueden ejecutarse.

El software construye las abstracciones necesarias para utilizar dichas capacidades, proporcionando un lenguaje ensamblador (ISA), un compilador y herramientas de desarrollo que ocultan los detalles de la codificación binaria.

Esta separación permite que el ISA evolucione independientemente de la representación física de las instrucciones y facilita que una misma operación visible para el programador pueda implementarse mediante distintas estrategias de codificación sin modificar la interfaz expuesta al usuario.

### Implementaciones de Milo

Milo se plantea como una arquitectura que puede materializarse mediante diferentes implementaciones dependiendo del propósito del sistema.

Actualmente se contemplan dos perfiles:

* **Milo Core**: implementación orientada a una integración simple y de menor complejidad. Está destinada a proporcionar el núcleo de procesamiento y los recursos fundamentales necesarios para construir sistemas embebidos o de control.
* **Milo Console**: implementación orientada a sistemas de entretenimiento y gráficos, incorporando los periféricos necesarios para este propósito.

Ambas implementaciones utilizan la misma arquitectura Milo y comparten sus principios fundamentales. La diferencia se encuentra principalmente en los recursos y periféricos integrados alrededor del CPU.

Por tanto, los periféricos no forman necesariamente parte del núcleo del procesador. Su incorporación depende de la implementación concreta y de las necesidades del sistema.

La definición detallada de cada periférico y de las interfaces utilizadas para integrarlo se encuentra documentada por separado.

---

## Software

La capa de software proporciona las herramientas necesarias para programar la arquitectura Milo.

Actualmente esta capa está compuesta por el compilador **MILO ASM**, cuya función consiste en traducir programas escritos utilizando el ISA de Milo hacia la representación binaria que comprende el procesador.

Durante este proceso el compilador interpreta el lenguaje ensamblador, valida su sintaxis, resuelve etiquetas y finalmente genera la codificación física que será almacenada en la memoria ROM del procesador.

Dependiendo de la instrucción, una operación del ISA puede traducirse en una o varias palabras de control físicas. Esto permite que el compilador implemente determinadas abstracciones utilizando múltiples palabras de control sin modificar la interfaz ofrecida al programador.

Internamente el compilador se encuentra dividido en etapas independientes encargadas del análisis léxico, análisis sintáctico y generación de código.

La organización interna del compilador se documenta en:

* [Detalles del Compilador](./compilador/Compiler_Details.md)

---

## Hardware

La implementación hardware corresponde al procesador Milo y a los componentes que lo acompañan en una implementación concreta.

El desarrollo actual utiliza **Logisim Evolution** como herramienta principal para la construcción y validación del hardware.

A diferencia de muchas arquitecturas tradicionales, el procesador no ejecuta directamente instrucciones del ISA. Su funcionamiento se basa en palabras de control que gobiernan el datapath mediante señales distribuidas hacia los distintos bloques funcionales.

La CPU implementa el banco de registros, la ALU, la unidad de control, el sistema de memoria y el resto de componentes necesarios para ejecutar dichas palabras de control.

La descripción de la organización física del CPU se encuentra en:

* [Detalles del CPU](./CPU_Details.md)

Los componentes gráficos y periféricos asociados a implementaciones específicas se documentan de manera independiente.

Actualmente se encuentra definida una implementación orientada a consola:

* [Detalles de la GPU](./GPU_Details.md)

La estructura física de las instrucciones se documenta por separado mediante la especificación de codificación.

Este documento define cómo se organiza cada palabra de control dentro de la memoria ROM y el significado de cada uno de sus campos.

* [Instruction Encoding Specification](./InstructionEncoding.md)

---

## Niveles de abstracción

Uno de los conceptos fundamentales de Milo consiste en distinguir claramente el ISA de la codificación física de las instrucciones.

Este mecanismo permite introducir instrucciones complejas sin incrementar necesariamente la complejidad del hardware. Cuando resulta conveniente, el compilador puede expandir una instrucción del ISA en una secuencia de palabras de control equivalentes.

Por ejemplo, la instrucción `RET` actualmente se implementa mediante dos palabras de control consecutivas: la primera actualiza el `Stack Pointer` y la segunda carga el `Program Counter` desde la pila.

Para el programador continúa existiendo una única instrucción `RET`, mientras que el procesador ejecuta la secuencia física generada por el compilador.

El ISA constituye una interfaz orientada al programador. Describe las instrucciones disponibles, su sintaxis y su comportamiento lógico.

La codificación de instrucciones, por el contrario, representa la forma física en la que dichas operaciones son almacenadas dentro de la memoria ROM y posteriormente interpretadas por la unidad de control del procesador.

En consecuencia:

* El programador trabaja sobre el **ISA**.
* El compilador traduce el **ISA** hacia la codificación física.
* La **CPU** interpreta y ejecuta las palabras de control binarias.

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

Una única instrucción del ISA puede traducirse en una o varias palabras de control dependiendo de la implementación del compilador, mientras que la CPU permanece completamente ajena a dicha abstracción y únicamente ejecuta la secuencia de palabras de control almacenadas en memoria.

---

## Flujo de desarrollo

El procedimiento para incorporar nuevas funcionalidades depende del nivel de la arquitectura que resulte afectado.

### Modelo mental de Milo

Antes de modificar cualquier componente del proyecto es importante identificar en qué nivel de abstracción se encuentra el cambio que se desea realizar.

En Milo existen los siguientes niveles principales:

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

Cada nivel posee una responsabilidad distinta:

| Nivel                | Responsabilidad                                                                      |
| -------------------- | ------------------------------------------------------------------------------------ |
| Hardware             | Ejecutar físicamente las señales de control.                                         |
| Instruction Encoding | Representar dichas señales mediante palabras binarias.                               |
| ISA                  | Proporcionar una interfaz de programación independiente de la implementación física. |

Los periféricos se encuentran asociados al nivel hardware y pueden variar entre las distintas implementaciones de Milo.

Antes de realizar cualquier modificación conviene preguntarse:

* ¿Estoy cambiando el comportamiento físico del procesador?
* ¿Estoy incorporando o modificando un componente hardware?
* ¿Estoy cambiando únicamente la representación binaria?
* ¿Estoy modificando únicamente la interfaz que utiliza el programador?

Responder estas preguntas permite identificar inmediatamente qué componentes deberán actualizarse.

---

### Modificaciones sobre el hardware

Cuando una modificación altera el funcionamiento interno del procesador o la representación física utilizada para controlarlo, es necesario actualizar la especificación correspondiente y adaptar las herramientas de generación de código cuando sea necesario.

```text
Modificar Hardware
        │
        ▼
Actualizar Hardware Details
        │
        ▼
Actualizar Instruction Encoding
        │
        ▼
Actualizar Encoder
        │
        ▼
Ejecutar Tests
```

No todas las modificaciones hardware requieren modificar el ISA.

Por ejemplo, un cambio en la organización interna del datapath puede modificar la codificación física o las señales de control sin cambiar el comportamiento observable de una instrucción.

De la misma forma, la incorporación de un periférico puede pertenecer únicamente a una implementación concreta de Milo sin modificar la arquitectura base del CPU.

Los tests escritos utilizando el ISA permiten comprobar que el comportamiento lógico expuesto al programador permanece compatible, mientras que las pruebas específicas de hardware permiten verificar el funcionamiento físico de los nuevos componentes.

---

### Modificaciones sobre el ISA

Las modificaciones del lenguaje ensamblador afectan a la interfaz ofrecida al programador.

Una vez definida la nueva instrucción, el compilador determina cómo traducirla hacia una o varias palabras de control compatibles con el hardware existente.

```text
Diseñar o modificar el ISA
          │
          ▼
     Actualizar Lexer
          │
          ▼
     Actualizar Parser
          │
          ▼
Definir traducción al
Instruction Encoding
          │
          ▼
     Actualizar Encoder
          │
          ▼
     Actualizar Tests
```

No existe una relación obligatoria de uno a uno entre el ISA y la codificación de instrucciones.

El ISA constituye una abstracción de programación, mientras que el Instruction Encoding representa la interfaz física utilizada por el procesador.

---

### Modificaciones sobre periféricos

Los periféricos forman parte de la implementación hardware y no necesariamente del núcleo arquitectónico de Milo.

Una modificación sobre un periférico debe analizarse según el nivel en el que afecte al sistema.

Si el cambio únicamente modifica su implementación física, puede limitarse al hardware correspondiente.

Si introduce una nueva interfaz accesible desde el software, será necesario actualizar también la documentación y las herramientas que permitan utilizar dicha interfaz.

```text
Modificar periférico
        │
        ▼
Determinar si cambia
su interfaz de software
        │
   ┌────┴────┐
   │         │
  No        Sí
   │         │
   ▼         ▼
Hardware   ISA / Software
```

Esta separación permite que distintas implementaciones de Milo incorporen conjuntos diferentes de periféricos sin convertirlos en requisitos obligatorios del CPU.

---

## Validación de la arquitectura

Toda modificación realizada sobre Milo debe validarse mediante el sistema de pruebas correspondiente.

Actualmente la arquitectura dispone de un conjunto de testbench que permiten verificar tanto las distintas etapas del compilador como la generación de la codificación binaria utilizada por el procesador.

Estas pruebas permiten comprobar que las modificaciones introducidas no alteren el comportamiento esperado del ISA, del compilador ni de la interfaz física utilizada por el hardware.

Algunas instrucciones del ISA pueden expandirse internamente en múltiples palabras de control durante la etapa de generación de código. Por este motivo, las pruebas de compilación verifican la secuencia física completa generada por el encoder y no únicamente una correspondencia uno a uno entre instrucciones del ISA y palabras almacenadas en la ROM.

En función del tipo de cambio realizado, puede ser suficiente ejecutar un subconjunto de las pruebas o resultar conveniente ejecutar la batería completa de testbench del proyecto.

Las modificaciones hardware deben validarse además mediante las pruebas correspondientes al circuito o componente afectado.

La organización del sistema de pruebas, el procedimiento para compilar cada test y la descripción de las distintas categorías disponibles se documentan en:

* [Testing Specification](./tests/TestingSpecification.md)

Como regla general:

* Modificaciones del **lexer** requieren validar, al menos, las pruebas del lexer y del parser.
* Modificaciones del **parser** requieren validar el parser y las pruebas de compilación.
* Modificaciones del **encoder** o del **Instruction Encoding** requieren ejecutar las pruebas de compilación.
* Modificaciones del **ISA** requieren validar el comportamiento de las instrucciones afectadas y su traducción física.
* Modificaciones del **hardware** requieren validar el circuito afectado y, cuando corresponda, comprobar que las pruebas del software continúan generando una secuencia compatible.
* Modificaciones de **periféricos** requieren validar tanto su comportamiento físico como cualquier interfaz de software que hayan introducido o modificado.

---

## Principio de diseño

En Milo el hardware constituye la fuente de verdad (*source of truth*) de la arquitectura.

El Instruction Encoding documenta cómo controlar físicamente dicho hardware y el ISA proporciona una interfaz de programación construida sobre esa representación.

En consecuencia, las herramientas de software se adaptan a la arquitectura física del procesador y no al contrario.

El compilador constituye el componente responsable de cerrar la brecha entre el nivel lógico del ISA y la implementación física del procesador. Esto permite que el hardware permanezca relativamente simple mientras que las abstracciones de programación evolucionan mediante transformaciones realizadas durante la compilación.

Al mismo tiempo, Milo mantiene separadas las capacidades fundamentales del CPU de los recursos específicos de cada implementación.

De esta manera, **Milo Core** puede proporcionar una implementación sencilla del procesador, mientras que **Milo Console** puede incorporar periféricos adicionales orientados a gráficos, entrada y entretenimiento sin convertir dichos componentes en requisitos del núcleo de la arquitectura.

Esta separación permite que Milo evolucione como una arquitectura común sobre la cual pueden construirse diferentes sistemas, manteniendo una interfaz de programación coherente y una implementación hardware adaptable a las necesidades de cada producto.
