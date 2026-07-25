# Test Resolución de Etiquetas

El test **Resolución de Etiquetas** verifica el funcionamiento del sistema de símbolos implementado por el compilador de Milo ASM.

Su objetivo consiste en comprobar que el proceso de preescaneo registre correctamente todas las etiquetas del programa, que cada referencia simbólica sea sustituida por su dirección física correspondiente y que la expansión de instrucciones del ISA no altere la resolución del flujo de control.

A diferencia de versiones anteriores, donde todas las direcciones debían calcularse manualmente, este test utiliza exclusivamente etiquetas simbólicas, delegando completamente esa responsabilidad al compilador.

## Escenarios cubiertos

Actualmente la prueba verifica:

- Registro de etiquetas durante la primera pasada del compilador.
- Resolución automática de destinos de salto.
- Resolución automática de llamadas mediante `CALL`.
- Resolución automática de retornos mediante `RET`.
- Correcto cálculo de direcciones físicas considerando la expansión de instrucciones.
- Funcionamiento conjunto de `JMP`, `CALL`, `RET` y saltos condicionales.
- Integración entre la tabla de símbolos y el parser.

El programa utilizado durante la prueba es el siguiente:

```text
JMP MAIN

SUMAR:

BUCLE:
    CMP R1, R5
    JZ FIN_FN

    ADD R1, R1, R0
    JMP BUCLE

FIN_FN:
    RET

MAIN:

    MOVI R0, #1
    MOVI R1, #0
    MOVI R5, #5

    CALL SUMAR

    CMP R1, R5
    JZ IGUALES

    MOVI R2, #2
    JMP FIN

IGUALES:
    MOVI R2, #1

FIN:
    NOP
```

Durante la compilación el compilador realiza una primera pasada sobre el código fuente para registrar todas las etiquetas presentes en el programa.

Posteriormente, durante la segunda pasada, cada referencia simbólica es reemplazada automáticamente por la dirección física correspondiente antes de que la instrucción sea enviada al encoder.

Este proceso resulta especialmente importante en instrucciones como `RET`, cuya implementación física ocupa dos palabras de control consecutivas. Gracias al preescaneo, las direcciones almacenadas en la tabla de símbolos corresponden siempre a posiciones físicas reales dentro de la ROM.

## Resultado esperado

La compilación debe finalizar sin errores y generar el archivo:

`compilaciones/instrucciones/test_etiquetas.txt`

El archivo generado puede cargarse posteriormente en la ROM del circuito CPU.circ para verificar el comportamiento físico del procesador.

Durante la ejecución debe observarse que:

- El salto inicial transfiere correctamente el control a `MAIN`.
- La instrucción `CALL` localiza correctamente la dirección de la etiqueta `SUMAR`.
- El bucle interno utiliza la etiqueta `BUCLE` como destino de los saltos.
- La condición de salida dirige la ejecución hacia `FIN_FN`.
- La instrucción `RET` retorna correctamente al programa principal.
- Las etiquetas `IGUALES` y `FIN` son resueltas correctamente por el compilador.
- El programa finaliza ejecutando la instrucción `NOP`.

El Program Counter debe recorrer exactamente el mismo flujo que en la versión con direcciones absolutas, demostrando que la resolución automática de etiquetas genera una codificación física equivalente.

## Equivalencia con la versión de direcciones absolutas

Este test corresponde funcionalmente al **Test Program Counter**.

La diferencia es que las direcciones físicas ya no son calculadas manualmente por el programador. Por ejemplo: `CALL SUMAR` es transformado internamente por el compilador en una llamada a la dirección física correspondiente a la etiqueta `SUMAR`.

Esta equivalencia demuestra que la incorporación del sistema de etiquetas no modifica el comportamiento del programa generado, sino únicamente la forma en que el código fuente es escrito, aumentando considerablemente su legibilidad y facilitando el mantenimiento de programas de mayor tamaño.