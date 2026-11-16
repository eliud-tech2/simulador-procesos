#ifndef ESTRUCTURAS_H_INCLUDED
#define ESTRUCTURAS_H_INCLUDED
#include "proceso.h" /* Importamos el TDA Proceso porque lo vamos a guardar aquí */

/* --------------------------------------------------------------------------
 * 1. NODO BÁSICO (Para la Lista Enlazada y la Cola FIFO)
 * -------------------------------------------------------------------------- */
typedef struct Nodo {
    Proceso* proceso;     // Puntero al proceso que guarda este nodo
    struct Nodo* siguiente; // Puntero al siguiente nodo en la fila
} Nodo;

/* --------------------------------------------------------------------------
 * 2. LISTA ENLAZADA (Registro histórico de todos los procesos)
 * -------------------------------------------------------------------------- */
typedef struct {
    Nodo* cabeza; // Apunta al primer elemento de la lista
    int tamano;   // Cuántos procesos hay registrados en total
} ListaProcesos;

/* --------------------------------------------------------------------------
 * 3. COLA FIFO (Para el planificador Round Robin)
 * -------------------------------------------------------------------------- */
typedef struct {
    Nodo* frente; // El proceso al que le toca usar el CPU
    Nodo* final;  // El último proceso que llegó a la fila
    int tamano;
} ColaListos;

/* --------------------------------------------------------------------------
 * 4. NODO Y ÁRBOL BINARIO DE BÚSQUEDA (Para buscar rápido por PID)
 * -------------------------------------------------------------------------- */
typedef struct NodoArbol {
    Proceso* proceso;
    struct NodoArbol* izquierda; // PIDs menores
    struct NodoArbol* derecha;   // PIDs mayores
} NodoArbol;

typedef struct {
    NodoArbol* raiz;
} ArbolPID;

/* --------------------------------------------------------------------------
 * PROTOTIPOS DE FUNCIONES (Las programaremos después en estructuras.c)
 * -------------------------------------------------------------------------- */
/* Funciones de la Lista */
void inicializar_lista(ListaProcesos* lista);
void insertar_lista(ListaProcesos* lista, Proceso* p);
void liberar_lista(ListaProcesos* lista, int destruir_procesos);

/* Funciones de la Cola Round Robin */
void inicializar_cola(ColaListos* cola);
void encolar(ColaListos* cola, Proceso* p);
Proceso* desencolar(ColaListos* cola);
int esta_en_cola(ColaListos* cola, int pid);
void eliminar_de_cola(ColaListos* cola, int pid);
void liberar_cola(ColaListos* cola);

/* Funciones del Arbol Binario de Busqueda (Por PID) */
void inicializar_arbol(ArbolPID* arbol);
void insertar_arbol(ArbolPID* arbol, Proceso* p);
Proceso* buscar_arbol_pid(NodoArbol* raiz, int pid_buscado);
void liberar_arbol(NodoArbol* raiz);

#endif
