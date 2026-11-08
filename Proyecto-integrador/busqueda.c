#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busqueda.h"

/* ==========================================================================
 * 1. BÚSQUEDA SECUENCIAL (Itera la lista general uno por uno)
 * ========================================================================== */
void busqueda_secuencial_estado(ListaProcesos* lista, EstadoProceso estado_buscado) {
    if (lista == NULL || lista->cabeza == NULL) return;

    printf("\n--- Procesos con el estado %d ---\n", estado_buscado);
    Nodo* actual = lista->cabeza;
    int encontrados = 0;

    while (actual != NULL) {
        if (actual->proceso->estado == estado_buscado) {
            printf("- PID: %d | Nombre: %s\n", actual->proceso->pid, actual->proceso->nombre);
            encontrados++;
        }
        actual = actual->siguiente; /* Salta al siguiente proceso de la lista */
    }

    if (encontrados == 0) {
        printf("No se encontraron procesos en ese estado.\n");
    }
}

/* ==========================================================================
 * 2. BÚSQUEDA BINARIA (Divide a la mitad buscando PIDs en un arreglo ordenado)
 * ========================================================================== */
Proceso* busqueda_binaria_pid(Proceso** arreglo, int total, int pid_buscado) {
    int izquierda = 0;
    int derecha = total - 1;

    while (izquierda <= derecha) {
        int medio = izquierda + (derecha - izquierda) / 2;

        if (arreglo[medio]->pid == pid_buscado) {
            return arreglo[medio]; /* Lo encontro, corta la busqueda */
        }
        if (arreglo[medio]->pid < pid_buscado) {
            izquierda = medio + 1; /* Descarta la mitad inferior */
        } else {
            derecha = medio - 1;   /* Descarta la mitad superior */
        }
    }
    return NULL; /* El PID no existe */
}

/* ==========================================================================
 * 3. BÚSQUEDA POR HASH CON RESOLUCIÓN DE COLISIONES
 * ========================================================================== */
void inicializar_tabla_hash(TablaHash* th) {
    for (int i = 0; i < HASH_SIZE; i++) {
        th->tabla[i] = NULL;
    }
}

/* Algoritmo de dispersión matemático (djb2): Convierte texto a un número de índice */
unsigned int calcular_hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % HASH_SIZE;
}

/* Inserta en la Tabla. Si hay colisión, encadena el nuevo dato al inicio */
void insertar_hash(TablaHash* th, Proceso* p) {
    if (th == NULL || p == NULL) return;

    unsigned int indice = calcular_hash(p->nombre);

    NodoHash* nuevo_nodo = (NodoHash*)malloc(sizeof(NodoHash));
    nuevo_nodo->proceso = p;

    /* Conectamos el nuevo nodo a lo que sea que ya estuviera en ese indice (Encadenamiento) */
    nuevo_nodo->siguiente = th->tabla[indice];
    th->tabla[indice] = nuevo_nodo;
}

/* Busca directamente en el índice y recorre las colisiones si las hay */
void buscar_hash_nombre(TablaHash* th, const char* nombre_buscado) {
    unsigned int indice = calcular_hash(nombre_buscado);
    NodoHash* actual = th->tabla[indice];
    int encontrados = 0;

    printf("\n--- Resultados Hash para el proceso '%s' ---\n", nombre_buscado);
    while (actual != NULL) {
        if (strcmp(actual->proceso->nombre, nombre_buscado) == 0) {
            printf("- PID: %d | RAM: %.1f%% | Estado: %d\n",
                   actual->proceso->pid, actual->proceso->mem_uso, actual->proceso->estado);
            encontrados++;
        }
        actual = actual->siguiente;
    }

    if (encontrados == 0) {
        printf("No hay procesos activos llamados '%s'.\n", nombre_buscado);
    } else {
        printf("Instancias totales encontradas: %d\n", encontrados);
    }
}
