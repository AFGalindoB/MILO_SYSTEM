# Test Input

El **Test Input** verifica la generación de código para el registro especial de entrada del procesador.

Su objetivo principal consiste en comprobar que el compilador traduzca correctamente las instrucciones que utilizan el registro `RINPT`, permitiendo leer el estado del sistema de entrada y utilizar dicha información para controlar el flujo del programa y actualizar periféricos como la GPU.

## Escenarios cubiertos

Actualmente la prueba verifica:

- Lectura del registro especial `RINPT`.
- Transferencia de datos desde un registro especial de solo lectura hacia un registro de propósito general mediante `MOV`.
- Evaluación individual de cada bit del registro de entrada utilizando máscaras y operaciones lógicas.
- Actualización de las banderas mediante `AND.F`.
- Uso de saltos condicionales (`JZ`) para detectar el estado de cada botón.
- Escritura sobre el registro especial `TBUF`.
- Sincronización CPU-GPU mediante `WAITV`.
- Funcionamiento conjunto del subsistema de entrada, la ALU, el control de flujo y la GPU.

El programa utilizado durante la prueba es el siguiente:

```text
; --- Lectura del registro de entrada ---
MOV R0, RINPT
MOVI R1, #0

; --- Botón A ---
MOVI R2, #1
AND.F R3, R0, R2
JZ #7
MOVI R1, #256
JMP #41

; --- Botón B ---
MOVI R2, #2
AND.F R3, R0, R2
JZ #12
MOVI R1, #512
JMP #41

; --- Botón X ---
MOVI R2, #4
AND.F R3, R0, R2
JZ #17
MOVI R1, #6144
JMP #41

; --- Botón Y ---
MOVI R2, #8
AND.F R3, R0, R2
JZ #22
MOVI R1, #6400
JMP #41

; --- Botón UP ---
MOVI R2, #16
AND.F R3, R0, R2
JZ #27
MOVI R1, #5376
JMP #41

; --- Botón LEFT ---
MOVI R2, #32
AND.F R3, R0, R2
JZ #32
MOVI R1, #3072
JMP #41

; --- Botón RIGHT ---
MOVI R2, #64
AND.F R3, R0, R2
JZ #37
MOVI R1, #4608
JMP #41

; --- Botón DOWN ---
MOVI R2, #128
AND.F R3, R0, R2
JZ #41
MOVI R1, #1024

; --- Sincronización y escritura ---
WAITV
MOV TBUF, R1
JMP #0
```

El programa implementa un bucle continuo que lee el estado del registro `RINPT`, evalúa secuencialmente cada uno de los ocho botones disponibles y selecciona el tile correspondiente para mostrarlo en pantalla.

Cada botón se identifica mediante una máscara distinta aplicada sobre el valor leído desde RINPT. Si el bit correspondiente está activo, se prepara el índice del tile asociado y posteriormente se escribe en el Tile Buffer durante el período de Vertical Blank mediante la instrucción `WAITV`.

## Resultado esperado

La compilación debe finalizar sin errores y generar el archivo:

`compilaciones/test_rinpt.txt`

El archivo generado puede cargarse posteriormente en la ROM del circuito CPU.circ para verificar el funcionamiento conjunto del procesador y el subsistema de entrada.

Durante la ejecución, al presionar cualquiera de los botones del controlador, el procesador debe actualizar el primer elemento del Tile Buffer con el carácter correspondiente:

| Botón | Carácter mostrado |
| ----- | ----------------- |
| A     | A                 |
| B     | B                 |
| X     | X                 |
| Y     | Y                 |
| UP    | U                 |
| LEFT  | L                 |
| RIGHT | R                 |
| DOWN  | D                 |

La actualización debe realizarse únicamente durante el período de **Vertical Blank**, verificando así el correcto funcionamiento de la instrucción `WAITV` y la comunicación entre la CPU y la GPU.