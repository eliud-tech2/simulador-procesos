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
 * 1. QUICKSORT (Ordena de Mayor a Menor por Memoria RAM)
 * ========================================================================== */
/* Función interna para intercambiar dos procesos de lugar en el arreglo */
static void intercambiar(Proceso** a, Proceso** b) {
    Proceso* temporal = *a;
    *a = *b;
    *b = temporal;
}

static int particion_memoria(Proceso** arreglo, int izquierda, int derecha) {
    float pivote = arreglo[derecha]->mem_uso; /* Tomamos el último como referencia */
    int i = (izquierda - 1);

    for (int j = izquierda; j < derecha; j++) {
        /* Si el proceso actual gasta MÁS o IGUAL memoria que el pivote, va a la izquierda */
        if (arreglo[j]->mem_uso >= pivote) {
            i++;
            intercambiar(&arreglo[i], &arreglo[j]);
        }
    }
    intercambiar(&arreglo[i + 1], &arreglo[derecha]);
    return (i + 1);
}

void quicksort_por_memoria(Proceso** arreglo, int izquierda, int derecha) {
    if (arreglo == NULL) return;
    if (izquierda < derecha) {
        int indice_pivote = particion_memoria(arreglo, izquierda, derecha);
        /* Ordenamos recursivamente la mitad izquierda y la mitad derecha */
        quicksort_por_memoria(arreglo, izquierda, indice_pivote - 1);
        quicksort_por_memoria(arreglo, indice_pivote + 1, derecha);
    }
}

/* ==========================================================================
 * 2. RADIX SORT (Ordena de Menor a Mayor por PID)
 * ========================================================================== */
/* Obtiene el PID más grande para saber cuántos dígitos debemos revisar */
static int obtener_maximo_pid(Proceso** arreglo, int total) {
    int max = arreglo[0]->pid;
    for (int i = 1; i < total; i++) {
        if (arreglo[i]->pid > max) {
            max = arreglo[i]->pid;
        }
    }
    return max;
}

/* Ordena los procesos según el dígito actual (exp = 1 unidades, 10 decenas, 100 centenas) */
static void counting_sort_digito(Proceso** arreglo, int total, int exp) {
    Proceso** salida = (Proceso**)malloc(total * sizeof(Proceso*));
    int cubetas[10] = {0};

    /* 1. Contamos cuántos PIDs caen en cada cubeta del 0 al 9 */
    for (int i = 0; i < total; i++) {
        int digito = (arreglo[i]->pid / exp) % 10;
        cubetas[digito]++;
    }

    /* 2. Acumulamos las posiciones para saber el lugar exacto en el arreglo de salida */
    for (int i = 1; i < 10; i++) {
        cubetas[i] += cubetas[i - 1];
    }

    /* 3. Acomodamos los procesos en el arreglo temporal de atrás hacia adelante */
    for (int i = total - 1; i >= 0; i--) {
        int digito = (arreglo[i]->pid / exp) % 10;
        salida[cubetas[digito] - 1] = arreglo[i];
        cubetas[digito]--;
    }

    /* 4. Copiamos el resultado ordenado al arreglo original y liberamos la memoria temporal */
    for (int i = 0; i < total; i++) {
        arreglo[i] = salida[i];
    }
    free(salida);
}

void radix_sort_por_pid(Proceso** arreglo, int total) {
    if (arreglo == NULL || total <= 1) return;

    int max_pid = obtener_maximo_pid(arreglo, total);

    /* Pasamos por unidades (1), decenas (10), centenas (100), etc. */
    for (int exp = 1; max_pid / exp > 0; exp *= 10) {
        counting_sort_digito(arreglo, total, exp);
    }
}
