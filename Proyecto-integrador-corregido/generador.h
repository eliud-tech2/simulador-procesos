#ifndef GENERADOR_H_INCLUDED
#define GENERADOR_H_INCLUDED
#include "estructuras.h" /* Necesita conocer la Lista y la Cola para inyectar los procesos */
#include "busqueda.h"

#define NUM_COLAS_PRIORIDAD 3 /* 0 = Alta, 1 = Media, 2 = Baja */

/* Versión integrada con Colas Múltiples, control de admisión de RAM/Swap y ciclo actual */
void intentar_crear_proceso_sistema(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                                    ArbolPID* arbol, TablaHash* th, int* contador_pid,
                                    int ciclo_actual, int permitir_admision);

/* Inyecta una ráfaga de procesos pesados para demostrar Swap, control de admisión y OOM Killer */
int inyectar_carga_estres(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                          ArbolPID* arbol, TablaHash* th, int* contador_pid,
                          int ciclo_actual, int cantidad);

#endif
