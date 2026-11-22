#include <stdio.h>
#include <stdlib.h>
#include "generador.h"
#include "proceso.h"

/* Servicios exclusivos del sistema operativo (usuario root) - sin espacios */
static const char* banco_root[10] = {
    "systemd", "sshd", "cron", "syslogd", "dbus",
    "udevd", "networkd", "iptables", "dockerd", "kerneld"
};

/* Aplicaciones exclusivas del usuario (eliud) - sin espacios */
static const char* banco_usuario[12] = {
    "chrome", "firefox", "opera", "spotify", "discord", "steam",
    "vscode", "python", "gcc", "vlc", "zoom", "teams"
};

/* Genera un PID aleatorio unico entre 100 y 999 para mantener balanceado el BST.
 * es_root = 1 -> PID par (usuario root); es_root = 0 -> PID impar (usuario eliud). */
static int generar_pid_aleatorio(ArbolPID* arbol, int* contador_respaldo, int es_root) {
    for (int intento = 0; intento < 200; intento++) {
        int base = 100 + (rand() % 450) * 2; /* Par entre 100 y 998 */
        int candidato = es_root ? base : (base + 1); /* Impar entre 101 y 999 */
        if (arbol == NULL || buscar_arbol_pid(arbol->raiz, candidato) == NULL) {
            return candidato;
        }
    }

    /* Respaldo seguro si se llenara el rango 100-999 */
    int respaldo = (*contador_respaldo < 1000) ? 1000 : *contador_respaldo;
    if ((respaldo % 2 == 0) != (es_root != 0)) {
        respaldo++;
    }
    *contador_respaldo = respaldo + 2;
    return respaldo;
}

void intentar_crear_proceso_sistema(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                                    ArbolPID* arbol, TablaHash* th, int* contador_pid,
                                    int ciclo_actual, int permitir_admision) {
    /* 35% de probabilidad de que llegue un proceso nuevo en cada ciclo */
    if (rand() % 100 < 35) {
        int es_root = (rand() % 100 < 35); /* 35% servicios de root, 65% apps de usuario */
        const char* nombre = es_root ? banco_root[rand() % 10] : banco_usuario[rand() % 12];
        int pid_nuevo = generar_pid_aleatorio(arbol, contador_pid, es_root);
        int prio_inicial = 0; /* En MLFQ (5.3.6) todo proceso nuevo entra a Q0 */
        float mem_random = (float)(rand() % 145 + 35) / 10.0f; /* 3.5% a 17.9% de RAM */

        Proceso* nuevo = crear_proceso(pid_nuevo, nombre, prio_inicial, mem_random, ciclo_actual);
        if (nuevo != NULL) {
            insertar_lista(lista, nuevo);
            if (arbol != NULL) insertar_arbol(arbol, nuevo);
            if (th != NULL) insertar_hash(th, nuevo);

            if (permitir_admision) {
                encolar(&colas[prio_inicial], nuevo);
            } else {
                terminar_proceso(nuevo, ciclo_actual, FIN_RECHAZADO_MEM);
            }
        }
    }
}

/* Crea varios procesos pesados de golpe para probar Swap y OOM Killer en vivo */
int inyectar_carga_estres(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                          ArbolPID* arbol, TablaHash* th, int* contador_pid,
                          int ciclo_actual, int cantidad) {
    const char* pesados[5] = { "chrome", "steam", "firefox", "discord", "vscode" };
    int creados = 0;

    for (int i = 0; i < cantidad; i++) {
        const char* nom = pesados[i % 5];
        int pid_nuevo = generar_pid_aleatorio(arbol, contador_pid, 0); /* Apps de usuario */
        int prio = 0; /* Entran por Q0 y al ser pesados bajan a Q1 y Q2 */
        float mem_pesada = (float)(rand() % 65 + 125) / 10.0f; /* 12.5% a 18.9% cada uno */

        Proceso* nuevo = crear_proceso(pid_nuevo, nom, prio, mem_pesada, ciclo_actual);
        if (nuevo != NULL) {
            insertar_lista(lista, nuevo);
            encolar(&colas[prio], nuevo);
            if (arbol != NULL) insertar_arbol(arbol, nuevo);
            if (th != NULL) insertar_hash(th, nuevo);
            creados++;
        }
    }
    return creados;
}
