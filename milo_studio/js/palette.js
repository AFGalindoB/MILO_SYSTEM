// Valores por defecto (colores iniciales ARGB simulados en CSS hex)
export const coloresLUT = [
    "#1a1c2c", "#5d275d", "#b13e53", "#ef7d57",
    "#ffcd75", "#a7f070", "#38b764", "#257179",
    "#29366f", "#3b5dc9", "#41a6f6", "#73eff7",
    "#f4f4f4", "#94b0c2", "#566c86", "#333c57"
];

export let colorSeleccionadoIndice = 0;
const paletteGrid = document.getElementById("paletteGrid");

// Callback que ejecutará app.js cuando el usuario elija otro color o cambie la paleta
let alCambiarColorCallback = null;

export function suscribirACambioColor(callback) {
    alCambiarColorCallback = callback;
}

export function inicializarPaleta() {
    if (!paletteGrid) return; // Salvaguarda pasiva
    paletteGrid.innerHTML = ""; 

    coloresLUT.forEach((colorHex, indice) => {
        const slot = document.createElement("button");
        slot.type = "button";
        slot.classList.add("color-slot");
        slot.style.backgroundColor = colorHex;
        slot.title = `Índice LUT: ${indice} (${colorHex})`;
        slot.dataset.index = indice;

        if (indice === colorSeleccionadoIndice) {
            slot.classList.add("selected");
        }

        slot.addEventListener("click", () => {
            const anterior = paletteGrid.querySelector(".color-slot.selected");
            if (anterior) anterior.classList.remove("selected");
            
            slot.classList.add("selected");
            colorSeleccionadoIndice = indice;

            if (alCambiarColorCallback) {
                alCambiarColorCallback(colorSeleccionadoIndice, coloresLUT[colorSeleccionadoIndice]);
            }
        });

        paletteGrid.appendChild(slot);
    });
}

/**
 * Carga una paleta desde un array de strings
 */
export function actualizarPaletaDesdeArray(nuevosColores) {
    for (let i = 0; i < 16; i++) {
        if (nuevosColores[i]) {
            coloresLUT[i] = nuevosColores[i];
        } else {
            coloresLUT[i] = "#000000"; // Relleno seguro si la paleta cargada es corta
        }
    }
    inicializarPaleta();
    
    if (alCambiarColorCallback) {
        alCambiarColorCallback(colorSeleccionadoIndice, coloresLUT[colorSeleccionadoIndice], true);
    }
}

// ---- CONVERSIÓN INVERSA: RGB565 A CSS HEX (#RRGGBB) ----
function decodificarRGB565aHex(rgb565Int) {
    const r5 = (rgb565Int >> 11) & 0x1F;
    const g6 = (rgb565Int >> 5)  & 0x3F;
    const b5 = (rgb565Int)       & 0x1F;

    const r8 = Math.round((r5 * 255) / 31);
    const g8 = Math.round((g6 * 255) / 63);
    const b8 = Math.round((b5 * 255) / 31);

    return "#" + ((1 << 24) + (r8 << 16) + (g8 << 8) + b8).toString(16).slice(1);
}

// ---- MANEJADOR DE ARCHIVOS DE PALETA MODIFICADO ----
document.getElementById("upload-pal")?.addEventListener("change", function(evento) {
    const archivo = evento.target.files[0];
    if (!archivo) return;

    const lector = new FileReader();
    lector.onload = function(e) {
        const contenido = e.target.result;
        const lineas = contenido.split(/\r?\n/).map(l => l.trim()).filter(l => l.length > 0);
        
        const nuevosColoresHex = [];

        // Procesar hasta 16 elementos de la paleta
        for (let i = 0; i < 16; i++) {
            if (i < lineas.length) {
                const tokenHex = lineas[i];
                const valorInt = parseInt(tokenHex, 16);
                
                if (!isNaN(valorInt)) {
                    const colorCssHex = decodificarRGB565aHex(valorInt);
                    nuevosColoresHex.push(colorCssHex);
                } else {
                    console.warn(`Línea ${i + 1} inválida: '${tokenHex}'. Usando respaldo negro.`);
                    nuevosColoresHex.push("#000000");
                }
            } else {
                // Relleno para mantener el tamaño estricto de 16 entradas de hardware
                nuevosColoresHex.push("#000000");
            }
        }

        if (nuevosColoresHex.length > 0) {
            actualizarPaletaDesdeArray(nuevosColoresHex);
        }
    };

    lector.readAsText(archivo);
    evento.target.value = ""; 
});