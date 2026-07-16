# Parser Details - Milo ASM

## Representacion Final

La representación intermedia utilizada durante esta etapa se encuentra definida por la estructura InstruccionParseada.

```c
typedef struct {
    TipoInstruccion tipo;
    uint32_t linea;

    union {
        ...
    } operandos;
} InstruccionParseada;
```

Cada instrucción válida del lenguaje produce exactamente una instancia de esta estructura, la cual contiene toda la información necesaria para la generación posterior de la palabra de control.

## Filosofía de diseño

El parser fue diseñado siguiendo una arquitectura basada en especialización por familias de instrucciones.

La función principal únicamente identifica el mnemónico correspondiente y delega el análisis sintáctico al módulo especializado encargado de esa categoría de instrucciones.

```text
parsear_linea()
        │
        ├──────────────┐
        │              │
        ▼              ▼
parsear_movimiento()  parsear_alu()
```

Por ejemplo:

- MOV, MOVI, LOAD y STORE son procesadas por `parsear_movimiento()`.
- ADD, SUB, ADC, SBC, AND, OR, XOR, NOT, SHL y SHR son procesadas por `parsear_alu()`.

Esta organización evita concentrar toda la gramática del lenguaje en una única función de gran tamaño y permite extender el ISA incorporando nuevos módulos especializados sin modificar el resto del compilador.

### Reconocimiento de instrucciones

El proceso de análisis sintáctico de cada línea sigue siempre la misma secuencia.

```text
Leer mnemónico
        │
        ▼
Identificar familia
        │
        ▼
Delegar al parser especializado
        │
        ▼
Validar operandos
        │
        ▼
Construir InstruccionParseada
```

Este flujo hace que el parser pueda incorporar nuevas familias de instrucciones con un impacto mínimo sobre el código existente.

### Recuperación ante errores

El parser implementa un mecanismo de recuperación conocido como panic mode.

Cuando se detecta un error sintáctico, el análisis de la instrucción actual se interrumpe y el parser descarta todos los tokens restantes de esa línea hasta encontrar un salto de línea o el final del archivo.

```text
Instrucción inválida
        │
        ▼
Registrar error
        │
        ▼
Descartar tokens
        │
        ▼
Continuar con la siguiente línea
```

Este mecanismo evita la propagación de errores en cascada y permite que una única compilación detecte múltiples errores sintácticos antes de finalizar.

El historial de errores conserva información como:

- Línea.
- Columna.
- Lexema encontrado.
- Tipo de token.
- Descripción del error.

Al finalizar el análisis, el parser genera un informe consolidado con todos los errores detectados durante la compilación.

### Extensibilidad

La arquitectura del parser fue diseñada para facilitar la incorporación de nuevas instrucciones del ISA.

En la mayoría de los casos, agregar una nueva familia de instrucciones únicamente requiere:

1. Incorporar el nuevo tipo en TipoInstruccion.
2. Crear un módulo especializado para dicha familia.
3. Registrar el nuevo mnemónico dentro de parsear_linea().

De esta forma cada familia permanece aislada del resto del compilador, reduciendo el acoplamiento y simplificando el mantenimiento del proyecto.
