# Test FLAGS

El test FLAGS verifica la generación de código para las instrucciones que actualizan el registro de banderas (Status Register) del procesador.

Su objetivo principal consiste en comprobar que el compilador genere correctamente las señales de control asociadas a la actualización de las banderas y que el hardware produzca el estado esperado tras ejecutar operaciones aritméticas específicas sobre la arquitectura actual de 24 bits.

## Escenarios cubiertos

Actualmente la prueba verifica:

- Actualización de la bandera **Carry (C)**.
- Propagación del acarreo mediante `ADC`.
- Actualización de la bandera **Zero (Z)`.
- Actualización de la bandera **Negative (N)`.
- Uso del acarreo en operaciones `SBC`.
- Actualización de la bandera **Overflow (V)`.
- Funcionamiento del modificador `.F`, encargado de habilitar la actualización del registro de banderas.

El programa utilizado durante la prueba es el siguiente:

```text
; --- 1. PROBAR CARRY (C) ---
MOVI R0, #16777215       ; Máximo valor sin signo de 24 bits (0xFFFFFF)
MOVI R1, #1
ADD.F R2, R0, R1         ; 0xFFFFFF + 1 = 0x000000 (Carry = 1)

; --- 2. PROBAR PROPAGACIÓN DE CARRY (ADC) ---
ADC R4, R3, R3           ; 0 + 0 + Carry = 1

; --- 3. PROBAR ZERO (Z) ---
MOVI R5, #5
SUB.F R6, R5, R5         ; 5 - 5 = 0 (Zero = 1)

; --- 4. PROBAR NEGATIVE (N) ---
MOVI R7, #2
SUB.F R8, R7, R5         ; 2 - 5 = -3 (0xFFFFFD) (Negative = 1)

; --- 5. PROBAR SBC ---
SBC R9, R3, R3           ; Resta utilizando el estado actual del Carry

; --- 6. PROBAR OVERFLOW (V) ---
MOVI R10, #8388607       ; Máximo entero positivo con signo en 24 bits (0x7FFFFF)
ADD.F R11, R10, R1       ; 0x7FFFFF + 1 = 0x800000 (Overflow = 1)
```

Cada bloque del programa está diseñado para provocar una condición específica sobre el registro de banderas del procesador y verificar que el compilador genere correctamente las señales de control necesarias para su actualización.

## Resultado esperado

La compilación debe finalizar sin errores y generar el archivo:

`compilaciones/instrucciones/test_flags.txt`

Posteriormente, el archivo puede cargarse en la ROM del circuito `CPU.circ` para verificar el comportamiento físico del procesador.

Al finalizar la ejecución completa del programa, el banco de registros debe presentar el siguiente estado:

![Test Flags Regs](../images/test_flags_1.png)

Durante la ejecución de la última instrucción (`ADD.F R11, R10, R1`), la bandera **Overflow (V)** debe encontrarse activa, indicando que se produjo un desbordamiento aritmético en una suma con signo de 24 bits.

![Test Flags Overflow](../images/test_flags_2.png)