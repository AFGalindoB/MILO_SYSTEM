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

Su responsabilidad consiste en coordinar todas las etapas del compilador, desde la inicialización del análisis léxico hasta la generación del archivo de salida.

El flujo general seguido por esta función es el siguiente.

```text
Inicializar Lexer
        │
        ▼
Parsear instrucciones
        │
        ▼
Codificar instrucciones
        │
        ▼
Almacenar PalabraROM
        │
        ▼
Emitir informe
        │
        ▼
Exportar ROM
```

Durante este proceso el compilador mantiene un buffer temporal en memoria que representa el contenido completo de la ROM antes de ser exportado al archivo de salida.

```c
PalabraROM programa_rom[256];
```

Esta estructura permite desacoplar completamente el proceso de compilación de la escritura física del archivo, facilitando futuras extensiones como simuladores, depuradores o formatos alternativos de exportación.

### Control del proceso de compilación

La función principal también centraliza la gestión del estado global de la compilación.

Entre sus responsabilidades se encuentran:

- Inicializar el lexer.
- Reiniciar el contador de errores.
- Ejecutar el ciclo principal de compilación.
- Controlar el tamaño máximo de la ROM.
- Emitir el informe final.
- Decidir si la compilación debe generar un archivo de salida.

De esta manera, el resto de módulos permanecen completamente enfocados en su responsabilidad específica sin conocer el estado global del proceso.

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