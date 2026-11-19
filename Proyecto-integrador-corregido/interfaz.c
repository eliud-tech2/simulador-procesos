#include <stdio.h>
#include <stdlib.h>
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
        case 32:  printf("\033[30;42m"); break;  /* Negro sobre fondo verde (franja htop) */
        case 176: printf("\033[30;46m"); break;  /* Negro sobre fondo cyan (teclas htop) */
        default:  printf("\033[0m");  break;
    }
#endif
}

static void restaurar_color() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Blanco/Gris por defecto
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
        fijar_color(10); // Verde suave
    } else if (porcentaje < 70.0f) {
        fijar_color(14); // Amarillo
    } else if (porcentaje < 88.0f) {
        fijar_color(6);  // Naranja
    } else {
        fijar_color(12); // Rojo peligro
    }

    for (int i = 0; i < total_bloques; i++) {
        if (i < bloques_llenos) printf("|");
        else printf(" ");
    }
    restaurar_color();
    printf("]");
}

static const char* texto_crit_orden(CriterioOrden c) {
    switch (c) {
        case CRITERIO_MEMORIA: return "MEM%";
        case CRITERIO_CPU:     return "CPU%";
        case CRITERIO_TIEMPO:  return "TIME+";
        case CRITERIO_PID:     return "PID";
        default:               return "MEM%";
    }
}

void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo) {
    dibujar_interfaz_htop(lista, cola, ciclo, CRITERIO_MEMORIA, 20);
}

void dibujar_interfaz_htop(ListaProcesos* lista, ColaListos* cola, int ciclo,
                           CriterioOrden crit_orden, int quantum) {
    consola_limpiar();

    float cpu_total_1 = 0.0f;
    float cpu_total_2 = 0.0f;
    float ram_usada_gb = 0.8f;

    int total_activos = 0;
    int total_ejecutando = 0;
    int total_espera = 0;
    int total_terminados = 0;

    Nodo* temp = lista->cabeza;
    int idx_cpu = 0;
    while (temp != NULL) {
        Proceso* p = temp->proceso;
        if (p->estado != TERMINADO) {
            total_activos++;
            ram_usada_gb += (p->mem_uso / 100.0f) * 1.2f;
            if (p->estado == EJECUCION) {
                total_ejecutando++;
                cpu_total_1 += p->cpu_uso;
            } else if (p->estado == ESPERA) {
                total_espera++;
            } else {
                if (idx_cpu % 2 == 0) cpu_total_2 += p->cpu_uso;
                else cpu_total_1 += p->cpu_uso * 0.3f;
                idx_cpu++;
            }
        } else {
            total_terminados++;
        }
        temp = temp->siguiente;
    }

    if (cpu_total_1 > 100.0f) cpu_total_1 = 98.5f;
    if (cpu_total_2 > 100.0f) cpu_total_2 = 96.0f;
    if (ram_usada_gb > 7.39f) ram_usada_gb = 7.25f;
    float ram_porcentaje = (ram_usada_gb / 7.39f) * 100.0f;

    int seg_totales = (int)(ciclo * 1.5f);
    int up_min = (seg_totales / 60) % 60;
    int up_seg = seg_totales % 60;
    float load_1 = (cpu_total_1 + cpu_total_2) / 100.0f;

    /* Encabezado superior de doble columna idéntico a htop de Linux */
    fijar_color(11); printf("  0"); restaurar_color();
    dibujar_barra_con_color(cpu_total_1);
    printf(" %5.1f%%   ", cpu_total_1);
    fijar_color(11); printf("Tasks: "); restaurar_color();
    printf("%d, %d thr; ", total_activos, cola->tamano);
    fijar_color(10); printf("%d running\n", total_ejecutando); restaurar_color();

    fijar_color(11); printf("  1"); restaurar_color();
    dibujar_barra_con_color(cpu_total_2);
    printf(" %5.1f%%   ", cpu_total_2);
    fijar_color(11); printf("Load average: "); restaurar_color();
    printf("%.2f %.2f %.2f\n", load_1, load_1 * 0.85f, load_1 * 0.70f);

    fijar_color(11); printf("Mem"); restaurar_color();
    dibujar_barra_con_color(ram_porcentaje);
    printf(" %.2fG/7.39G  ", ram_usada_gb);
    fijar_color(11); printf("Uptime: "); restaurar_color();
    printf("00:%02d:%02d (Ciclo #%d)\n", up_min, up_seg, ciclo);

    fijar_color(11); printf("Swp"); restaurar_color();
    dibujar_barra_con_color(0.0f);
    printf(" 0.00G/2.00G  ");
    fijar_color(11); printf("Sched: "); restaurar_color();
    printf("RoundRobin(Q=%d) | Heapsort(%s)\n\n", quantum, texto_crit_orden(crit_orden));

    Proceso** arreglo = convertir_lista_a_arreglo(lista);
    if (arreglo != NULL && lista->tamano > 0) {
        heapsort_procesos(arreglo, lista->tamano, crit_orden);
    }

    /* Franja de columnas con fondo verde y letras negras estilo htop */
    fijar_color(32);
    printf("  PID USER      State  CPU%%  MEM%%     TIME+  Command                            ");
    restaurar_color();
    printf("\n");

    /* Solo mostramos procesos activos para mantener la tabla limpia */
    for (int i = 0; i < lista->tamano && arreglo != NULL; i++) {
        Proceso* p = arreglo[i];
        if (p->estado == TERMINADO) continue;

        const char* user = (p->pid % 2 == 0) ? "root" : "eliud";
        int mins = p->tiempo_ejecucion / 60;
        int secs = p->tiempo_ejecucion % 60;
        int cent = (p->tiempo_rafaga - p->tiempo_ejecucion) % 100;
        if (cent < 0) cent = 0;

        if (p->estado == EJECUCION) {
            fijar_color(10);
            printf("%5d %-9s   R   %5.1f %5.1f  %2d:%02d.%02d  /usr/bin/%s\n",
                   p->pid, user, p->cpu_uso, p->mem_uso, mins, secs, cent, p->nombre);
            restaurar_color();
        } else if (p->estado == ESPERA) {
            fijar_color(11);
            printf("%5d %-9s   T   %5.1f %5.1f  %2d:%02d.%02d  /usr/bin/%s\n",
                   p->pid, user, p->cpu_uso, p->mem_uso, mins, secs, cent, p->nombre);
            restaurar_color();
        } else {
            printf("%5d %-9s   S   %5.1f %5.1f  %2d:%02d.%02d  /usr/bin/%s\n",
                   p->pid, user, p->cpu_uso, p->mem_uso, mins, secs, cent, p->nombre);
        }
    }

    /* Barra inferior de una sola línea estilo teclas de función de htop */
    printf("\n");
    fijar_color(11);  printf("1"); fijar_color(176); printf("Search ");
    fijar_color(11);  printf("2"); fijar_color(176); printf("SortBy ");
    fijar_color(11);  printf("3"); fijar_color(176); printf("State(S/T) ");
    fijar_color(11);  printf("4"); fijar_color(176); printf("Quantum ");
    fijar_color(11);  printf("5"); fijar_color(176); printf("Terminated(%d) ", total_terminados);
    fijar_color(11);  printf("6"); fijar_color(176); printf("Kill ");
    fijar_color(11);  printf("0"); fijar_color(176); printf("Quit ");
    restaurar_color();
    printf("\n");

    if (arreglo != NULL) free(arreglo);
}
