#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "interfaz.h"
#include "consola.h"
#include "ordenamiento.h"

static void fijar_color(int color_win) {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_win);
#else
    switch (color_win) {
        case 10:  printf("\033[92m"); break;     /* Verde brillante */
        case 14:  printf("\033[93m"); break;     /* Amarillo */
        case 6:   printf("\033[33m"); break;     /* Naranja */
        case 12:  printf("\033[91m"); break;     /* Rojo */
        case 11:  printf("\033[96m"); break;     /* Cyan */
        case 8:   printf("\033[90m"); break;     /* Gris */
        case 32:  printf("\033[30;42m"); break;  /* Negro sobre fondo verde */
        case 176: printf("\033[30;46m"); break;  /* Negro sobre fondo cyan */
        default:  printf("\033[0m");  break;
    }
#endif
}

static void restaurar_color() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    printf("\033[0m");
#endif
}

static void dibujar_barra_con_color(float porcentaje) {
    int total_bloques = 25;
    int bloques_llenos = (int)((porcentaje / 100.0f) * total_bloques);
    if (bloques_llenos > total_bloques) bloques_llenos = total_bloques;
    if (bloques_llenos < 0) bloques_llenos = 0;

    printf("[");
    if (porcentaje < 40.0f) {
        fijar_color(10);
    } else if (porcentaje < 70.0f) {
        fijar_color(14);
    } else if (porcentaje < 88.0f) {
        fijar_color(6);
    } else {
        fijar_color(12);
    }

    for (int i = 0; i < total_bloques; i++) {
        if (i < bloques_llenos) printf("|");
        else printf(" ");
    }
    restaurar_color();
    printf("]");
}

void dibujar_interfaz_htop(ListaProcesos* lista, ColaListos colas[NUM_COLAS_PRIORIDAD],
                           int ciclo, CriterioOrden crit_orden, int quantum,
                           float ram_gb, float swp_gb, int admision_abierta,
                           const char* mensaje_oom) {
    (void)quantum;
    consola_limpiar();

    float cpu_total_1 = 0.0f;
    float cpu_total_2 = 0.0f;
    int total_activos = 0;
    int total_ejecutando = 0;
    int total_terminados = 0;

    Nodo* temp = lista->cabeza;
    while (temp != NULL) {
        Proceso* p = temp->proceso;
        if (p->estado != TERMINADO) {
            total_activos++;
            if (p->estado == EJECUCION) {
                total_ejecutando++;
                cpu_total_1 += p->cpu_uso;
            }
        } else {
            total_terminados++;
        }
        temp = temp->siguiente;
    }

    if (cpu_total_1 > 100.0f) cpu_total_1 = 98.5f;

    float ram_porcentaje = (ram_gb / 7.39f) * 100.0f;
    float swp_porcentaje = (swp_gb / 2.00f) * 100.0f;

    int seg_totales = (int)(ciclo * 1.5f);
    int up_min = (seg_totales / 60) % 60;
    int up_seg = seg_totales % 60;

    /* Encabezado superior limpio estilo htop */
    fijar_color(11); printf("  0"); restaurar_color();
    dibujar_barra_con_color(cpu_total_1);
    printf(" %5.1f%%   ", cpu_total_1);
    fijar_color(11); printf("Tasks: "); restaurar_color();
    printf("%d (Q0:%d Q1:%d Q2:%d); ", total_activos, colas[0].tamano, colas[1].tamano, colas[2].tamano);
    fijar_color(10); printf("%d running\n", total_ejecutando); restaurar_color();

    fijar_color(11); printf("  1"); restaurar_color();
    dibujar_barra_con_color(cpu_total_2);
    printf(" %5.1f%%\n", cpu_total_2);

    fijar_color(11); printf("Mem"); restaurar_color();
    dibujar_barra_con_color(ram_porcentaje);
    printf(" %.2fG/7.39G  ", ram_gb);
    fijar_color(11); printf("Uptime: "); restaurar_color();
    printf("00:%02d:%02d (Ciclo #%d)\n", up_min, up_seg, ciclo);

    fijar_color(11); printf("Swp"); restaurar_color();
    dibujar_barra_con_color(swp_porcentaje);
    printf(" %.2fG/2.00G  ", swp_gb);
    fijar_color(11); printf("Sched: "); restaurar_color();
    printf("MLFQ\n");

    if (!admision_abierta) {
        fijar_color(14);
        printf("[CONTROL DE ADMISION] Memoria saturada: ingreso de procesos nuevos pausado.\n");
        restaurar_color();
    }
    if (mensaje_oom != NULL && strlen(mensaje_oom) > 0) {
        fijar_color(12);
        printf("[%s]\n", mensaje_oom);
        restaurar_color();
    }
    printf("\n");

    Proceso** arreglo = convertir_lista_a_arreglo(lista);
    if (arreglo != NULL && lista->tamano > 0) {
        heapsort_procesos(arreglo, lista->tamano, crit_orden);
    }

    /* Franja de columnas con fondo verde y letras negras estilo htop */
    fijar_color(32);
    printf("  PID USER      PRI State  CPU%%  MEM%%  Q_USE  TIME+   Command                   ");
    restaurar_color();
    printf("\n");

    /* Solo mostramos procesos activos para mantener la tabla limpia */
    for (int i = 0; i < lista->tamano && arreglo != NULL; i++) {
        Proceso* p = arreglo[i];
        if (p->estado == TERMINADO) continue;

        const char* user = (p->pid % 2 == 0) ? "root" : "eliud";

        if (p->estado == EJECUCION) {
            fijar_color(10);
            printf("%5d %-9s Q%d    R   %5.1f %5.1f   %3d   %3d/%-3d  /usr/bin/%s\n",
                   p->pid, user, p->prioridad, p->cpu_uso, p->mem_uso,
                   p->quantums_usados, p->tiempo_ejecucion, p->tiempo_rafaga, p->nombre);
            restaurar_color();
        } else if (p->estado == ESPERA) {
            fijar_color(11);
            printf("%5d %-9s Q%d    T   %5.1f %5.1f   %3d   %3d/%-3d  /usr/bin/%s\n",
                   p->pid, user, p->prioridad, p->cpu_uso, p->mem_uso,
                   p->quantums_usados, p->tiempo_ejecucion, p->tiempo_rafaga, p->nombre);
            restaurar_color();
        } else {
            printf("%5d %-9s Q%d    S   %5.1f %5.1f   %3d   %3d/%-3d  /usr/bin/%s\n",
                   p->pid, user, p->prioridad, p->cpu_uso, p->mem_uso,
                   p->quantums_usados, p->tiempo_ejecucion, p->tiempo_rafaga, p->nombre);
        }
    }

    /* Barra inferior de una sola linea estilo teclas de funcion de htop */
    printf("\n");
    fijar_color(11);  printf("1"); fijar_color(176); printf("Search ");
    fijar_color(11);  printf("2"); fijar_color(176); printf("SortBy ");
    fijar_color(11);  printf("3"); fijar_color(176); printf("State ");
    fijar_color(11);  printf("4"); fijar_color(176); printf("Quantum ");
    fijar_color(11);  printf("5"); fijar_color(176); printf("Terminated(%d) ", total_terminados);
    fijar_color(11);  printf("6"); fijar_color(176); printf("Kill ");
    fijar_color(11);  printf("7"); fijar_color(176); printf("Stress ");
    fijar_color(11);  printf("0"); fijar_color(176); printf("Quit ");
    restaurar_color();
    printf("\n");

    if (arreglo != NULL) free(arreglo);
}
