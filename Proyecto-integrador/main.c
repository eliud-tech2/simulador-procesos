#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "proceso.h"
#include "estructuras.h"
#include "planificador.h"
#include "generador.h" /* <--- Conectamos el nuevo modulo inyector de procesos */

int main() {
    printf("--- Simulador de CPU: Bucle Infinito OS ---\n\n");
    printf("Presiona Ctrl+C en cualquier momento para detener el simulador.\n\n");

    srand(time(NULL));

    ListaProcesos mi_lista;
    inicializar_lista(&mi_lista);

    ColaListos mi_cola_rr;
    inicializar_cola(&mi_cola_rr);

    /* Procesos base del sistema operativo */
    Proceso* p1 = crear_proceso(100, "bash", 1, 5.0f);
    Proceso* p2 = crear_proceso(101, "htop", 1, 2.5f);
    Proceso* p3 = crear_proceso(102, "gcc", 2, 18.0f);

    insertar_lista(&mi_lista, p1);
    insertar_lista(&mi_lista, p2);
    insertar_lista(&mi_lista, p3);

    encolar(&mi_cola_rr, p1);
    encolar(&mi_cola_rr, p2);
    encolar(&mi_cola_rr, p3);

    int quantum_sistema = 20;
    int ciclo = 1;
    int contador_pid_global = 103;

    /* Bucle infinito: el corazon del sistema operativo */
    while(1) {
        printf("\n--- CICLO DE RELOJ %d ---\n", ciclo);

        /* 1. El procesador atiende a quien le toque en la fila */
        ejecutar_round_robin(&mi_cola_rr, quantum_sistema);

        /* 2. Usamos el nuevo modulo para inyectar programas aleatorios */
        intentar_crear_proceso_nuevo(&mi_lista, &mi_cola_rr, &contador_pid_global);

        printf("      -> [Procesos en espera de CPU: %d]\n", mi_cola_rr.tamano);

        Sleep(1500);
        ciclo++;
    }

    return 0;
}
