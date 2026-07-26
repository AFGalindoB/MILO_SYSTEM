# Test Movimiento General

El test **Movimiento General** verifica la correcta generación de código para las instrucciones de transferencia de datos entre registros, memoria y registros especiales de la arquitectura Milo.

Estas instrucciones constituyen las operaciones fundamentales utilizadas para mover información dentro del procesador y permiten comprobar el funcionamiento del datapath asociado al Bus C.

## Escenarios cubiertos

Actualmente la prueba incluye las siguientes instrucciones:

```text
MOVI R0, #5
MOV  R1, R0

STORE R0, [R0]
LOAD  R5, [R0]

MOVI R2, #1

STORE #3, [R2]
LOAD  R3, [R2]

BUCLE:
STORE RINPT, [R3]
JMP BUCLE
```

Con esta secuencia se valida:

- Carga de valores inmediatos mediante `MOVI`.
- Transferencia entre registros mediante `MOV`.
- Escritura en memoria desde un registro (`STORE Rn, [Rn]`).
- Escritura en memoria desde un valor inmediato (`STORE #n, [Rn]`).
- Escritura en memoria desde un registro de solo lectura (`STORE RINPT, [Rn]`).
- Lectura desde memoria mediante `LOAD`.
- Funcionamiento del salto incondicional `JMP`.

El programa verifica además que la instrucción `STORE` pueda obtener el dato desde cualquiera de las fuentes actualmente soportadas por el **Bus C Source Selector**:

- Banco de registros.
- Memoria ROM (valor inmediato).
- Registro de entrada (`RINPT`).

Cada una de estas instrucciones utiliza rutas de datos distintas dentro del procesador, permitiendo comprobar la correcta configuración de buses, registros y señales de control durante la generación del código.

## Resultado esperado

La compilación debe finalizar correctamente y generar el archivo:

`compilaciones/instrucciones/test_movimiento_general.txt`

El archivo generado puede cargarse posteriormente en la memoria ROM del circuito `CPU.circ` para verificar el comportamiento físico del procesador.

Durante la ejecución debe observarse que:

- `MOVI` carga correctamente valores inmediatos.
- `MOV` copia correctamente datos entre registros.
- `STORE` escribe correctamente en memoria cuando el dato proviene del banco de registros.
- `STORE` escribe correctamente en memoria cuando el dato proviene de un valor inmediato.
- `STORE` escribe correctamente en memoria cuando el dato proviene del registro `RINPT`.
- `LOAD` recupera correctamente los valores previamente almacenados.
- El bucle final mantiene la escritura continua del registro de entrada sobre la dirección indicada por `R3`, permitiendo observar el comportamiento dinámico del sistema de entrada.

La memoria RAM, el banco de registros y el Bus C deben reflejar el comportamiento esperado durante toda la ejecución.

![Test Movimiento General](../images/test_movimiento_general.png)