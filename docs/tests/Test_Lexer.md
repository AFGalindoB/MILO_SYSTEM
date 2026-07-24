# Test del Lexer

Versión 1.0.0

## Objetivo

El test del lexer verifica que el código fuente escrito en lenguaje ensamblador sea transformado correctamente en la secuencia de tokens que posteriormente consumirá el parser.

Al tratarse de la primera etapa del compilador, cualquier error durante el análisis léxico se propagará al resto del pipeline de compilación. Por este motivo, el objetivo principal de esta prueba consiste en garantizar que cada elemento del lenguaje sea reconocido de forma correcta antes de iniciar el análisis sintáctico.

## Aspectos verificados

Actualmente el test valida el reconocimiento de los principales elementos léxicos definidos por el lenguaje Milo ASM.

Entre ellos:

- Mnemónicos.
- Etiquetas.
- Registros.
- Valores inmediatos.
- Números.
- Modificadores de instrucción.
- Comas y símbolos de puntuación.
- Corchetes utilizados para direccionamiento.
- Comentarios.
- Finales de línea.
- Caracteres inválidos.
- Información de línea y columna utilizada para el diagnóstico de errores.

## Casos de prueba

El programa utilizado por el test contiene tanto instrucciones válidas como entradas deliberadamente incorrectas.

Esta combinación permite comprobar simultáneamente:

- Reconocimiento correcto de instrucciones válidas.
- Normalización del texto (por ejemplo, conversión a mayúsculas).
- Manejo de los límites del lenguaje (como el registro `R15`).
- Identificación de registros inexistentes.
- Manejo de inmediatos mal formados.
- Detección de caracteres ilegales.
- Validación de modificadores de instrucción.
- Recuperación ante errores léxicos.

El objetivo no consiste únicamente en comprobar que el lexer funcione correctamente cuando la entrada es válida, sino también verificar que produzca tokens de error adecuados cuando encuentra código fuente inválido.

## Salida esperada

Durante la ejecución el test imprime una tabla con todos los tokens generados por el lexer.

Cada fila representa un token reconocido e incluye la siguiente información:

| Campo   | Descripción                                                          |
| ------- | -------------------------------------------------------------------- |
| Línea   | Línea donde fue encontrado el token.                                 |
| Columna | Columna inicial del token.                                           |
| Tipo    | Categoría léxica reconocida.                                         |
| Lexema  | Texto original asociado al token.                                    |
| Valor   | Valor numérico cuando aplica (registros, inmediatos, números, etc.). |

La inspección de esta salida permite comprobar visualmente que el flujo de tokens generado por el lexer coincide con el código fuente original.

## Criterios de éxito

La prueba se considera satisfactoria cuando:

- Todos los elementos válidos producen el tipo de token esperado.
- Los valores asociados a registros e inmediatos son correctos.
- La información de línea y columna coincide con la posición real dentro del programa.
- Las entradas inválidas generan tokens de error sin interrumpir el análisis del resto del archivo.
- El flujo de tokens finaliza con un único token `TOKEN_EOF`.