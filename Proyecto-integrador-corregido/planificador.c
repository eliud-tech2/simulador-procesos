#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "planificador.h"
#include "proceso.h"

#define RAM_TOTAL_GB 7.39f
#define SWP_TOTAL_GB 2.00f

/* Quantum por nivel en MLFQ (Silberschatz 5.3.6): Q0 = q, Q1 = 2q, Q2 = 4q */
static int obtener_quantum_cola(int nivel, int quantum_base) {
    if (nivel <= 0) return quantum_base;
    if (nivel == 1) return quantum_base * 2;
    return quantum_base * 4;
}

/* Planificador MLFQ (Colas Multinivel Realimentadas, Silberschatz 5.3.6) */
void ejecutar_planificador_multicola(ColaListos colas[NUM_COLAS_PRIORIDAD], int quantum,
                                     Proceso** proceso_en_cpu, int ciclo_actual) {
    if (colas == NULL || proceso_en_cpu == NULL) return;

    /* 1. Evaluar el proceso que ocupó la CPU durante el turno anterior */
    if (*proceso_en_cpu != NULL) {
        Proceso* p_actual = *proceso_en_cpu;

        /* Si fue suspendido o cerrado manualmente/por OOM, solo soltamos la CPU */
        if (p_actual->estado != EJECUCION) {
            p_actual->cpu_uso = 0.0f;
            *proceso_en_cpu = NULL;
        } else {
            int q_cola = obtener_quantum_cola(p_actual->prioridad, quantum);
            int restante = p_actual->tiempo_rafaga - p_actual->tiempo_ejecucion;
            if (restante < 0) restante = 0;

            /* Round Robin exacto (5.3.4): nunca consume más que su ráfaga restante */
            int ejecutado = (restante < q_cola) ? restante : q_cola;
            p_actual->tiempo_ejecucion += ejecutado;
            p_actual->quantums_usados++;

            if (p_actual->tiempo_ejecucion >= p_actual->tiempo_rafaga) {
                terminar_proceso(p_actual, ciclo_actual, FIN_COMPLETADO);
                *proceso_en_cpu = NULL;
            } else {
                /* Realimentación (5.3.6): agotó su quantum -> baja un nivel de cola */
                if (p_actual->prioridad < NUM_COLAS_PRIORIDAD - 1) {
                    p_actual->prioridad++;
                }
                p_actual->estado = LISTO;
                p_actual->ciclos_esperando = 0;
                p_actual->cpu_uso = 0.0f; /* En LISTO no consume CPU */
                encolar(&colas[p_actual->prioridad], p_actual);
                *proceso_en_cpu = NULL;
            }
        }
    }

    /* 2. Despachador con prioridad estricta (Q0 -> Q1 -> Q2) y orden FIFO interno */
    if (*proceso_en_cpu == NULL) {
        Proceso* siguiente = NULL;
        for (int nivel = 0; nivel < NUM_COLAS_PRIORIDAD && siguiente == NULL; nivel++) {
            while (colas[nivel].frente != NULL) {
                Proceso* candidato = desencolar(&colas[nivel]);
                if (candidato != NULL && candidato->estado == LISTO) {
                    siguiente = candidato;
                    break;
                }
            }
        }

        if (siguiente != NULL) {
            siguiente->estado = EJECUCION;
            siguiente->ciclos_esperando = 0;
            siguiente->cpu_uso = (float)(rand() % 650 + 200) / 10.0f;
            *proceso_en_cpu = siguiente;
        }
    }
}

