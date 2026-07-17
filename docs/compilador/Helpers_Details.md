# Helpers Details - Milo ASM

Actualmente el módulo se encuentra dividido en dos componentes principales.

```text
Helpers
    │
    ├──────────────┐
    ▼              ▼
Compilador     Escritor ROM
```

## Compilador

La función `compilar_y_exportar()` constituye el punto de entrada del proceso completo de compilación.

Su responsabilidad consiste en coordinar todas las etapas del compilador, desde la inicialización del análisis léxico hasta la generación del archivo de salida compatible con la memoria ROM del procesador.

El flujo general seguido por esta función es el siguiente.

```text
Inicializar Lexer
        │
        ▼
Parsear instrucción
        │
        ▼
Codificar instrucción
        │
        ▼
Generar una o varias
PalabrasROM
        │
        ▼
Almacenar en la ROM
        │
        ▼
Emitir informe
        │
        ▼
Exportar ROM
```

Durante este proceso el compilador mantiene un buffer en memoria que representa el contenido completo de la ROM antes de ser exportado.

```c
PalabraROM programa_rom[256];
```

La generación de código utiliza además un búfer temporal para recibir las palabras de control producidas por el encoder.

```c
PalabraROM buffer_temporal[4];
```

Este diseño permite que una única instrucción del ISA pueda expandirse en varias palabras de control sin modificar el resto del pipeline de compilación.

Posteriormente, cada palabra generada es copiada secuencialmente al programa final respetando el orden de ejecución.

### Expansión de instrucciones

Aunque la mayoría de instrucciones del ISA producen una única palabra de control, el compilador admite instrucciones cuya implementación física requiere varias.

Por ejemplo, la instrucción `RET` se traduce internamente en dos palabras de control consecutivas:

1. Decrementar el Stack Pointer.
2. Cargar el Program Counter desde el Stack.

Desde el punto de vista del programador continúa existiendo una única instrucción `RET`, mientras que el procesador ejecuta la secuencia física necesaria para implementar dicho comportamiento.

Esta estrategia permite que el ISA permanezca independiente de la complejidad de la implementación hardware.

### Control del proceso de compilación

La función principal también centraliza la gestión del estado global de la compilación.

Entre sus responsabilidades se encuentran:

- Inicializar el lexer.
- Reiniciar el contador global de errores.
- Ejecutar el ciclo principal de compilación.
- Invocar al parser para interpretar cada instrucción.
- Solicitar al encoder la generación de una o varias palabras de control.
- Gestionar la expansión de instrucciones del ISA.
- Controlar el tamaño máximo de la ROM física.
- Construir la imagen final del programa.
- Emitir el informe de compilación.
- Decidir si debe exportarse el archivo de salida.

De esta manera, el resto de módulos permanecen completamente enfocados en su responsabilidad específica sin conocer el estado global del proceso.

### Independencia entre ISA y hardware

Una característica importante de esta arquitectura es que el compilador no asume una correspondencia uno a uno entre el ISA y la representación física.

```text
      ISA
       │
       ▼
   Compilador
       │
       ├──────────────┐
       ▼              ▼
1 Palabra ROM   Varias Palabras ROM
       │              │
       └──────┬───────┘
              ▼
          Programa Final
```

Gracias a esta separación, el ISA puede ofrecer instrucciones de alto nivel cuya implementación física requiera varias palabras de control consecutivas. Esto permite simplificar la programación sin aumentar la complejidad visible para el usuario y facilita futuras optimizaciones o expansiones de la arquitectura sin modificar la sintaxis del lenguaje ensamblador.

## Escritor ROM

El módulo `escritor_rom` constituye la última etapa del compilador.

Su responsabilidad consiste en transformar el contenido del buffer de memoria en un archivo compatible con la memoria ROM utilizada por el procesador.

Cada elemento del arreglo `PalabraROM` es escrito siguiendo exactamente el formato definido en **Instruction Encoding Specification**.

```text
Control Word (32 bits)
Immediate    (32 bits)
        │
        ▼
XXXXXXXXYYYYYYYY
```

Cada línea del archivo representa una única posición de la memoria de programa. Ej:

```text
0800060100000005
0840060100000001
0484000800000000
```

Este formato facilita la carga directa del programa dentro del simulador del procesador sin requerir etapas adicionales de conversión.

### Filosofía de diseño

El módulo Helpers fue diseñado para mantener completamente desacopladas las distintas responsabilidades del compilador.

```text
Lexer
    │
Parser
    │
Codificador
    │
──────────────
Helpers
    │
Escritor ROM
```

Ninguna de las etapas del pipeline conoce cómo se ejecuta el proceso completo ni cómo será almacenado el resultado final.

Del mismo modo, el orquestador desconoce los detalles internos del análisis léxico, sintáctico o de la codificación.

Esta separación permite modificar cualquiera de las etapas del compilador sin afectar al resto del sistema, manteniendo una arquitectura modular y de bajo acoplamiento.