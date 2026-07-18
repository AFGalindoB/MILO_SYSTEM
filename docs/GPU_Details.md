# GPU MILO - Especificación de Arquitectura

## Objetivo

La GPU MILO es un controlador gráfico 2D basado en tiles cuya función es generar una señal de vídeo a partir de estructuras de memoria simples.

El diseño prioriza:

- Hardware reducido.
- Funcionamiento determinista.
- Bajo consumo de memoria.
- Renderizado por hardware basado en tiles.
- Interfaz simple con la CPU.

El diseño prioriza un hardware pequeño, determinista y sencillo de implementar.

## Filosofía de Diseño

La GPU MILO actúa como un generador continuo de vídeo.

Durante cada ciclo de renderizado obtiene la información necesaria desde las memorias gráficas para calcular el color correspondiente al píxel actual.

La GPU no posee conocimiento del contenido lógico mostrado en pantalla. Su única responsabilidad es transformar la información almacenada en memoria en una señal de vídeo.

Toda la lógica de alto nivel es responsabilidad del software ejecutado por la CPU.

Las responsabilidades quedan divididas así:

La CPU es responsable de administrar el contenido mostrado en pantalla.

Dependiendo del software en ejecución, esto puede incluir:

- Interfaces de usuario.
- Consolas de comandos.
- Aplicaciones gráficas.
- Videojuegos.
- Sistemas operativos.

GPU

- Lee continuamente el Tile Buffer.
- Lee la ROM de recursos.
- Consulta la LUT de colores.
- Genera el color de cada píxel.

## Arquitectura General

```text
CPU
│
├───────────────┐
│               │
▼               ▼

Registros      Tile Buffer 
  
        |
        v
Generador de Direcciones
        |
        v
  ROM de Recursos
        |
        v
       LUT
        |
        v
    Video Output
```

## Caracteristicas

## Características

| Parámetro            | Valor            |
| -------------------- | ---------------- |
| Resolución           | 64 × 64 píxeles  |
| Tamaño del tile      | 8 × 8 píxeles    |
| Viewport             | 8 × 8 tiles      |
| Profundidad de color | 4 bits indexados |
| Salida de vídeo      | RGB565           |

## Tile Buffer

El Tile Buffer almacena los tiles que serán utilizados durante el proceso de renderizado.

```text
+-------------------------------+
|                               |
|      Tile Buffer              |
|                               |
|      +-----------+            |
|      | Viewport  |            |
|      | 64x64     |            |
|      +-----------+            |
|                               |
+-------------------------------+
```

El Tile Buffer contiene los índices de los tiles visibles utilizados durante el proceso de renderizado.

Su contenido puede ser actualizado por la CPU en cualquier momento.

La GPU no modifica esta memoria.

## Scroll

El scroll se divide en dos niveles.

### Pixel Offset

Registro utilizado para desplazar el viewport a nivel de píxel.

No modifica el contenido del Tile Buffer.

---

### Tile Offset

Registro utilizado para modificar la dirección inicial de lectura del Tile Buffer.

Permite implementar scroll por hardware sin necesidad de mover bloques completos de memoria.

### Tile Buffer Circular

El Tile Buffer puede utilizarse como una memoria circular.

En esta configuración la CPU sobrescribe las filas o columnas que abandonan el viewport con nuevos datos provenientes del mapa del software.

Este mecanismo permite representar mapas de tamaño arbitrario utilizando una cantidad fija de memoria.

## Recursos Gráficos

Los recursos gráficos se almacenan en una memoria ROM.

Cada dirección representa un píxel perteneciente a un tile.

El dato almacenado no corresponde directamente a un color RGB, sino a un índice de color.

Este índice es utilizado posteriormente por la LUT para obtener el color final.

### LUT

La Look-Up Table (LUT) convierte un índice de color en un valor RGB565.

Este mecanismo permite modificar la representación de los colores sin alterar los recursos gráficos almacenados en la ROM.

## Pipeline de Renderizado

Cada píxel sigue el siguiente recorrido.

```text
Raster

↓

Generación de Coordenadas

↓

Tile Buffer (Indice del tile)

↓

Generación de Dirección del Tile

↓

ROM de Recursos (Indice de color)

↓

LUT (RGB 565)

↓

Salida de Vídeo
```

## Responsabilidades del Hardware

La GPU únicamente:

- Lee memoria.
- Calcula direcciones.
- Genera colores.

No:

- Gestiona mapas.
- Gestiona chunks.
- Gestiona IA.
- Gestiona lógica del juego.

Toda esa responsabilidad pertenece a la CPU.

## Interfaz Hardware

La GPU es controlada mediante un conjunto de registros y buses de datos accesibles por la CPU.

### Registro Pixel Offset

Ancho: **6 bits**

| Bits  | Campo    | Descripción                                   |
| ----- | -------- | --------------------------------------------- |
| `5-3` | Y Offset | Desplazamiento fino vertical (0-7 píxeles).   |
| `2-0` | X Offset | Desplazamiento fino horizontal (0-7 píxeles). |

Cuando una suma produce un acarreo, este se propaga al registro **Tile Offset** correspondiente.

---

### Registro Tile Offset

Ancho: **8 bits**

| Bits  | Campo  | Descripción                                |
| ----- | ------ | ------------------------------------------ |
| `7-4` | Tile Y | Desplazamiento vertical del Tile Buffer.   |
| `3-0` | Tile X | Desplazamiento horizontal del Tile Buffer. |

Este registro define el tile inicial desde el cual comienza la lectura del Tile Buffer.

## Propagación del Carry

El Pixel Offset y el Tile Offset forman un único desplazamiento lógico.

Cuando el Pixel Offset produce un desbordamiento:

X Pixel Offset → Carry → Tile Offset X

Y Pixel Offset → Carry → Tile Offset Y

El desbordamiento del Tile Offset se descarta, implementando un comportamiento circular sobre el Tile Buffer.

## Escritura de Offset

La CPU escribe simultáneamente ambos registros mediante un bus de 14 bits.

| Bits   | Destino      |
| ------ | ------------ |
| `13-8` | Pixel Offset |
| `7-0`  | Tile Offset  |

## Escritura del Tile Buffer

Cada operación de escritura utiliza un bus de 24 bits.

| Bits   | Campo           | Descripción                                                               |
| ------ | --------------- | ------------------------------------------------------------------------- |
| `23-8` | Índice del tile | Selecciona el recurso gráfico almacenado en la ROM.                       |
| `7-0`  | Dirección       | Selecciona la posición dentro del Tile Buffer donde se almacenará el tile.|

## Buses

| Bus                 |   Ancho | Uso                             |
| ------------------- | ------- | ------------------------------- |
| Pixel Offset        |  6 bits | Scroll fino                     |
| Tile Offset         |  8 bits | Scroll grueso                   |
| Tile Buffer Write   | 24 bits | Escritura del Tile Buffer       |
| Control             |  9 bits | Señales de escritura            |
| Tile Buffer Address |  8 bits | Dirección interna               |
| Tile ROM Address    | 16 bits | Dirección de la ROM de recursos |
| LUT Address         |  4 bits | Índice de color                 |
| RGB Output          | 16 bits | RGB565                          |
