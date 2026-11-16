#include <stdio.h>
#include <stdlib.h>
#include "planificador.h"
#include "proceso.h"

void ejecutar_round_robin(ColaListos* cola, int quantum) {
    if (cola == NULL || cola->frente == NULL) {
        return; // El CPU está inactivo porque no hay fila
    }

    // 1. Despachador: Sacamos al proceso que está al frente de la cola
    Proceso* p_actual = desencolar(cola);

    // 2. Cambio de contexto: Entra al procesador
    p_actual->estado = EJECUCION;

    // Generamos un consumo de CPU dinámico y realista entre 12.5% y 88.4% mientras corre
    p_actual->cpu_uso = (float)(rand() % 760 + 125) / 10.0f;

    // 3. Ejecución: Pasa el tiempo del quantum
    p_actual->tiempo_ejecucion += quantum;

    // 4. Evaluación: ¿Terminó su trabajo?
    if (p_actual->tiempo_ejecucion >= p_actual->tiempo_rafaga) {
        terminar_proceso(p_actual); // Libera CPU y RAM al terminar
    } else {
        // 5. Expropiación: Se acabó el quantum pero no ha terminado. Vuelve a la fila.
        p_actual->estado = LISTO;
        // Al salir de ejecución baja su consumo de CPU a un rango pasivo (0.5% a 3.2%)
        p_actual->cpu_uso = (float)(rand() % 28 + 5) / 10.0f;
        encolar(cola, p_actual);
    }
}

/* Planificador completo: mantiene visible el estado EJECUCION entre ciclos */
void ejecutar_planificador(ColaListos* cola, AlgoritmoPlanificacion algoritmo, int quantum, Proceso** proceso_en_cpu) {
    if (cola == NULL || proceso_en_cpu == NULL) return;

    /* 1. Evaluar el proceso que estuvo en CPU durante el ciclo anterior */
    if (*proceso_en_cpu != NULL) {
        Proceso* p_actual = *proceso_en_cpu;

        /* Si el usuario lo suspendió o terminó manualmente, soltamos el CPU */
        if (p_actual->estado != EJECUCION) {
            *proceso_en_cpu = NULL;
        } else {
            p_actual->tiempo_ejecucion += quantum;

            if (p_actual->tiempo_ejecucion >= p_actual->tiempo_rafaga) {
                terminar_proceso(p_actual);
                *proceso_en_cpu = NULL;
            } else if (algoritmo == ALGORITMO_RR) {
                /* Expropiación en Round Robin al terminar el quantum */
                p_actual->estado = LISTO;
                p_actual->cpu_uso = (float)(rand() % 28 + 5) / 10.0f;
                encolar(cola, p_actual);
                *proceso_en_cpu = NULL;
            } else {
                /* En Prioridad y SJF continúa en CPU hasta terminar o ser suspendido */
                p_actual->cpu_uso = (float)(rand() % 760 + 125) / 10.0f;
                return;
            }
        }
    }

    /* 2. Si el CPU quedó libre, seleccionar el siguiente proceso LISTO */
    if (*proceso_en_cpu == NULL && cola->frente != NULL) {
        Proceso* siguiente = NULL;

        if (algoritmo == ALGORITMO_RR) {
            while (cola->frente != NULL) {
                Proceso* candidato = desencolar(cola);
                if (candidato != NULL && candidato->estado == LISTO) {
                    siguiente = candidato;
                    break;
                }
            }
        } else if (algoritmo == ALGORITMO_PRIORIDAD) {
            /* Usamos la Cola de Prioridad para ordenar los procesos listos */
            ColaPrioridad cp;
            inicializar_cola_prioridad(&cp);

            while (cola->frente != NULL) {
                Proceso* candidato = desencolar(cola);
                if (candidato != NULL && candidato->estado == LISTO) {
                    encolar_prioridad(&cp, candidato);
                }
            }

            siguiente = desencolar_prioridad(&cp);

            /* Devolvemos los demás procesos listos a la cola del sistema */
            while (cp.frente != NULL) {
                encolar(cola, desencolar_prioridad(&cp));
            }
        } else if (algoritmo == ALGORITMO_SJF) {
            /* Shortest Job First: menor tiempo restante estimado */
            Nodo* act = cola->frente;
            Proceso* mejor = NULL;
            int menor_restante = 999999;

            while (act != NULL) {
                if (act->proceso != NULL && act->proceso->estado == LISTO) {
                    int restante = act->proceso->tiempo_rafaga - act->proceso->tiempo_ejecucion;
                    if (restante < menor_restante) {
                        menor_restante = restante;
                        mejor = act->proceso;
                    }
                }
                act = act->siguiente;
            }

            if (mejor != NULL) {
                eliminar_de_cola(cola, mejor->pid);
                siguiente = mejor;
            }
        }

        if (siguiente != NULL) {
            siguiente->estado = EJECUCION;
            siguiente->cpu_uso = (float)(rand() % 760 + 125) / 10.0f;
            *proceso_en_cpu = siguiente;
        }
    }
}
