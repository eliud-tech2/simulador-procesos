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
        case 10: printf("\033[92m"); break; /* Verde */
        case 14: printf("\033[93m"); break; /* Amarillo */
        case 6:  printf("\033[33m"); break; /* Naranja */
        case 12: printf("\033[91m"); break; /* Rojo */
        case 11: printf("\033[96m"); break; /* Cyan */
        case 8:  printf("\033[90m"); break; /* Gris */
        default: printf("\033[0m");  break;
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
    int total_bloques = 20;
    int bloques_llenos = (int)((porcentaje / 100.0f) * total_bloques);
    if (bloques_llenos > total_bloques) bloques_llenos = total_bloques;
    if (bloques_llenos < 0) bloques_llenos = 0;

    if (porcentaje < 40.0f) {
        fijar_color(10); // Verde suave
    } else if (porcentaje < 70.0f) {
        fijar_color(14); // Amarillo
    } else if (porcentaje < 88.0f) {
        fijar_color(6);  // Naranja
    } else {
        fijar_color(12); // Rojo peligro
    }

    printf("[");
    for (int i = 0; i < total_bloques; i++) {
        if (i < bloques_llenos) printf("|");
        else printf(" ");
    }
    printf("]");
    restaurar_color();
}

static const char* texto_alg_orden(AlgoritmoOrden a) {
    switch (a) {
        case ORDEN_QUICKSORT: return "Quicksort";
        case ORDEN_RADIX:     return "Radix";
        default:              return "Quicksort";
    }
}

static const char* texto_crit_orden(CriterioOrden c) {
    switch (c) {
        case CRITERIO_MEMORIA: return "RAM%";
        case CRITERIO_CPU:     return "CPU%";
        case CRITERIO_TIEMPO:  return "Tiempo";
        case CRITERIO_PID:     return "PID";
        default:               return "RAM%";
    }
}

void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo) {
    dibujar_interfaz_htop(lista, cola, ciclo, ORDEN_QUICKSORT, CRITERIO_MEMORIA, 20);
}

void dibujar_interfaz_htop(ListaProcesos* lista, ColaListos* cola, int ciclo,
                           AlgoritmoOrden alg_orden, CriterioOrden crit_orden,
                           int quantum) {
    consola_limpiar();

    float cpu_total_1 = 0.0f;
    float cpu_total_2 = 0.0f;
    float ram_usada_gb = 0.8f;

    int total_activos = 0;
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

    Proceso** arreglo = convertir_lista_a_arreglo(lista);
    if (arreglo != NULL && lista->tamano > 0) {
        ordenar_procesos(arreglo, lista->tamano, alg_orden, crit_orden);
    }

    printf("==================================================================================\n");
    printf(" [MODO LINUX - htop]  Planificador: Round Robin (Q=%d) | Orden: %s (%s)\n",
           quantum, texto_alg_orden(alg_orden), texto_crit_orden(crit_orden));
    printf("----------------------------------------------------------------------------------\n");
    printf(" CPU 1 "); dibujar_barra_con_color(cpu_total_1); printf(" %5.1f%%\n", cpu_total_1);
    printf(" CPU 2 "); dibujar_barra_con_color(cpu_total_2); printf(" %5.1f%%\n", cpu_total_2);
    printf(" MEM   "); dibujar_barra_con_color(ram_porcentaje); printf(" %.2fG/7.39G (%5.1f%%)\n", ram_usada_gb, ram_porcentaje);
    printf("----------------------------------------------------------------------------------\n");
    printf(" Activos: %d | Listos(Cola): %d | Espera: %d | Terminados: %d | Ciclo #%d\n",
           total_activos, cola->tamano, total_espera, total_terminados, ciclo);
    printf("==================================================================================\n\n");

    fijar_color(10);
    printf("  PID   USUARIO    CPU%%    RAM%%   TIEMPO       ESTADO         COMANDO\n");
    restaurar_color();
    printf("----------------------------------------------------------------------------------\n");

    /* Solo mostramos procesos activos para no llenar la tabla con procesos terminados */
    for (int i = 0; i < lista->tamano && arreglo != NULL; i++) {
        Proceso* p = arreglo[i];
        if (p->estado == TERMINADO) continue;

        const char* user = (p->pid % 2 == 0) ? "root" : "eliud";

        printf("%5d   %-7s   %5.1f   %5.1f  %3d/%-3d      ",
               p->pid, user, p->cpu_uso, p->mem_uso,
               p->tiempo_ejecucion, p->tiempo_rafaga);

        if (p->estado == EJECUCION) {
            fijar_color(10); printf("%-13s ", "[EJECUTANDO]");
        } else if (p->estado == ESPERA) {
            fijar_color(11); printf("%-13s ", "[ESPERA]");
        } else {
            fijar_color(14); printf("%-13s ", "[LISTO]");
        }
        restaurar_color();
        printf("%s\n", p->nombre);
    }

    printf("\n==================================================================================\n");
    printf(" OPCIONES DEL SISTEMA (Usa las teclas numericas):\n");
    printf(" [1] Buscar     (Hash Nombre / Binaria PID)    | [4] Quantum    (Ajustar RR)\n");
    printf(" [2] Ordenar    (Quicksort / Radix Sort)       | [5] Terminados (Ver finalizados)\n");
    printf(" [3] Procesos   (Suspender/Reanudar/Forzar)    | [6] Salir\n");
    printf("==================================================================================\n");

    if (arreglo != NULL) free(arreglo);
}