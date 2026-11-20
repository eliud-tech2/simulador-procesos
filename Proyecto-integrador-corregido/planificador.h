#ifndef PLANIFICADOR_H_INCLUDED
#define PLANIFICADOR_H_INCLUDED
#include "estructuras.h"
#include "generador.h"

/* Planificador de Colas Múltiples con Prioridad (0=Alta, 1=Media, 2=Baja) + Round Robin */
void ejecutar_planificador_multicola(ColaListos colas[NUM_COLAS_PRIORIDAD], int quantum,
                                     Proceso** proceso_en_cpu, int ciclo_actual);

/* Envejecimiento (Aging): sube progresivamente de cola a procesos que llevan muchos ciclos esperando */
void aplicar_envejecimiento(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                            int ciclos_para_subir);

/* Evalúa memoria total (RAM + Swap) y dispara el OOM Killer si supera nivel crítico */
int verificar_memoria_y_oom(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                            Proceso** proceso_en_cpu, int ciclo_actual,
                            float* out_ram_gb, float* out_swp_gb,
                            char* mensaje_oom, int tam_mensaje);

#endif
