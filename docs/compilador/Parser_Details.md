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

La función principal identifica el mnemónico correspondiente y delega el análisis sintáctico al módulo especializado encargado de esa categoría de instrucciones.

```text
parsear_linea()
        │
        ├──────────────┬───────────────┐
        │              │               │
        ▼              ▼               ▼
parsear_movimiento() parsear_alu() parsear_pc()
```

Actualmente las familias implementadas son:

- **Movimiento de datos:** `MOV`, `MOVI`, `LOAD`, `STORE`.
- **Operaciones ALU:** `ADD`, `ADC`, `SUB`, `SBC`, `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`.
- **Control de flujo:** `CMP`, `JMP`, `CALL`, `RET`, `JZ`, `JNZ`, `JC`, `JNC`, `JN`, `JNN`, `JV` y `JNV`.

Esta organización evita concentrar toda la gramática del lenguaje en una única función de gran tamaño y permite extender el ISA incorporando nuevos módulos especializados sin modificar el resto del compilador.

### Reconocimiento de instrucciones

El proceso de análisis sintáctico de cada línea sigue siempre la misma secuencia.

```text
Leer mnemónico
        │
        ▼
Seleccionar familia
        │
        ▼
Parser especializado
        │
        ▼
Validar sintaxis
        │
        ▼
Construir InstruccionParseada
        │
        ▼
Registrar errores (si existen)
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

Además de evitar errores en cascada, el parser conserva información suficiente para generar un diagnóstico detallado al finalizar la compilación.

Cada error registrado almacena:

- Línea.
- Columna.
- Lexema encontrado.
- Tipo del token.
- Descripción del error.

Utilizando esta información, el informe final reproduce la línea del código fuente donde ocurrió el problema e indica visualmente la posición aproximada mediante un marcador (`^`).

Por ejemplo:

```text
[1] Error sintáctico en Línea 8, Columna 12

Detalle:
Se esperaba una coma.

     8 | ADD R1 R2 R3
                ^
```

Este formato facilita localizar rápidamente el origen del error sin necesidad de inspeccionar manualmente todo el archivo fuente.

### Extensibilidad

En la mayoría de los casos, incorporar una nueva familia de instrucciones requiere únicamente:

1. Añadir el nuevo tipo a `TipoInstruccion`.
2. Incorporar la estructura correspondiente dentro de `InstruccionParseada`, cuando sea necesario.
3. Implementar un parser especializado para esa familia.
4. Registrar el nuevo mnemónico en `parsear_linea()`.
5. Implementar la traducción correspondiente en el encoder.

Gracias a esta organización, el parser permanece desacoplado del hardware y únicamente describe la estructura sintáctica del ISA.