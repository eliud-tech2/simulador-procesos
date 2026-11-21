#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proceso.h"

/* Construye un proceso nuevo reservando memoria RAM */
Proceso* crear_proceso(int pid, const char* nombre, int prioridad, float mem_uso, int ciclo_actual) {
    Proceso* nuevo = (Proceso*)malloc(sizeof(Proceso));

    if (nuevo == NULL) {
        printf("Error fatal: No hay memoria para el proceso %d\n", pid);
        return NULL;
    }

    nuevo->pid = pid;
    strncpy(nuevo->nombre, nombre, 49);
    nuevo->nombre[49] = '\0';

    if (prioridad < 0) prioridad = 0;
    if (prioridad > 2) prioridad = 2;
    nuevo->prioridad = prioridad;

    nuevo->cpu_uso = 0.0f;
    nuevo->mem_uso = mem_uso;
    nuevo->estado = LISTO;
    nuevo->motivo_fin = FIN_NINGUNO;
    nuevo->tiempo_ejecucion = 0;
    /* Rafaga entre 5 y 100 ciclos: permite procesos cortos en Q0 y largos que bajan a Q1/Q2 */
    nuevo->tiempo_rafaga = 5 + (rand() % 96);
    nuevo->quantums_usados = 0;
    nuevo->ciclos_esperando = 0;
    nuevo->ciclo_llegada = ciclo_actual;
    nuevo->ciclo_fin = 0;

    return nuevo;
}

/* Cambia el estado a ESPERA simulando un bloqueo por Entrada/Salida */
void suspender_proceso(Proceso* p) {
    if (p != NULL && p->estado != TERMINADO) {
        p->estado = ESPERA;
        p->cpu_uso = 0.0f;
    }
}

/* Regresa el proceso a la fila del planificador */
void reanudar_proceso(Proceso* p) {
    if (p != NULL && p->estado == ESPERA) {
        p->estado = LISTO;
    }
}

/* Simula la muerte logica del proceso registrando el motivo y liberando sus recursos */
void terminar_proceso(Proceso* p, int ciclo_actual, MotivoFin motivo) {
    if (p != NULL) {
        p->estado = TERMINADO;
        p->motivo_fin = motivo;
        p->cpu_uso = 0.0f;
        p->mem_uso = 0.0f;
        p->ciclos_esperando = 0;
        if (p->ciclo_fin == 0) {
            p->ciclo_fin = ciclo_actual;
        }
    }
}

/* Destruye fisicamente el proceso liberando la memoria RAM */
void destruir_proceso(Proceso* p) {
    if (p != NULL) {
        free(p);
    }
}
