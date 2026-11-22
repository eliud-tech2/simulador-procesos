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

/* Puente: Crea un arreglo de punteros a partir de la Lista Enlazada */
Proceso** convertir_lista_a_arreglo(ListaProcesos* lista);

/* Heapsort: Unico algoritmo de ordenamiento del sistema O(n log n) */
void heapsort_procesos(Proceso** arreglo, int total, CriterioOrden criterio);
void ordenar_procesos(Proceso** arreglo, int total, CriterioOrden criterio);

#endif
