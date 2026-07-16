# Test del Parser

Los tests del parser verifican que la secuencia de tokens generada por el lexer sea interpretada correctamente de acuerdo con la gramática definida por el ISA de Milo Alpha.

Su objetivo consiste en comprobar que el compilador construya la representación interna correcta de cada instrucción antes de iniciar la etapa de codificación.

## Aspectos verificados

Las pruebas cubren, entre otros, los siguientes escenarios:

- Reconocimiento de todas las familias de instrucciones soportadas por el ISA.
- Interpretación correcta de registros y operandos.
- Construcción de la representación interna (AST) utilizada por el encoder.
- Procesamiento de modificadores de instrucción, como `.F`.
- Validación de instrucciones con diferente cantidad de operandos.
- Detección de errores sintácticos.
- Recuperación tras errores para continuar analizando el resto del programa.
- Generación de diagnósticos con información de línea y columna.

## Estrategia de prueba

El archivo de prueba puede ejecutarse en dos modos diferentes.

- **Pruebas válidas**, orientadas a verificar que cada instrucción del ISA produzca la representación interna esperada.
- **Pruebas con errores**, destinadas a comprobar la capacidad del parser para detectar errores sintácticos y recuperarse sin detener completamente el análisis.

El modo utilizado se controla mediante la constante:

```c
#define PROBAR_ERROR_SINTACTICO true
```

Cuando su valor es `false`, se ejecuta un conjunto de instrucciones válidas.

Cuando su valor es `true`, se ejecuta un programa que contiene distintos errores sintácticos diseñados para validar los mecanismos de diagnóstico y recuperación del parser.

## Salida del test

Por cada instrucción reconocida correctamente, el test imprime la estructura interna generada por el parser.

Dependiendo del tipo de instrucción, la salida muestra los operandos correspondientes y la categoría sintáctica a la que pertenece, por ejemplo:

- Operaciones ALU binarias.
- Operaciones ALU unarias.
- Transferencias entre registros.
- Operaciones con inmediatos.
- Accesos a memoria.

Al finalizar la ejecución también se presenta un resumen del análisis sintáctico junto con el número de errores detectados, permitiendo verificar tanto la interpretación correcta del programa como la calidad de los diagnósticos emitidos por el compilador.

## Alcance

Este test valida exclusivamente la etapa de análisis sintáctico del compilador.

No verifica la codificación binaria de las instrucciones ni su representación física dentro de la ROM del procesador. Esa responsabilidad corresponde a las pruebas de compilación, las cuales utilizan la representación interna generada por el parser como entrada para el encoder.