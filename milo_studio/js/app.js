import { 
    coloresLUT, 
    inicializarPaleta, 
    suscribirACambioColor
} from './palette.js';

import { CanvasEditor, TOTAL_PIXELS } from './canvas.js';

const inputNombreArchivo = document.getElementById("inputNombreArchivo");
const canvasGrid = document.getElementById("canvasGrid");

const indicadorPrincipal = document.getElementById("indicadorColorPrincipal");
const indicadorSecundario = document.getElementById("indicadorColorSecundario");

// Elementos del selector de lienzos
const listaSpritesDOM = document.getElementById("listaSprites");
const btnAnadirSprite = document.getElementById("btnAnadirSprite");

let estaPintando = false;
let botonMousePresionado = null;
let colorPrincipalIndice = 0;
let colorSecundarioIndice = 15;

// --- BANCO DE LIENZOS EN MEMORIA ---
// Iniciamos con un solo lienzo vacío (lleno de ceros)
let bancoLienzos = [
    { id: 0, nombre: "Lienzo 1", matriz: new Array(TOTAL_PIXELS).fill(0) }
];
let indiceLienzoActivo = 0;

const editorLienzo = new CanvasEditor(canvasGrid);

function actualizarMonitoresDeBrocha() {
    if (indicadorPrincipal) indicadorPrincipal.style.backgroundColor = coloresLUT[colorPrincipalIndice];
    if (indicadorSecundario) indicadorSecundario.style.backgroundColor = coloresLUT[colorSecundarioIndice];
}

function procesarIntentoPintado(indicePixel) {
    const colorAEscribir = (botonMousePresionado === 0) ? colorPrincipalIndice : colorSecundarioIndice;
    
    editorLienzo.actualizarPixel(indicePixel, colorAEscribir);
    
    bancoLienzos[indiceLienzoActivo].matriz[indicePixel] = colorAEscribir;
}

function manejadorMouseDown(e, elementoPixel, indice, x, y) {
    e.preventDefault();
    if (e.button === 0 || e.button === 2) {
        estaPintando = true;
        botonMousePresionado = e.button;
        procesarIntentoPintado(indice);
    }
}

function manejadorMouseEnter(elementoPixel, indice, x, y) {
    if (estaPintando && botonMousePresionado !== null) {
        procesarIntentoPintado(indice);
    }
}

function eliminarLienzo(indexAEliminar) {
    // 1. Regla de seguridad: No permitir borrar si solo queda un lienzo en el banco
    if (bancoLienzos.length <= 1) {
        alert("El editor debe contener al menos un lienzo activo.");
        return;
    }

    // 2. Remover el lienzo de la memoria
    bancoLienzos.splice(indexAEliminar, 1);

    // 3. Ajustar el puntero del lienzo activo
    if (indexAEliminar === indiceLienzoActivo) {
        // Si borramos el lienzo en el que estábamos parados, seleccionamos el anterior o el cero
        indiceLienzoActivo = Math.max(0, indexAEliminar - 1);
    } else if (indexAEliminar < indiceLienzoActivo) {
        // Si borramos un lienzo a la izquierda del activo, recorremos el índice 1 posición atrás
        indiceLienzoActivo--;
    }

    // 4. Refrescar el canvas físico y la lista gráfica
    editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz);
    renderizarListaLienzos();
    console.log(`🗑️ Lienzo borrado. Quedan ${bancoLienzos.length} lienzos.`);
}

function renderizarListaLienzos() {
    if (!listaSpritesDOM) return;
    listaSpritesDOM.innerHTML = "";

    bancoLienzos.forEach((lienzo, index) => {
        const item = document.createElement("div");
        item.classList.add("sprite-item");
        if (index === indiceLienzoActivo) item.classList.add("active");
        item.dataset.index = index;

        // Añadimos el botón de eliminar <button class="btn-delete-sprite">×</button>
        item.innerHTML = `
            <span class="sprite-index">#${String(index + 1).padStart(2, '0')}</span>
            <span class="sprite-name">${lienzo.nombre}</span>
            <button class="btn-delete-sprite" title="Eliminar lienzo" aria-label="Eliminar lienzo">×</button>
        `;

        // Intercambio de buffer al cambiar de lienzo
        item.addEventListener("click", () => {
            if (indiceLienzoActivo === index) return;
            
            // Sincronizar el estado anterior por seguridad antes de saltar
            bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();
            
            // Cambiar puntero del lienzo activo
            indiceLienzoActivo = index;
            
            // Cargar los datos del nuevo lienzo en la VRAM y redibujar
            editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz);
            renderizarListaLienzos();
        });

        // Evento para la X de eliminación
        const btnEliminar = item.querySelector(".btn-delete-sprite");
        btnEliminar.addEventListener("click", (e) => {
            e.stopPropagation(); // Evitar que seleccione el lienzo al hacer clic en borrar
            eliminarLienzo(index);
        });

        listaSpritesDOM.appendChild(item);
    });
}

