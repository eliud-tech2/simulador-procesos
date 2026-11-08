#include <stdio.h>
#include "planificador.h"

void ejecutar_round_robin(ColaListos* cola, int quantum) {
    if (cola == NULL || cola->frente == NULL) {
        return; // El CPU está inactivo porque no hay fila
    }

    // 1. Despachador: Sacamos al proceso que está al frente de la cola
    Proceso* p_actual = desencolar(cola);

    // 2. Cambio de contexto: Entra al procesador
    p_actual->estado = EJECUCION;
    p_actual->cpu_uso = 85.5f; // Simulamos alto consumo mientras corre

    printf("\n[CPU] Ejecutando PID: %d (%s) | Tiempo Acumulado anterior: %d\n",
           p_actual->pid, p_actual->nombre, p_actual->tiempo_ejecucion);

    // 3. Ejecución: Pasa el tiempo del quantum
    printf("      -> Asignando procesador por %d unidades de tiempo (Quantum)...\n", quantum);
    p_actual->tiempo_ejecucion += quantum;

    // 4. Evaluación: ¿Terminó su trabajo?
    // (Para esta simulación, definimos que todo proceso finaliza al llegar a 50 unidades de tiempo)
    if (p_actual->tiempo_ejecucion >= 50) {
        p_actual->estado = TERMINADO;
        p_actual->cpu_uso = 0.0f; // Libera el CPU
        printf("      -> [X] El proceso ha TERMINADO su ejecucion y sale del sistema.\n");
    } else {
        // 5. Expropiación: Se acabó el quantum pero no ha terminado. Vuelve a la fila.
        p_actual->estado = LISTO;
        p_actual->cpu_uso = 0.0f;
        printf("      -> [-] Fin del quantum. Volviendo al final de la fila de Listos.\n");
        encolar(cola, p_actual);
    }
}
