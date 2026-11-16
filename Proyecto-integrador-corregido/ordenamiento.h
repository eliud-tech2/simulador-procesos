#ifndef ORDENAMIENTO_H_INCLUDED
#define ORDENAMIENTO_H_INCLUDED
#include "estructuras.h"

/* Criterios de ordenamiento soportados por el simulador */
typedef enum {
    CRITERIO_MEMORIA,
    CRITERIO_CPU,
    CRITERIO_TIEMPO,
    CRITERIO_PID
} CriterioOrden;

/* Algoritmos de ordenamiento elegidos en el alcance (Quicksort y Radix) */
typedef enum {
    ORDEN_QUICKSORT,
    ORDEN_RADIX
} AlgoritmoOrden;

/* Puente: Crea un arreglo de punteros a partir de la Lista Enlazada */
Proceso** convertir_lista_a_arreglo(ListaProcesos* lista);

/* 1. Quicksort: Ordena de MAYOR a MENOR uso de Memoria RAM */
void quicksort_por_memoria(Proceso** arreglo, int izquierda, int derecha);

/* 2. Radix Sort: Ordena de MENOR a MAYOR por numero de PID */
void radix_sort_por_pid(Proceso** arreglo, int total);

/* Funciones para ordenar por cualquier criterio con Quicksort y Radix */
void quicksort_procesos(Proceso** arreglo, int izquierda, int derecha, CriterioOrden criterio);
void radix_sort_procesos(Proceso** arreglo, int total, CriterioOrden criterio);
void ordenar_procesos(Proceso** arreglo, int total, AlgoritmoOrden algoritmo, CriterioOrden criterio);

#endif