if (btnAnadirSprite) {
    btnAnadirSprite.addEventListener("click", () => {
        // Asegurar persistencia del lienzo actual
        bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();

        const nuevoId = bancoLienzos.length;
        const nuevoLienzo = {
            id: nuevoId,
            nombre: `Lienzo ${nuevoId + 1}`,
            matriz: new Array(TOTAL_PIXELS).fill(0)
        };

        bancoLienzos.push(nuevoLienzo);
        indiceLienzoActivo = nuevoId;

        // Limpiar el editor físico e iterar interfaz
        editorLienzo.setData(nuevoLienzo.matriz);
        renderizarListaLienzos();
    });
}

canvasGrid.addEventListener("contextmenu", e => e.preventDefault());

suscribirACambioColor((nuevoIndice, colorHex, paletaCompletaCambio = false) => {
    colorPrincipalIndice = nuevoIndice;
    actualizarMonitoresDeBrocha();

    if (paletaCompletaCambio) {
        editorLienzo.refrescarColoresPorCambioPaleta();
    }
});

document.getElementById("paletteGrid").addEventListener("contextmenu", (e) => {
    e.preventDefault();
    const botonColor = e.target.closest(".color-slot");
    if (botonColor) {
        const nuevoIndice = parseInt(botonColor.dataset.index, 10);
        if (!isNaN(nuevoIndice)) {
            colorSecundarioIndice = nuevoIndice;
            actualizarMonitoresDeBrocha();
        }
    }
});

window.addEventListener("mouseup", () => {
    estaPintando = false;
    botonMousePresionado = null;
});

// ---- EXPORTACIÓN DE TODO EL TILESET (8bpp: 8 píxeles por fila, 2 dígitos hex por píxel) ----
document.getElementById("btnExportar").addEventListener("click", () => {
    // 1. Asegurar que el lienzo activo se guarde en el banco de memoria
    bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();

    const lineasExportacion = [];

    // 2. Procesar cada lienzo del banco
    bancoLienzos.forEach(lienzo => {
        // Un tile de 8x8 consta de 8 filas
        for (let fila = 0; fila < 8; fila++) {
            let stringFila = "";
            for (let col = 0; col < 8; col++) {
                const indicePixel = fila * 8 + col;
                const valorColor = lienzo.matriz[indicePixel];
                
                // Formatear a 2 dígitos hexadecimales en minúscula (ej: 0 -> "00", 12 -> "0c")
                const hex2Digitos = valorColor.toString(16).padStart(2, '0');
                stringFila += hex2Digitos;
            }
            lineasExportacion.push(stringFila);
        }
    });

    // 3. Unir las filas por salto de línea
    const textoPlano = lineasExportacion.join("\n");

    // 4. Descargar el archivo
    const blob = new Blob([textoPlano], { type: "text/plain;charset=utf-8" });
    const url = URL.createObjectURL(blob);
    
    let nombreArchivo = inputNombreArchivo ? inputNombreArchivo.value.trim() : "tileset_8bpp";
    if (nombreArchivo === "") nombreArchivo = "tileset_8bpp";

    const a = document.createElement("a");
    a.href = url;
    a.download = `${nombreArchivo}.txt`;
    document.body.appendChild(a);
    a.click();
    
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    console.log(`🚀 Tileset 8bpp exportado con éxito. Total lienzos: ${bancoLienzos.length}`);
});

