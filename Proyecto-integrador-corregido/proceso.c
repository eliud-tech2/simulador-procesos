#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proceso.h"

/* Construye un proceso nuevo reservando memoria RAM */
Proceso* crear_proceso(int pid, const char* nombre, int prioridad, float mem_uso) {
    // 1. Pedimos la memoria exacta que pesa un struct Proceso
    Proceso* nuevo = (Proceso*)malloc(sizeof(Proceso));

    // Si la PC se queda sin RAM, malloc devuelve NULL. Hay que validar siempre.
    if (nuevo == NULL) {
        printf("Error fatal: No hay memoria para el proceso %d\n", pid);
        return NULL;
    }

    // 2. Inicializamos los atributos según las reglas de los sistemas operativos
    nuevo->pid = pid;

    // strncpy es más seguro que strcpy para evitar desbordamientos de memoria
    strncpy(nuevo->nombre, nombre, 49);
    nuevo->nombre[49] = '\0';

    nuevo->prioridad = prioridad;
    nuevo->cpu_uso = 0.0f; // Nace sin usar CPU, el planificador se lo subirá
    nuevo->mem_uso = mem_uso;
    nuevo->estado = LISTO; // Todo proceso nuevo entra directamente a la fila de LISTOS
    nuevo->tiempo_ejecucion = 0;
    nuevo->tiempo_rafaga = 40 + (rand() % 61); // Ráfaga estimada entre 40 y 100 ciclos

    return nuevo;
}

/* Cambia el estado a ESPERA simulando un bloqueo por Entrada/Salida */
void suspender_proceso(Proceso* p) {
    if (p != NULL && p->estado != TERMINADO) {
        p->estado = ESPERA;
        p->cpu_uso = 0.0f; // Al suspenderse libera el procesador
    }
}

/* Regresa el proceso a la fila del planificador */
void reanudar_proceso(Proceso* p) {
    if (p != NULL && p->estado == ESPERA) {
        p->estado = LISTO;
    }
}

/* Simula la muerte lógica del proceso liberando sus recursos */
void terminar_proceso(Proceso* p) {
    if (p != NULL) {
        p->estado = TERMINADO;
        p->cpu_uso = 0.0f;
        p->mem_uso = 0.0f;
    }
}

/* Destruye físicamente el proceso liberando la memoria RAM de tu computadora */
void destruir_proceso(Proceso* p) {
    if (p != NULL) {
        free(p); // La regla de oro en C: todo malloc() requiere su free()
    }
}
