#include <stdio.h>
#include <stdlib.h>
#include "generador.h"
#include "proceso.h"

static const char* banco_nombres[20] = {
    "steam", "word", "code::blocks", "powerpoint", "excel",
    "Google Chrome", "opera", "discord", "spotify", "firefox",
    "vlc", "zoom", "teams", "mysql", "python",
    "bash", "htop", "gcc", "explorer", "antivirus"
};

void intentar_crear_proceso_sistema(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                                    ArbolPID* arbol, TablaHash* th, int* contador_pid,
                                    int ciclo_actual, int permitir_admision) {
    /* 35% de probabilidad de que llegue un proceso nuevo en cada ciclo */
    if (rand() % 100 < 35) {
        int indice_aleatorio = rand() % 20;
        int prio_random = rand() % NUM_COLAS_PRIORIDAD; /* 0=Alta, 1=Media, 2=Baja */
        float mem_random = (float)(rand() % 145 + 35) / 10.0f; /* 3.5% a 17.9% de RAM */

        Proceso* nuevo = crear_proceso(*contador_pid, banco_nombres[indice_aleatorio],
                                       prio_random, mem_random, ciclo_actual);
        if (nuevo != NULL) {
            insertar_lista(lista, nuevo);
            if (arbol != NULL) insertar_arbol(arbol, nuevo);
            if (th != NULL) insertar_hash(th, nuevo);
            (*contador_pid)++;

            if (permitir_admision) {
                encolar(&colas[prio_random], nuevo);
            } else {
                /* No logró entrar por saturación: se registra como rechazado en el historial */
                terminar_proceso(nuevo, ciclo_actual, FIN_RECHAZADO_MEM);
            }
        }
    }
}

/* Crea varios procesos pesados de golpe para probar Swap y OOM Killer en vivo */
int inyectar_carga_estres(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                          ArbolPID* arbol, TablaHash* th, int* contador_pid,
                          int ciclo_actual, int cantidad) {
    const char* pesados[5] = { "Google Chrome", "steam", "mysql", "firefox", "discord" };
    int creados = 0;

    for (int i = 0; i < cantidad; i++) {
        const char* nom = pesados[i % 5];
        int prio = (i % 3 == 0) ? 1 : 2; /* Mayoría en prioridad Media/Baja */
        float mem_pesada = (float)(rand() % 65 + 125) / 10.0f; /* 12.5% a 18.9% cada uno */

        Proceso* nuevo = crear_proceso(*contador_pid, nom, prio, mem_pesada, ciclo_actual);
        if (nuevo != NULL) {
            insertar_lista(lista, nuevo);
            encolar(&colas[prio], nuevo);
            if (arbol != NULL) insertar_arbol(arbol, nuevo);
            if (th != NULL) insertar_hash(th, nuevo);
            (*contador_pid)++;
            creados++;
        }
    }
    return creados;
}