// ---- IMPORTACIÓN HÍBRIDA / AUTO-DETECTABLE (v1 Legacy & v2 8bpp) ----
document.getElementById("upload-tile").addEventListener("change", function(evento) {
    const archivo = evento.target.files[0];
    if (!archivo) return;

    const nombreSinExtension = archivo.name.replace(/\.[^/.]+$/, "");
    if (inputNombreArchivo) inputNombreArchivo.value = nombreSinExtension;

    const lector = new FileReader();
    lector.onload = function(e) {
        const contenido = e.target.result;
        // Limpiar saltos de línea y filtrar líneas vacías
        const lineas = contenido.split(/\r?\n/).map(l => l.trim()).filter(l => l.length > 0);
        
        if (lineas.length === 0) {
            alert("Error: El archivo cargado está vacío.");
            return;
        }

        // 1. Detección automática de formato examinando la primera línea
        const primeraLineaLen = lineas[0].length;
        let esFormatoV2 = false;

        if (primeraLineaLen > 2) {
            // Si la línea tiene más de 2 caracteres (esperamos 16), asumimos Formato v2 (8bpp por fila)
            esFormatoV2 = true;
        } else if (lineas.length % TOTAL_PIXELS === 0) {
            // Si las líneas equivalen exactamente a múltiplos de 64, es Formato v1 (1 píxel por línea)
            esFormatoV2 = false;
        } else if (lineas.length % 8 === 0) {
            // Si la longitud de línea es ambigua pero el total de líneas es múltiplo de 8, asumimos v2
            esFormatoV2 = true;
        }

        // 2. Validación de integridad de estructura según el formato detectado
        if (esFormatoV2) {
            if (lineas.length % 8 !== 0) {
                alert(`Error [v2 8bpp]: El archivo debe tener un número de líneas múltiplo de 8. Se encontraron ${lineas.length} líneas.`);
                return;
            }
        } else {
            if (lineas.length % TOTAL_PIXELS !== 0) {
                alert(`Error [v1 Legacy]: El archivo debe tener un número de líneas múltiplo de 64 (${TOTAL_PIXELS}). Se encontraron ${lineas.length} líneas.`);
                return;
            }
        }

        // 3. Persistir el lienzo actual antes de cargar los nuevos
        bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();

        let totalLienzosPrevios = bancoLienzos.length;
        let primerNuevoIndice = totalLienzosPrevios;
        let contadorNuevosLienzos = 0;

        // 4. Parser según el formato detectado
        if (esFormatoV2) {
            // --- PARSER FORMATO V2 (8bpp: 8 filas por tile, 16 hex chars por fila) ---
            for (let i = 0; i < lineas.length; i += 8) {
                const matrizTile = [];

                for (let fila = 0; fila < 8; fila++) {
                    const lineaHex = lineas[i + fila];

                    for (let col = 0; col < 8; col++) {
                        // Extraer pares de caracteres hexadecimales (00 - FF)
                        const subHex = lineaHex.substr(col * 2, 2);
                        let valIndex = parseInt(subHex, 16);

                        if (isNaN(valIndex)) valIndex = 0;

                        // Como el GUI actual soporta 16 colores, aplicamos una máscara
                        // para proyectarlo de forma segura al rango 0-15
                        valIndex = valIndex & 0x0F;

                        matrizTile.push(valIndex);
                    }
                }

                contadorNuevosLienzos++;
                const nuevoId = totalLienzosPrevios + contadorNuevosLienzos - 1;

                bancoLienzos.push({
                    id: nuevoId,
                    nombre: `Lienzo ${nuevoId + 1}`,
                    matriz: matrizTile
                });
            }
            console.log(`📥 Importación V2 (8bpp) exitosa. Se añadieron ${contadorNuevosLienzos} lienzos.`);

        } else {
            // --- PARSER FORMATO V1 (Legacy: 1 hex char por línea, 64 líneas por tile) ---
            let matrizTemporal = [];

            for (let i = 0; i < lineas.length; i++) {
                const caracterHex = lineas[i];
                let indiceColor = parseInt(caracterHex, 16);

                if (isNaN(indiceColor)) indiceColor = 0;
                indiceColor = indiceColor & 0x0F;

                matrizTemporal.push(indiceColor);

                if (matrizTemporal.length === TOTAL_PIXELS) {
                    contadorNuevosLienzos++;
                    const nuevoId = totalLienzosPrevios + contadorNuevosLienzos - 1;

                    bancoLienzos.push({
                        id: nuevoId,
                        nombre: `Lienzo ${nuevoId + 1}`,
                        matriz: [...matrizTemporal]
                    });

                    matrizTemporal = [];
                }
            }
            console.log(`📥 Importación V1 (Legacy) exitosa. Se añadieron ${contadorNuevosLienzos} lienzos.`);
        }

        // 5. Mover foco al primer lienzo recién importado y refrescar la vista
        indiceLienzoActivo = primerNuevoIndice;
        editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz);
        renderizarListaLienzos();
    };

    lector.readAsText(archivo);
    evento.target.value = ""; // Permitir recargar el mismo archivo si se desea
});

inicializarPaleta();
editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz); // Cargar datos del Lienzo 1
editorLienzo.inicializarCuadrigrid(manejadorMouseDown, manejadorMouseEnter);
actualizarMonitoresDeBrocha();
renderizarListaLienzos();