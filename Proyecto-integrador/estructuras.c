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

    Nodo* nuevo_nodo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo_nodo == NULL) {
        printf("Error: No hay memoria para insertar en la lista.\n");
        return;
    }
    nuevo_nodo->proceso = p;
    nuevo_nodo->siguiente = NULL;

    if (lista->cabeza == NULL) {
        lista->cabeza = nuevo_nodo;
    } else {
        Nodo* actual = lista->cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
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
        cola->frente = nuevo_nodo;
        cola->final = nuevo_nodo;
    } else {
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

    Nodo* nodo_salida = cola->frente;
    Proceso* p_salida = nodo_salida->proceso;

    cola->frente = cola->frente->siguiente;

    if (cola->frente == NULL) {
        cola->final = NULL;
    }

    cola->tamano--;

    free(nodo_salida);

    return p_salida;
} // ESTA ES LA LLAVE QUE FALTABA

/* ==========================================================================
 * FUNCIONES DEL ÁRBOL BINARIO DE BÚSQUEDA (BST)
 * ========================================================================== */

void inicializar_arbol(ArbolPID* arbol) {
    if (arbol != NULL) {
        arbol->raiz = NULL;
    }
}

/* Funcion recursiva oculta para navegar las ramas y encontrar el lugar vacio */
static NodoArbol* insertar_nodo_arbol(NodoArbol* nodo, Proceso* p) {
    if (nodo == NULL) {
        NodoArbol* nuevo = (NodoArbol*)malloc(sizeof(NodoArbol));
        nuevo->proceso = p;
        nuevo->izquierda = NULL;
        nuevo->derecha = NULL;
        return nuevo;
    }

    if (p->pid < nodo->proceso->pid) {
        nodo->izquierda = insertar_nodo_arbol(nodo->izquierda, p);
    }
    else if (p->pid > nodo->proceso->pid) {
        nodo->derecha = insertar_nodo_arbol(nodo->derecha, p);
    }

    return nodo;
}

/* Funcion principal que el sistema llama para meter un proceso al arbol */
void insertar_arbol(ArbolPID* arbol, Proceso* p) {
    if (arbol != NULL && p != NULL) {
        arbol->raiz = insertar_nodo_arbol(arbol->raiz, p);
    }
}

/* Busqueda ultrarrapida: descarta mitades enteras del arbol en cada salto */
Proceso* buscar_arbol_pid(NodoArbol* raiz, int pid_buscado) {
    if (raiz == NULL || raiz->proceso->pid == pid_buscado) {
        return (raiz != NULL) ? raiz->proceso : NULL;
    }

    if (pid_buscado < raiz->proceso->pid) {
        return buscar_arbol_pid(raiz->izquierda, pid_buscado);
    }
    else {
        return buscar_arbol_pid(raiz->derecha, pid_buscado);
    }
}