/* Envejecimiento (Aging): evita la inanición subiendo de prioridad a procesos que esperan demasiado */
void aplicar_envejecimiento(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                            int ciclos_para_subir) {
    if (lista == NULL || colas == NULL || ciclos_para_subir <= 0) return;

    Nodo* actual = lista->cabeza;
    while (actual != NULL) {
        Proceso* p = actual->proceso;
        if (p != NULL && p->estado == LISTO && p->prioridad > 0) {
            p->ciclos_esperando++;
            if (p->ciclos_esperando >= ciclos_para_subir) {
                int cola_vieja = p->prioridad;
                p->prioridad--; /* Sube un nivel: Q2 -> Q1 o Q1 -> Q0 */
                p->ciclos_esperando = 0;

                /* Se mueve físicamente de la cola inferior a la cola superior */
                eliminar_de_cola(&colas[cola_vieja], p->pid);
                if (!esta_en_cola(&colas[p->prioridad], p->pid)) {
                    encolar(&colas[p->prioridad], p);
                }
            }
        }
        actual = actual->siguiente;
    }
}

/* Calcula RAM y Swap reales; activa control de admisión y OOM Killer si es crítico.
 * Retorna 1 si se permite admitir nuevos procesos, o 0 si la memoria está saturada. */
int verificar_memoria_y_oom(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                            Proceso** proceso_en_cpu, int ciclo_actual,
                            float* out_ram_gb, float* out_swp_gb,
                            char* mensaje_oom, int tam_mensaje) {
    float mem_total_gb = 0.80f; /* Base del sistema operativo */
    Nodo* temp = lista->cabeza;

    while (temp != NULL) {
        if (temp->proceso->estado != TERMINADO) {
            mem_total_gb += (temp->proceso->mem_uso / 100.0f) * RAM_TOTAL_GB;
        }
        temp = temp->siguiente;
    }

    float limite_critico = RAM_TOTAL_GB + (SWP_TOTAL_GB * 0.90f); /* ~9.19 GB */
    float limite_admision = RAM_TOTAL_GB + (SWP_TOTAL_GB * 0.65f); /* ~8.69 GB */

    if (mensaje_oom != NULL && tam_mensaje > 0) {
        mensaje_oom[0] = '\0';
    }

    /* OOM Killer automático: si Mem + Swap supera el límite crítico, sacrifica al más pesado de menor prioridad */
    if (mem_total_gb >= limite_critico) {
        Proceso* victima = NULL;

        for (int prio_obj = NUM_COLAS_PRIORIDAD - 1; prio_obj >= 0 && victima == NULL; prio_obj--) {
            Nodo* act = lista->cabeza;
            while (act != NULL) {
                Proceso* p = act->proceso;
                if (p->estado != TERMINADO && p->prioridad == prio_obj && p != *proceso_en_cpu) {
                    if (victima == NULL || p->mem_uso > victima->mem_uso) {
                        victima = p;
                    }
                }
                act = act->siguiente;
            }
        }

        if (victima != NULL) {
            float gb_liberados = (victima->mem_uso / 100.0f) * RAM_TOTAL_GB;
            int v_pid = victima->pid;
            int v_prio = victima->prioridad;
            char v_nom[50];
            strncpy(v_nom, victima->nombre, 49);
            v_nom[49] = '\0';

            terminar_proceso(victima, ciclo_actual, FIN_OOM_KILLER);
            eliminar_de_cola(&colas[v_prio], v_pid);
            mem_total_gb -= gb_liberados;

            if (mensaje_oom != NULL && tam_mensaje > 0) {
                snprintf(mensaje_oom, tam_mensaje,
                         "OOM-Killer: cerrado PID %d (%s, Q%d) -> liberados %.2fG",
                         v_pid, v_nom, v_prio, gb_liberados);
            }
        }
    }

    /* Reparto entre RAM física y Swap */
    float ram_gb = mem_total_gb;
    float swp_gb = 0.0f;
    if (ram_gb > RAM_TOTAL_GB) {
        swp_gb = ram_gb - RAM_TOTAL_GB;
        ram_gb = RAM_TOTAL_GB;
        if (swp_gb > SWP_TOTAL_GB) swp_gb = SWP_TOTAL_GB;
    }

    if (out_ram_gb != NULL) *out_ram_gb = ram_gb;
    if (out_swp_gb != NULL) *out_swp_gb = swp_gb;

    return (mem_total_gb < limite_admision) ? 1 : 0;
}
