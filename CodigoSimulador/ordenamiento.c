#include <stdio.h>
#include <stdlib.h>
#include "ordenamiento.h"

/* ==========================================================================
 * FUNCIÓN PUENTE: De Lista Enlazada a Arreglo Dinámico
 * ========================================================================== */
Proceso** convertir_lista_a_arreglo(ListaProcesos* lista) {
    if (lista == NULL || lista->tamano == 0) return NULL;

    /* Reservamos espacio para guardar únicamente las direcciones (punteros) */
    Proceso** arreglo = (Proceso**)malloc(lista->tamano * sizeof(Proceso*));
    if (arreglo == NULL) return NULL;

    Nodo* actual = lista->cabeza;
    for (int i = 0; i < lista->tamano && actual != NULL; i++) {
        arreglo[i] = actual->proceso;
        actual = actual->siguiente;
    }

    return arreglo;
}

/* ==========================================================================
 * HEAPSORT (Ordenamiento mediante Montículo Binario - O(n log n))
 * ========================================================================== */

/* Función interna para intercambiar dos procesos de lugar en el arreglo */
static void intercambiar(Proceso** a, Proceso** b) {
    Proceso* temporal = *a;
    *a = *b;
    *b = temporal;
}

/* Retorna 1 si 'a' debe quedar DESPUÉS que 'b' (en el fondo del arreglo al extraer la cima) */
static int debe_ir_despues(Proceso* a, Proceso* b, CriterioOrden criterio) {
    switch (criterio) {
        case CRITERIO_MEMORIA:
            return a->mem_uso < b->mem_uso; /* Para ordenar de Mayor a Menor */
        case CRITERIO_CPU:
            return a->cpu_uso < b->cpu_uso; /* Para ordenar de Mayor a Menor */
        case CRITERIO_TIEMPO:
            return a->tiempo_ejecucion < b->tiempo_ejecucion; /* Mayor a Menor */
        case CRITERIO_PID:
            return a->pid > b->pid; /* Para ordenar PID de Menor a Mayor */
        default:
            return a->mem_uso < b->mem_uso;
    }
}

/* Reorganiza un subárbol con raíz en 'i' para mantener la propiedad del Heap */
static void heapify(Proceso** arreglo, int n, int i, CriterioOrden criterio) {
    int extremo = i;          /* Inicializamos el extremo como la raíz */
    int izq = 2 * i + 1;      /* Hijo izquierdo en el arreglo */
    int der = 2 * i + 2;      /* Hijo derecho en el arreglo */

    /* Si el hijo izquierdo debe ir al final antes que la raíz actual */
    if (izq < n && debe_ir_despues(arreglo[izq], arreglo[extremo], criterio)) {
        extremo = izq;
    }

    /* Si el hijo derecho debe ir al final antes que el extremo actual */
    if (der < n && debe_ir_despues(arreglo[der], arreglo[extremo], criterio)) {
        extremo = der;
    }

    /* Si el extremo no es la raíz, intercambiamos y hundimos recursivamente */
    if (extremo != i) {
        intercambiar(&arreglo[i], &arreglo[extremo]);
        heapify(arreglo, n, extremo, criterio);
    }
}

/* Algoritmo principal Heapsort */
void heapsort_procesos(Proceso** arreglo, int total, CriterioOrden criterio) {
    if (arreglo == NULL || total <= 1) return;

    /* 1. Construir el montículo (reorganizar el arreglo desde la mitad hacia arriba) */
    for (int i = total / 2 - 1; i >= 0; i--) {
        heapify(arreglo, total, i, criterio);
    }

    /* 2. Extraer uno por uno los elementos de la cima del montículo hacia el final */
    for (int i = total - 1; i > 0; i--) {
        intercambiar(&arreglo[0], &arreglo[i]);
        heapify(arreglo, i, 0, criterio);
    }
}

void ordenar_procesos(Proceso** arreglo, int total, CriterioOrden criterio) {
    heapsort_procesos(arreglo, total, criterio);
}
