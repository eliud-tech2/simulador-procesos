#include <stdio.h>
#include <stdlib.h>
#include "planificador.h"

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
    if (p_actual->tiempo_ejecucion >= 50) {
        p_actual->estado = TERMINADO;
        p_actual->cpu_uso = 0.0f; // Libera el CPU
    } else {
        // 5. Expropiación: Se acabó el quantum pero no ha terminado. Vuelve a la fila.
        p_actual->estado = LISTO;
        // Al salir de ejecución baja su consumo de CPU a un rango pasivo (0.5% a 3.2%)
        p_actual->cpu_uso = (float)(rand() % 28 + 5) / 10.0f;
        encolar(cola, p_actual);
    }
}
