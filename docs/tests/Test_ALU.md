# Test ALU

El test ALU verifica la generación de código para las instrucciones aritméticas y lógicas implementadas por la Unidad Aritmético-Lógica (ALU) de Milo Alpha.

Su objetivo es comprobar que el compilador traduzca correctamente las instrucciones del ensamblador a palabras de control, seleccionando la operación de la ALU, los operandos, el destino del resultado y las señales de control correspondientes.

La prueba también verifica las distintas combinaciones de operandos admitidas por la sintaxis de Milo, incluyendo registros, valores inmediatos y accesos a RAM.

## Escenarios cubiertos

Actualmente, la prueba contempla los siguientes escenarios:

- **Operaciones aritméticas:** suma (`ADD`, `ADC`) y resta (`SUB`, `SBC`).
- **Operaciones lógicas:** AND (`AND`), OR (`OR`), XOR (`XOR`) y complemento (`NOT`).
- **Desplazamientos:** desplazamiento a la izquierda (`SHL`) y a la derecha (`SHR`).
- **Operandos en registros:** operaciones entre dos registros de propósito general.
- **Operandos inmediatos:** operaciones con un registro y un inmediato, así como operaciones entre dos inmediatos.
- **Acceso a RAM como fuente:** operaciones que utilizan datos almacenados en memoria, en diferentes posiciones de los operandos.
- **Destino en RAM:** operaciones cuyo resultado se escribe directamente en memoria, sin requerir una instrucción adicional para transferirlo desde un registro.
- **Comparación (`CMP`):** ejecución de una resta destinada a actualizar las flags sin almacenar el resultado numérico.
- **Restricciones de codificación:** uso del único campo Immediate/Payload de 24 bits, incluyendo la coincidencia de los valores inmediatos cuando una misma instrucción los referencia varias veces.

## Programa de prueba

El programa inicializa los registros base y ejecuta las operaciones anteriores. Los comentarios indican los resultados esperados en hexadecimal, considerando los operandos, el ancho de datos de 24 bits y el estado de las flags cuando corresponde.

Los resultados se almacenan en posiciones conocidas de RAM para facilitar su inspección posterior. Las operaciones con destino directo en RAM escriben sus resultados en las direcciones especificadas por la instrucción. Algunas de estas operaciones utilizan el contenido previo de la propia dirección como operando fuente.

El programa completo se encuentra en el archivo de prueba `test_alu.milo`.

## Resultado esperado

La compilación debe finalizar sin errores y generar el archivo:

`compilaciones/instrucciones/test_alu.txt`

Cada instrucción debe producir una palabra de control cuya codificación corresponda con la especificación del ISA y del Instruction Encoding de Milo Alpha. Esto incluye la selección de la operación de la ALU, los registros fuente y destino, los selectores de bus, la dirección de memoria y las señales de habilitación pertinentes.

La verificación se divide en dos etapas:

1. **Validación del compilador:** comprobar que el programa se analiza y codifica correctamente y que se genera el archivo de instrucciones esperado.
2. **Validación del hardware:** cargar el programa compilado en la ROM del circuito `CPU.circ`, ejecutarlo y contrastar los resultados obtenidos con los valores esperados.

Los resultados de la ejecución pueden visualizarse directamente en la RAM del circuito. Las posiciones de memoria contienen los resultados en el siguiente orden:

### Resultados de operaciones con registros

| Dirección RAM | Operación         | Resultado esperado |
| ------------- | ----------------- | ------------------ |
| `0x00`        | `ADD R2, R0, R1`  | `0x00000D`         |
| `0x01`        | `ADC R3, R1, R0`  | `0x00000D`         |
| `0x02`        | `SUB R4, R0, R1`  | `0x000007`         |
| `0x03`        | `SBC R5, R1, R0`  | `0xFFFFF9`         |
| `0x04`        | `AND R6, R1, R0`  | `0x000002`         |
| `0x05`        | `OR R7, R0, R1`   | `0x00000B`         |
| `0x06`        | `XOR R8, R1, R0`  | `0x000009`         |
| `0x07`        | `SHL R9, R1, R0`  | `0x000C00`         |
| `0x08`        | `SHR R10, R0, R1` | `0x000001`         |
| `0x09`        | `NOT R11, R1`     | `0xFFFFFC`         |

### Resultados con operandos inmediatos

| Dirección RAM | Operación               | Resultado esperado |
| ------------- | ----------------------- | ------------------ |
| `0x0A`        | `ADD R12, #0x0A, R1`    | `0x00000D`         |
| `0x0B`        | `ADD R13, R1, #0x03`    | `0x000006`         |
| `0x0C`        | `ADD R14, #0x05, #0x05` | `0x00000A`         |

### Resultados con RAM como operando fuente

| Dirección RAM | Operación              | Resultado esperado |
| ------------- | ---------------------- | ------------------ |
| `0x0D`        | `ADD R15, R1, [R0]`    | `0x000010`         |
| `0x0E`        | `ADD R15, [R0], R1`    | `0x000010`         |
| `0x0F`        | `ADD R15, [R0], RAM`   | `0x00001A`         |
| `0x10`        | `ADD R15, #0x03, [R0]` | `0x000010`         |
| `0x11`        | `ADD R15, [R0], #0x03` | `0x000010`         |

### Resultados de operaciones con destino directo en RAM

| Dirección RAM | Operación                   | Resultado esperado |
| ------------- | --------------------------- | ------------------ |
| `0x12`        | `ADD [#0x12], R1, R2`       | `0x000010`         |
| `0x13`        | `ADD [R2], #0x03, R1`       | `0x000006`         |
| `0x14`        | `ADD [R2], R1, #0x03`       | `0x000006`         |
| `0x15`        | `ADD [#0x15], #0x15, #0x15` | `0x00002A`         |
| `0x16`        | `ADD [R2], RAM, R1`         | `0x000003`         |
| `0x17`        | `ADD [R2], R1, RAM`         | `0x000003`         |
| `0x18`        | `ADD [#0x18], #0x18, RAM`   | `0x000018`         |
| `0x19`        | `ADD [#0x19], RAM, #0x19`   | `0x000019`         |
| `0x1A`        | `ADD [R2], RAM, RAM`        | `0x000000`         |

### Comparación

La instrucción final ejecuta `CMP [R0], RAM`, que compara el contenido de `RAM[0x0A]` consigo mismo. El resultado de la resta es `0x000000`, por lo que se espera que las flags reflejen una comparación de valores iguales.

Al finalizar la ejecución, únicamente debe estar activa la flag **Zero**. Este estado puede observarse directamente en el registro de FLAGS del circuito como `0x02`.

Para reproducir los resultados esperados, debe considerarse el estado inicial de la RAM y de las flags de entrada de `ADC` y `SBC`. Las posiciones empleadas como operandos deben contener los valores previstos por el programa antes de su lectura.

La prueba se considera satisfactoria cuando el compilador genera el archivo de instrucciones sin errores y la ejecución en `CPU.circ` produce los resultados esperados en las posiciones de RAM indicadas, junto con el estado de FLAGS esperado.