#ifndef PLANIFICADOR_H_INCLUDED
#define PLANIFICADOR_H_INCLUDED
#include "estructuras.h"

/* Ejecuta un ciclo de despacho asignando el CPU al proceso en turno */
void ejecutar_round_robin(ColaListos* cola, int quantum);

/* Ejecuta un ciclo de Round Robin manteniendo visible el estado EJECUCION */
void ejecutar_planificador_rr(ColaListos* cola, int quantum, Proceso** proceso_en_cpu);

#endif
