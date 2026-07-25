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

Su responsabilidad consiste en coordinar todas las etapas necesarias para transformar un programa escrito en Milo ASM en una imagen ROM ejecutable por el procesador.

A diferencia de versiones anteriores, el compilador implementa una arquitectura de doble pasada, permitiendo resolver referencias simbólicas mediante una tabla de símbolos antes de iniciar la generación de código.

El flujo general es el siguiente.

```text
Primera pasada
──────────────

Inicializar Lexer
        │
        ▼
Pre-escaneo
de etiquetas
        │
        ▼
Construir
Tabla de Símbolos

────────────────────────────────

Segunda pasada

Reiniciar Lexer
        │
        ▼
Parser
        │
        ▼
Encoder
        │
        ▼
Generación de
Palabras ROM
        │
        ▼
Construcción
del programa
        │
        ▼
Emitir informe
        │
        ▼
Exportar ROM
```

### Primera pasada

Durante la primera pasada no se genera código máquina.

El compilador recorre completamente el programa con el único objetivo de construir la tabla de símbolos.

Para cada línea se realizan dos tareas principales:

- Detectar definiciones de etiquetas.
- Calcular la dirección física que ocupará cada instrucción dentro de la ROM.

Esta segunda tarea resulta necesaria debido a que una instrucción del ISA puede expandirse en múltiples palabras ROM. Por ejemplo:

```asm
RET
```

Ocupa dos posiciones físicas en memoria.

Por este motivo, las direcciones almacenadas en la tabla de símbolos corresponden siempre a direcciones físicas de la ROM y no simplemente al número de línea del programa.

**Tabla de símbolos:**

Las etiquetas detectadas durante el pre-escaneo se almacenan en una tabla de símbolos.

```c
typedef struct {
    char nombre[64];
    uint32_t direccion;
} Etiqueta;
```

Todas las etiquetas registradas forman parte de: `TablaSimbolos`

Cada entrada almacena:

- Nombre simbólico.
- Dirección física dentro de la ROM.

Durante esta etapa también se detectan errores como:

- Redefinición de etiquetas.
- Exceso del número máximo permitido.

Durante este proceso el compilador mantiene un buffer en memoria que representa el contenido completo de la ROM antes de ser exportado.

### Segunda pasada

Una vez construida la tabla de símbolos, el compilador reinicia el lexer y comienza la generación del programa.

Cada línea sigue el siguiente recorrido:

```text
Lexer
   │
   ▼
Parser
   │
   ▼
Encoder
   │
   ▼
Programa ROM
```

Cuando el parser encuentra una referencia simbólica:

```asm
JMP LOOP
```

Consulta la tabla de símbolos y reemplaza automáticamente la etiqueta por la dirección física correspondiente.

De esta forma el encoder únicamente recibe operandos completamente resueltos.

### Construcción del programa

Durante la segunda pasada el compilador mantiene un buffer que representa el contenido completo de la memoria ROM.

```c
PalabraROM programa_rom[256];
```

La generación de código utiliza además un buffer temporal para recibir las palabras producidas por el encoder.

```c
PalabraROM buffer_temporal[4];
```

Cada instrucción del ISA puede producir una o varias palabras ROM.

Posteriormente dichas palabras son copiadas secuencialmente al programa final respetando el orden de ejecución.

### Expansión de instrucciones

El compilador no asume una correspondencia uno a uno entre una instrucción del ISA y una palabra ROM.

Por ejemplo `RET` es expandida internamente como:

1. Decrementar el Stack Pointer.
2. Cargar el Program Counter desde el Stack.

Desde el punto de vista del programador continúa existiendo una única instrucción, mientras que el hardware ejecuta la secuencia física necesaria para implementarla.

Esta separación mantiene desacoplado el ISA de la implementación física del procesador.

### Control del proceso de compilación

La función `compilar_y_exportar()` centraliza el estado global del compilador.

Entre sus responsabilidades se encuentran:

- Ejecutar la primera pasada.
- Construir la tabla de símbolos.
- Reiniciar el lexer para la segunda pasada.
- Inicializar el contador global de errores.
- Invocar al parser.
- Solicitar al encoder la generación del microcódigo.
- Gestionar la expansión de instrucciones.
- Controlar el tamaño máximo de la ROM.
- Construir la imagen final del programa.
- Emitir el informe de compilación.
- Decidir si el archivo ROM debe exportarse.

De esta manera, el resto de módulos permanecen completamente enfocados en su responsabilidad específica sin conocer el estado global del proceso.

### Independencia entre ISA y hardware

La arquitectura mantiene completamente desacoplados tres niveles distintos.

```text
Código ASM
      │
      ▼
Parser
      │
      ▼
Representación Intermedia (IR)
      │
      ▼
Encoder
      │
      ▼
Palabras ROM
```

El compilador actúa como orquestador entre estas etapas, sin depender de los detalles internos de cada una de ellas.

Gracias a esta separación:

- El parser no necesita conocer la codificación física.
- El encoder no necesita interpretar texto ensamblador.
- El compilador puede incorporar nuevas etapas, como la resolución de etiquetas, sin modificar la interfaz entre parser y encoder.

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
              Compilador
                  │
      ┌───────────┴───────────┐
      │                       │
Primera pasada           Segunda pasada
      │                       │
    Lexer                   Lexer
      │                       │
Tabla de símbolos           Parser
                              │
                           Encoder
                              │
                          Escritor ROM
```

Ninguna de las etapas del pipeline conoce cómo se ejecuta el proceso completo ni cómo será almacenado el resultado final.

Del mismo modo, el orquestador desconoce los detalles internos del análisis léxico, sintáctico o de la codificación.

Esta separación permite modificar cualquiera de las etapas del compilador sin afectar al resto del sistema, manteniendo una arquitectura modular y de bajo acoplamiento.