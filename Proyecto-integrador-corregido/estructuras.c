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

/* Libera todos los nodos de la lista y opcionalmente destruye los procesos */
void liberar_lista(ListaProcesos* lista, int destruir_procesos) {
    if (lista == NULL) return;
    Nodo* actual = lista->cabeza;
    while (actual != NULL) {
        Nodo* siguiente = actual->siguiente;
        if (destruir_procesos && actual->proceso != NULL) {
            destruir_proceso(actual->proceso);
        }
        free(actual);
        actual = siguiente;
    }
    lista->cabeza = NULL;
    lista->tamano = 0;
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

    return p_salida; // ESTA ES LA LLAVE QUE FALTABA
}

/* Verifica si un PID ya se encuentra en la cola */
int esta_en_cola(ColaListos* cola, int pid) {
    if (cola == NULL) return 0;
    Nodo* actual = cola->frente;
    while (actual != NULL) {
        if (actual->proceso != NULL && actual->proceso->pid == pid) {
            return 1;
        }
        actual = actual->siguiente;
    }
    return 0;
}

/* Elimina un proceso específico de la cola por su PID */
void eliminar_de_cola(ColaListos* cola, int pid) {
    if (cola == NULL || cola->frente == NULL) return;

    Nodo* actual = cola->frente;
    Nodo* anterior = NULL;

    while (actual != NULL) {
        if (actual->proceso != NULL && actual->proceso->pid == pid) {
            if (anterior == NULL) {
                cola->frente = actual->siguiente;
                if (cola->frente == NULL) {
                    cola->final = NULL;
                }
            } else {
                anterior->siguiente = actual->siguiente;
                if (actual == cola->final) {
                    cola->final = anterior;
                }
            }
            free(actual);
            cola->tamano--;
            return;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
}

/* Libera todos los nodos de la cola FIFO */
void liberar_cola(ColaListos* cola) {
    if (cola == NULL) return;
    while (cola->frente != NULL) {
        desencolar(cola);
    }
}

/* ==========================================================================
 * FUNCIONES DE LA COLA DE PRIORIDAD (Mayor prioridad al frente)
 * ========================================================================== */

void inicializar_cola_prioridad(ColaPrioridad* cp) {
    if (cp != NULL) {
        cp->frente = NULL;
        cp->tamano = 0;
    }
}

/* Inserta ordenado de MAYOR a MENOR prioridad (0=Baja .. 3=Crítica) */
void encolar_prioridad(ColaPrioridad* cp, Proceso* p) {
    if (cp == NULL || p == NULL) return;

    Nodo* nuevo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo == NULL) {
        printf("Error: No hay memoria para encolar en prioridad.\n");
        return;
    }
    nuevo->proceso = p;
    nuevo->siguiente = NULL;

    if (cp->frente == NULL || p->prioridad > cp->frente->proceso->prioridad) {
        nuevo->siguiente = cp->frente;
        cp->frente = nuevo;
    } else {
        Nodo* actual = cp->frente;
        while (actual->siguiente != NULL && actual->siguiente->proceso->prioridad >= p->prioridad) {
            actual = actual->siguiente;
        }
        nuevo->siguiente = actual->siguiente;
        actual->siguiente = nuevo;
    }
    cp->tamano++;
}

/* Extrae el proceso con mayor prioridad */
Proceso* desencolar_prioridad(ColaPrioridad* cp) {
    if (cp == NULL || cp->frente == NULL) return NULL;
    Nodo* nodo_salida = cp->frente;
    Proceso* p = nodo_salida->proceso;
    cp->frente = cp->frente->siguiente;
    cp->tamano--;
    free(nodo_salida);
    return p;
}

void liberar_cola_prioridad(ColaPrioridad* cp) {
    if (cp == NULL) return;
    while (cp->frente != NULL) {
        desencolar_prioridad(cp);
    }
}

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
        if (nuevo == NULL) return NULL;
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

/* Libera los nodos del árbol en postorden */
void liberar_arbol(NodoArbol* raiz) {
    if (raiz == NULL) return;
    liberar_arbol(raiz->izquierda);
    liberar_arbol(raiz->derecha);
    free(raiz);
}
