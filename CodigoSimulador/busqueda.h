#ifndef BUSQUEDA_H_INCLUDED
#define BUSQUEDA_H_INCLUDED
#include "estructuras.h"

/* --- ESTRUCTURAS PARA LA TABLA HASH --- */
#define HASH_SIZE 97 // Número primo para mejor distribución matemática

typedef struct NodoHash {
    Proceso* proceso;
    struct NodoHash* siguiente; // Puntero clave para manejar colisiones (varios "spotify")
} NodoHash;

typedef struct {
    NodoHash* tabla[HASH_SIZE];
} TablaHash;

/* --- PROTOTIPOS DE LAS FUNCIONES DE BÚSQUEDA --- */

/* 1. Secuencial: Recorre la lista buscando atributos repetidos (ej. Estado) */
void busqueda_secuencial_estado(ListaProcesos* lista, EstadoProceso estado_buscado);
Proceso* busqueda_secuencial_pid(ListaProcesos* lista, int pid_buscado);
void busqueda_secuencial_nombre(ListaProcesos* lista, const char* nombre_buscado);

/* 2. Binaria: Encuentra UN proceso único por PID (requiere un arreglo ordenado) */
Proceso* busqueda_binaria_pid(Proceso** arreglo_procesos, int total, int pid_buscado);

/* 3. Hash: Funciones para administrar y buscar nombres a velocidad O(1) */
void inicializar_tabla_hash(TablaHash* th);
void insertar_hash(TablaHash* th, Proceso* p);
void buscar_hash_nombre(TablaHash* th, const char* nombre_buscado);
void liberar_tabla_hash(TablaHash* th);

#endif
