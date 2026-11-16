#ifndef PLANIFICADOR_H_INCLUDED
#define PLANIFICADOR_H_INCLUDED
#include "estructuras.h"

/* Algoritmos de planificación soportados en tiempo real */
typedef enum {
    ALGORITMO_RR,
    ALGORITMO_PRIORIDAD,
    ALGORITMO_SJF
} AlgoritmoPlanificacion;

/* Ejecuta un ciclo de despacho asignando el CPU al proceso en turno */
void ejecutar_round_robin(ColaListos* cola, int quantum);

/* Ejecuta un ciclo del planificador según el algoritmo activo (RR, Prioridad o SJF) */
void ejecutar_planificador(ColaListos* cola, AlgoritmoPlanificacion algoritmo, int quantum, Proceso** proceso_en_cpu);

#endif
