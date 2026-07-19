import { coloresLUT } from './palette.js';

const GRID_SIZE = 8;
export const TOTAL_PIXELS = GRID_SIZE * GRID_SIZE;

export class CanvasEditor {
    constructor(canvasGridElement) {
        this.canvasGrid = canvasGridElement;
        this.matrizSprite = new Array(TOTAL_PIXELS).fill(0);
    }

    /**
     * Dibuja los botones en el DOM y les asigna sus IDs físicos.
     * Recibe los callbacks desde fuera para que app.js maneje los eventos del mouse.
     */
    inicializarCuadrigrid(onMouseDown, onMouseEnter) {
        this.canvasGrid.innerHTML = ""; 

        for (let i = 0; i < TOTAL_PIXELS; i++) {
            const pixel = document.createElement("button");
            pixel.type = "button";
            pixel.classList.add("pixel");
            pixel.dataset.pixelId = i;
            
            // Calcular coordenadas X, Y puras para lógica de mapas/hojas si se requiere
            const x = i % GRID_SIZE;
            const y = Math.floor(i / GRID_SIZE);
            pixel.dataset.x = x;
            pixel.dataset.y = y;

            // Pintar con el estado inicial de la matriz
            pixel.style.backgroundColor = coloresLUT[this.matrizSprite[i]];

            // Registrar los eventos pasando el elemento, el índice lineal y sus coordenadas
            pixel.addEventListener("mousedown", (e) => onMouseDown(e, pixel, i, x, y));
            pixel.addEventListener("mouseenter", () => onMouseEnter(pixel, i, x, y));

            this.canvasGrid.appendChild(pixel);
        }
    }

    /**
     * Altera el color y el índice de un píxel específico
     */
    actualizarPixel(indiceMatriz, indiceColor) {
        this.matrizSprite[indiceMatriz] = indiceColor;
        
        const pixelDOM = this.canvasGrid.querySelector(`[data-pixel-id="${indiceMatriz}"]`);
        if (pixelDOM) {
            pixelDOM.style.backgroundColor = coloresLUT[indiceColor];
        }
    }

    /**
     * Refresca la pantalla completa si cambia la paleta de colores física (LUT ROM)
     */
    refrescarColoresPorCambioPaleta() {
        const pixelesDOM = this.canvasGrid.querySelectorAll(".pixel");
        pixelesDOM.forEach((pixel, i) => {
            pixel.style.backgroundColor = coloresLUT[this.matrizSprite[i]];
        });
    }

    // Exportar e importar el búfer crudo
    getData() {
        return [...this.matrizSprite];
    }

    setData(nuevaMatriz) {
        if (nuevaMatriz.length === TOTAL_PIXELS) {
            this.matrizSprite = [...nuevaMatriz];
            this.refrescarColoresPorCambioPaleta();
        }
    }
}