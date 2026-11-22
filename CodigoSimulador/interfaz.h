#ifndef INTERFAZ_H_INCLUDED
#define INTERFAZ_H_INCLUDED
#include "estructuras.h"
#include "generador.h"
#include "ordenamiento.h"

/* Dibuja la interfaz estilo htop con Colas Múltiples, Swap real, OOM Killer y Heapsort */
void dibujar_interfaz_htop(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                           int ciclo, CriterioOrden crit_orden, int quantum,
                           float ram_gb, float swp_gb, int admision_abierta,
                           const char* mensaje_oom);

#endif
