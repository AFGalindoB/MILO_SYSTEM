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

function renderizarListaLienzos() {
    if (!listaSpritesDOM) return;
    listaSpritesDOM.innerHTML = "";

    bancoLienzos.forEach((lienzo, index) => {
        const item = document.createElement("div");
        item.classList.add("sprite-item");
        if (index === indiceLienzoActivo) item.classList.add("active");
        item.dataset.index = index;

        item.innerHTML = `
            <span class="sprite-index">#${String(index + 1).padStart(2, '0')}</span>
            <span class="sprite-name">${lienzo.nombre}</span>
        `;

        // Intercambio de buffer al cambiar de lienzo
        item.addEventListener("click", () => {
            if (indiceLienzoActivo === index) return;
            
            // Sincronizar el estado anterior por seguridad antes de saltar
            bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();
            
            // Cambiar puntero del lienzo activo
            indiceLienzoActivo = index;
            
            // Cargar los datos del nuevo lienzo en la VRAM y redibujar selectores
            editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz);
            renderizarListaLienzos();
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

// ---- EXPORTACIÓN DE TODO EL TILESET (TODOS LOS LIENZOS) ----
document.getElementById("btnExportar").addEventListener("click", () => {
    // 1. Asegurar que el lienzo que se está editando actualmente esté guardado en memoria
    bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();

    // 2. Concatenar las matrices de TODOS los lienzos en un único arreglo plano
    const todosLosPixeles = [];
    bancoLienzos.forEach(lienzo => {
        todosLosPixeles.push(...lienzo.matriz);
    });

    // 3. Convertir cada índice a su representación Hexadecimal (0-f) por línea
    const listaIndicesHex = todosLosPixeles.map(indice => indice.toString(16));
    const textoPlano = listaIndicesHex.join("\n");

    // 4. Descargar el archivo empaquetado
    const blob = new Blob([textoPlano], { type: "text/plain;charset=utf-8" });
    const url = URL.createObjectURL(blob);
    
    let nombreArchivo = inputNombreArchivo ? inputNombreArchivo.value.trim() : "tileset_indexed";
    if (nombreArchivo === "") nombreArchivo = "tileset_indexed";

    const a = document.createElement("a");
    a.href = url;
    a.download = `${nombreArchivo}.txt`;
    document.body.appendChild(a);
    a.click();
    
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    console.log(`🚀 Tileset exportado con éxito. Total lienzos: ${bancoLienzos.length}`);
});

// ---- IMPORTACIÓN DINÁMICA DE TILESET AGREGANDO AL FINAL ----
document.getElementById("upload-tile").addEventListener("change", function(evento) {
    const archivo = evento.target.files[0];
    if (!archivo) return;

    const nombreSinExtension = archivo.name.replace(/\.[^/.]+$/, "");
    if (inputNombreArchivo) inputNombreArchivo.value = nombreSinExtension;

    const lector = new FileReader();
    lector.onload = function(e) {
        const contenido = e.target.result;
        // Limpiar saltos de línea y filtrar vacíos
        const lineas = contenido.split(/\r?\n/).map(l => l.trim()).filter(l => l.length > 0);
        
        // Validación básica: el archivo debe ser múltiplo de 64 (8x8)
        if (lineas.length === 0 || lineas.length % TOTAL_PIXELS !== 0) {
            alert(`Error: El archivo no tiene un tamaño válido. Contiene ${lineas.length} píxeles, y debe ser múltiplo de ${TOTAL_PIXELS} para armar la cuadrícula de 8x8.`);
            return;
        }

        // 1. Asegurar la persistencia del lienzo que el usuario está editando actualmente antes de la importación
        bancoLienzos[indiceLienzoActivo].matriz = editorLienzo.getData();

        // 2. Determinar en qué posición del banco vamos a empezar a insertar
        let totalLienzosPrevios = bancoLienzos.length;
        let primerNuevoIndice = totalLienzosPrevios; // El primer lienzo importado tomará este índice

        let matrizTemporal = [];
        let contadorNuevosLienzos = 0;

        // 3. Procesar el flujo de píxeles e ir agregando los nuevos lienzos al banco existente
        for (let i = 0; i < lineas.length; i++) {
            const caracterHex = lineas[i];
            const indiceColor = parseInt(caracterHex, 16);
            
            // Validar rango de color (0-15)
            const colorValidado = (!isNaN(indiceColor) && indiceColor >= 0 && indiceColor <= 15) ? indiceColor : 0;
            matrizTemporal.push(colorValidado);

            // Cada vez que acumulamos un bloque de 64 píxeles (8x8)
            if (matrizTemporal.length === TOTAL_PIXELS) {
                contadorNuevosLienzos++;
                const nuevoId = totalLienzosPrevios + contadorNuevosLienzos - 1;

                bancoLienzos.push({
                    id: nuevoId,
                    nombre: `Lienzo ${nuevoId + 1}`,
                    matriz: [...matrizTemporal]
                });

                // Resetear el acumulador para el próximo bloque
                matrizTemporal = [];
            }
        }

        // 4. Mover el foco activo automáticamente al primer lienzo de la tanda recién importada
        indiceLienzoActivo = primerNuevoIndice;

        // 5. Cargar los datos en la pantalla física y actualizar el selector del DOM
        editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz);
        renderizarListaLienzos();
        
        console.log(`📥 Importación sucesiva completada. Se añadieron ${contadorNuevosLienzos} nuevos lienzos sin alterar los anteriores.`);
    };

    lector.readAsText(archivo);
    evento.target.value = ""; // Resetear el input para permitir cargar el mismo archivo consecutivamente si se desea
});

inicializarPaleta();
editorLienzo.setData(bancoLienzos[indiceLienzoActivo].matriz); // Cargar datos del Lienzo 1
editorLienzo.inicializarCuadrigrid(manejadorMouseDown, manejadorMouseEnter);
actualizarMonitoresDeBrocha();
renderizarListaLienzos();