# Milo Assembly - Syntax Highlighting

Extensión de resaltado de sintaxis para el **Milo ISA v2.0.0**.

Compatible con:
- VS Code
- VSCodium
- Code-OSS

## Instrucciones soportadas

**Transferencia**
- `MOV`

**Aritmético-Lógicas**
- `ADD` `ADC` `SUB` `SBC`
- `AND` `OR` `XOR`
- `SHL` `SHR`
- `CMP` `NOT`

**Control de flujo**
- `JMP`
- `JZ` `JNZ`
- `JC` `JNC`
- `JN` `JNN`
- `JV` `JNV`
- `CALL` `RET`

**Otros**
- Modificador `.F`
- Registros `R0`–`R15`
- Inmediatos `#n` / `#0x...`
- Acceso a memoria `[Rn]`, `[#n]`, `RAM`
- Comentarios con `;`
- Etiquetas `NOMBRE:`

## Instalación

### Método recomendado (desarrollo local)

1. Abre VS Code / VSCodium / Code-OSS
2. `Ctrl+Shift+P` → escribe: **Developer: Install Extension from Location...**
3. Selecciona la carpeta `milo-language`
4. Recarga la ventana (`Ctrl+Shift+P` → **Developer: Reload Window**)

### Método alternativo (enlace simbólico)

```bash
# Linux / macOS
ln -s /ruta/a/milo-language ~/.vscode/extensions/milo-language
# o para VSCodium:
ln -s /ruta/a/milo-language ~/.vscode-oss/extensions/milo-language
```

Luego recarga la ventana.

## Extensiones de archivo reconocidas

- `.milo`
- `.s`
- `.asm`

## Notas

- La grammar es **case-insensitive** (acepta tanto `MOV` como `mov`).
- Solo se reconocen registros del `R0` al `R15`.
- Los comentarios empiezan con `;` y llegan hasta el final de la línea.
