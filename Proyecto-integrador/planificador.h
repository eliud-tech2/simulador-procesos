#ifndef PLANIFICADOR_H_INCLUDED
#define PLANIFICADOR_H_INCLUDED
#include "estructuras.h"

/* Ejecuta un ciclo de despacho asignando el CPU al proceso en turno */
void ejecutar_round_robin(ColaListos* cola, int quantum);

#endif
