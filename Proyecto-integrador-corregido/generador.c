#include <stdio.h>
#include <stdlib.h>
#include "generador.h"
#include "proceso.h"

void intentar_crear_proceso_nuevo(ListaProcesos* lista, ColaListos* cola, int* contador_pid) {

    /* Banco de 20 nombres de procesos variados */
    const char* banco_nombres[20] = {
        "steam", "word", "code::blocks", "powerpoint", "excel",
        "Google Chrome", "opera", "discord", "spotify", "firefox",
        "vlc", "zoom", "teams", "mysql", "python",
        "bash", "htop", "gcc", "explorer", "antivirus"
    };

    /* 35% de probabilidad de que llegue un proceso nuevo en cada ciclo */
    if (rand() % 100 < 35) {

        int indice_aleatorio = rand() % 20;
        float mem_random = (rand() % 150) / 10.0f;

        Proceso* nuevo = crear_proceso(*contador_pid, banco_nombres[indice_aleatorio], mem_random);

        insertar_lista(lista, nuevo);
        encolar(cola, nuevo);
        (*contador_pid)++;
    }
}

void intentar_crear_proceso_sistema(ListaProcesos* lista, ColaListos* cola, ArbolPID* arbol, TablaHash* th, int* contador_pid) {

    /* Banco de 20 nombres de procesos variados */
    const char* banco_nombres[20] = {
        "steam", "word", "code::blocks", "powerpoint", "excel",
        "Google Chrome", "opera", "discord", "spotify", "firefox",
        "vlc", "zoom", "teams", "mysql", "python",
        "bash", "htop", "gcc", "explorer", "antivirus"
    };

    /* 35% de probabilidad de que llegue un proceso nuevo en cada ciclo */
    if (rand() % 100 < 35) {

        int indice_aleatorio = rand() % 20;
        float mem_random = (float)(rand() % 145 + 5) / 10.0f;

        Proceso* nuevo = crear_proceso(*contador_pid, banco_nombres[indice_aleatorio], mem_random);
        if (nuevo != NULL) {
            insertar_lista(lista, nuevo);
            encolar(cola, nuevo);
            if (arbol != NULL) insertar_arbol(arbol, nuevo);
            if (th != NULL) insertar_hash(th, nuevo);
            (*contador_pid)++;
        }
    }
}
