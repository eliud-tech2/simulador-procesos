#include <stdio.h>
#include <stdlib.h>
#include "estructuras.h"

/* ==========================================================================
 * FUNCIONES DE LA LISTA ENLAZADA (Registro General)
 * ========================================================================== */

/* Prepara la lista vacía para empezar a recibir procesos */
void inicializar_lista(ListaProcesos* lista) {
    if (lista != NULL) {
        lista->cabeza = NULL;
        lista->tamano = 0;
    }
}

/* Inserta un proceso al final de la lista general */
void insertar_lista(ListaProcesos* lista, Proceso* p) {
    if (lista == NULL || p == NULL) return;

    // 1. Creamos la caja (Nodo) para guardar el proceso
    Nodo* nuevo_nodo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo_nodo == NULL) {
        printf("Error: No hay memoria para insertar en la lista.\n");
        return;
    }
    nuevo_nodo->proceso = p;
    nuevo_nodo->siguiente = NULL;

    // 2. Si la lista está vacía, este es el primer elemento
    if (lista->cabeza == NULL) {
        lista->cabeza = nuevo_nodo;
    } else {
        // 3. Si ya hay elementos, recorremos la lista hasta llegar al final
        Nodo* actual = lista->cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        // Lo conectamos al final
        actual->siguiente = nuevo_nodo;
    }

    lista->tamano++;
}


/* ==========================================================================
 * FUNCIONES DE LA COLA FIFO (Para Planificador Round Robin)
 * ========================================================================== */

/* Prepara la cola vacía */
void inicializar_cola(ColaListos* cola) {
    if (cola != NULL) {
        cola->frente = NULL;
        cola->final = NULL;
        cola->tamano = 0;
    }
}

/* Mete un proceso al FINAL de la fila (Encolar) */
void encolar(ColaListos* cola, Proceso* p) {
    if (cola == NULL || p == NULL) return;

    Nodo* nuevo_nodo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo_nodo == NULL) {
        printf("Error: No hay memoria para encolar.\n");
        return;
    }
    nuevo_nodo->proceso = p;
    nuevo_nodo->siguiente = NULL;

    if (cola->final == NULL) {
        // Si la cola estaba vacía, el frente y el final son el mismo
        cola->frente = nuevo_nodo;
        cola->final = nuevo_nodo;
    } else {
        // Si ya hay gente en la fila, lo ponemos detrás del último
        cola->final->siguiente = nuevo_nodo;
        cola->final = nuevo_nodo;
    }

    cola->tamano++;
}

/* Saca al proceso que está al FRENTE de la fila (Desencolar) */
Proceso* desencolar(ColaListos* cola) {
    if (cola == NULL || cola->frente == NULL) {
        return NULL; // La cola está vacía
    }

    // 1. Agarramos el nodo que está al frente
    Nodo* nodo_salida = cola->frente;
    Proceso* p_salida = nodo_salida->proceso;

    // 2. Movemos la fila: el nuevo frente es el que seguía
    cola->frente = cola->frente->siguiente;

    // 3. Si al sacar a este, la fila se quedó vacía, el final también es NULL
    if (cola->frente == NULL) {
        cola->final = NULL;
    }

    cola->tamano--;

    // 4. Liberamos la "caja" vacía (pero no el proceso, ese se va al CPU)
    free(nodo_salida);

    return p_salida;
}
