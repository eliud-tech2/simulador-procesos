#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busqueda.h"

static const char* nombre_estado_texto(EstadoProceso estado) {
    switch (estado) {
        case LISTO: return "LISTO";
        case EJECUCION: return "EJECUCION";
        case ESPERA: return "ESPERA";
        case TERMINADO: return "TERMINADO";
        default: return "DESCONOCIDO";
    }
}

/* ==========================================================================
 * 1. BÚSQUEDA SECUENCIAL (Itera la lista general uno por uno)
 * ========================================================================== */
void busqueda_secuencial_estado(ListaProcesos* lista, EstadoProceso estado_buscado) {
    if (lista == NULL || lista->cabeza == NULL) return;

    printf("\n--- Procesos con el estado %s (%d) ---\n", nombre_estado_texto(estado_buscado), estado_buscado);
    Nodo* actual = lista->cabeza;
    int encontrados = 0;

    while (actual != NULL) {
        if (actual->proceso->estado == estado_buscado) {
            printf("- PID: %d | Nombre: %-15s | CPU: %5.1f%% | RAM: %5.1f%% | Ciclos: %d/%d\n",
                   actual->proceso->pid, actual->proceso->nombre,
                   actual->proceso->cpu_uso, actual->proceso->mem_uso,
                   actual->proceso->tiempo_ejecucion, actual->proceso->tiempo_rafaga);
            encontrados++;
        }
        actual = actual->siguiente; /* Salta al siguiente proceso de la lista */
    }

    if (encontrados == 0) {
        printf("No se encontraron procesos en ese estado.\n");
    } else {
        printf("\nTotal encontrados: %d\n", encontrados);
    }
}

/* Búsqueda secuencial por PID en la lista enlazada */
Proceso* busqueda_secuencial_pid(ListaProcesos* lista, int pid_buscado) {
    if (lista == NULL) return NULL;
    Nodo* actual = lista->cabeza;
    while (actual != NULL) {
        if (actual->proceso != NULL && actual->proceso->pid == pid_buscado) {
            return actual->proceso;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

/* Búsqueda secuencial por Nombre en la lista enlazada */
void busqueda_secuencial_nombre(ListaProcesos* lista, const char* nombre_buscado) {
    if (lista == NULL || lista->cabeza == NULL || nombre_buscado == NULL) return;

    printf("\n--- Búsqueda secuencial para '%s' ---\n", nombre_buscado);
    Nodo* actual = lista->cabeza;
    int encontrados = 0;

    while (actual != NULL) {
        if (strcmp(actual->proceso->nombre, nombre_buscado) == 0) {
            printf("- PID: %d | RAM: %.1f%% | Estado: %s\n",
                   actual->proceso->pid,
                   actual->proceso->mem_uso, nombre_estado_texto(actual->proceso->estado));
            encontrados++;
        }
        actual = actual->siguiente;
    }

    if (encontrados == 0) {
        printf("No se encontraron procesos llamados '%s'.\n", nombre_buscado);
    }
}

/* ==========================================================================
 * 2. BÚSQUEDA BINARIA (Divide a la mitad buscando PIDs en un arreglo ordenado)
 * ========================================================================== */
Proceso* busqueda_binaria_pid(Proceso** arreglo, int total, int pid_buscado) {
    if (arreglo == NULL || total <= 0) return NULL;
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
    if (th == NULL) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        th->tabla[i] = NULL;
    }
}

/* Algoritmo de dispersión matemático (djb2): Convierte texto a un número de índice */
unsigned int calcular_hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % HASH_SIZE;
}

/* Inserta en la Tabla. Si hay colisión, encadena el nuevo dato al inicio */
void insertar_hash(TablaHash* th, Proceso* p) {
    if (th == NULL || p == NULL) return;

    unsigned int indice = calcular_hash(p->nombre);

    NodoHash* nuevo_nodo = (NodoHash*)malloc(sizeof(NodoHash));
    if (nuevo_nodo == NULL) return;
    nuevo_nodo->proceso = p;

    /* Conectamos el nuevo nodo a lo que sea que ya estuviera en ese indice (Encadenamiento) */
    nuevo_nodo->siguiente = th->tabla[indice];
    th->tabla[indice] = nuevo_nodo;
}

/* Busca directamente en el índice y recorre las colisiones si las hay */
void buscar_hash_nombre(TablaHash* th, const char* nombre_buscado) {
    if (th == NULL || nombre_buscado == NULL) return;
    unsigned int indice = calcular_hash(nombre_buscado);
    NodoHash* actual = th->tabla[indice];
    int encontrados = 0;

    printf("\n--- Resultados Hash para el proceso '%s' (Indice #%u) ---\n", nombre_buscado, indice);
    while (actual != NULL) {
        if (strcmp(actual->proceso->nombre, nombre_buscado) == 0) {
            printf("- PID: %d | CPU: %.1f%% | RAM: %.1f%% | Estado: %s\n",
                   actual->proceso->pid,
                   actual->proceso->cpu_uso, actual->proceso->mem_uso,
                   nombre_estado_texto(actual->proceso->estado));
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

/* Libera todos los nodos de la tabla hash */
void liberar_tabla_hash(TablaHash* th) {
    if (th == NULL) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        NodoHash* actual = th->tabla[i];
        while (actual != NULL) {
            NodoHash* sig = actual->siguiente;
            free(actual);
            actual = sig;
        }
        th->tabla[i] = NULL;
    }
}
