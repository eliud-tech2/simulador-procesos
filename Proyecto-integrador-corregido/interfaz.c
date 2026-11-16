#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "interfaz.h"
#include "ordenamiento.h"

static void limpiar_pantalla() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

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

static const char* texto_planificador(AlgoritmoPlanificacion p) {
    switch (p) {
        case ALGORITMO_RR:        return "Round Robin";
        case ALGORITMO_PRIORIDAD: return "Prioridad";
        case ALGORITMO_SJF:       return "SJF";
        default:                  return "Round Robin";
    }
}

static const char* texto_alg_orden(AlgoritmoOrden a) {
    switch (a) {
        case ORDEN_QUICKSORT: return "Quicksort";
        case ORDEN_SHELLSORT: return "Shellsort";
        case ORDEN_RADIX:     return "Radix";
        default:              return "Quicksort";
    }
}

static const char* texto_crit_orden(CriterioOrden c) {
    switch (c) {
        case CRITERIO_MEMORIA:   return "RAM%";
        case CRITERIO_CPU:       return "CPU%";
        case CRITERIO_PRIORIDAD: return "Prioridad";
        case CRITERIO_TIEMPO:    return "Tiempo";
        case CRITERIO_PID:       return "PID";
        default:                 return "RAM%";
    }
}

static const char* texto_prioridad(int prio) {
    switch (prio) {
        case 3:  return "Critica";
        case 2:  return "Alta";
        case 1:  return "Normal";
        default: return "Baja";
    }
}

void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo) {
    dibujar_interfaz_completa(lista, cola, ciclo, VISTA_LINUX, ALGORITMO_RR, ORDEN_QUICKSORT, CRITERIO_MEMORIA, 20);
}

void dibujar_interfaz_completa(ListaProcesos* lista, ColaListos* cola, int ciclo,
                               ModoVista vista, AlgoritmoPlanificacion planificador,
                               AlgoritmoOrden alg_orden, CriterioOrden crit_orden,
                               int quantum) {
    limpiar_pantalla();

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

    Proceso** arreglo = convertir_lista_a_arreglo(lista);
    if (arreglo != NULL && lista->tamano > 0) {
        ordenar_procesos(arreglo, lista->tamano, alg_orden, crit_orden);

        /* Mover los procesos activos al inicio para que los terminados no tapen los nuevos */
        Proceso** orden_vista = (Proceso**)malloc(lista->tamano * sizeof(Proceso*));
        if (orden_vista != NULL) {
            int k = 0;
            for (int i = 0; i < lista->tamano; i++) {
                if (arreglo[i]->estado != TERMINADO) {
                    orden_vista[k++] = arreglo[i];
                }
            }
            for (int i = 0; i < lista->tamano; i++) {
                if (arreglo[i]->estado == TERMINADO) {
                    orden_vista[k++] = arreglo[i];
                }
            }
            for (int i = 0; i < lista->tamano; i++) {
                arreglo[i] = orden_vista[i];
            }
            free(orden_vista);
        }
    }

    int limite = lista->tamano;

    if (vista == VISTA_LINUX) {
        printf("==================================================================================\n");
        printf(" [MODO LINUX - htop]  Planificador: %-11s (Q=%d) | Orden: %s (%s)\n",
               texto_planificador(planificador), quantum, texto_alg_orden(alg_orden), texto_crit_orden(crit_orden));
        printf("----------------------------------------------------------------------------------\n");
        printf(" CPU 1 "); dibujar_barra_con_color(cpu_total_1); printf(" %5.1f%%\n", cpu_total_1);
        printf(" CPU 2 "); dibujar_barra_con_color(cpu_total_2); printf(" %5.1f%%\n", cpu_total_2);
        printf(" MEM   "); dibujar_barra_con_color(ram_porcentaje); printf(" %.2fG/7.39G (%5.1f%%)\n", ram_usada_gb, ram_porcentaje);
        printf("----------------------------------------------------------------------------------\n");
        printf(" Activos: %d | Listos(Cola): %d | Espera: %d | Terminados: %d | Ciclo #%d\n",
               total_activos, cola->tamano, total_espera, total_terminados, ciclo);
        printf("==================================================================================\n\n");

        fijar_color(10);
        printf("  PID   USUARIO   PRI    CPU%%    RAM%%   TIEMPO       ESTADO         COMANDO\n");
        restaurar_color();
        printf("----------------------------------------------------------------------------------\n");

        for (int i = 0; i < limite && arreglo != NULL; i++) {
            Proceso* p = arreglo[i];
            const char* user = (p->pid % 2 == 0) ? "root" : "eliud";

            printf("%5d   %-7s   %d     %5.1f   %5.1f  %3d/%-3d      ",
                   p->pid, user, p->prioridad, p->cpu_uso, p->mem_uso,
                   p->tiempo_ejecucion, p->tiempo_rafaga);

            if (p->estado == EJECUCION) {
                fijar_color(10); printf("%-13s ", "[EJECUTANDO]");
            } else if (p->estado == ESPERA) {
                fijar_color(11); printf("%-13s ", "[ESPERA]");
            } else if (p->estado == TERMINADO) {
                fijar_color(8);  printf("%-13s ", "[TERMINADO]");
            } else {
                fijar_color(14); printf("%-13s ", "[LISTO]");
            }
            restaurar_color();
            printf("%s\n", p->nombre);
        }
    } else if (vista == VISTA_WINDOWS) {
        printf("==================================================================================\n");
        fijar_color(11);
        printf("  ADMINISTRADOR DE TAREAS (WINDOWS)  |  [Procesos]  Rendimiento  Detalles\n");
        restaurar_color();
        printf("----------------------------------------------------------------------------------\n");
        printf("  CPU Total: %5.1f%%  |  Memoria: %5.1f%% (%.2f GB / 7.39 GB)  |  Ciclo #%d\n",
               (cpu_total_1 + cpu_total_2) / 2.0f, ram_porcentaje, ram_usada_gb, ciclo);
        printf("  Planificador: %-11s (Q=%d)  |  Orden: %s por %s\n",
               texto_planificador(planificador), quantum, texto_alg_orden(alg_orden), texto_crit_orden(crit_orden));
        printf("==================================================================================\n\n");

        fijar_color(11);
        printf(" %-18s  PID    Estado          Prioridad    CPU     Memoria    Tiempo\n", "Nombre");
        restaurar_color();
        printf("----------------------------------------------------------------------------------\n");

        for (int i = 0; i < limite && arreglo != NULL; i++) {
            Proceso* p = arreglo[i];
            printf(" %-18.18s %5d   ", p->nombre, p->pid);

            if (p->estado == EJECUCION) {
                fijar_color(10); printf("%-14s ", "En ejecucion");
            } else if (p->estado == ESPERA) {
                fijar_color(11); printf("%-14s ", "Suspendido");
            } else if (p->estado == TERMINADO) {
                fijar_color(8);  printf("%-14s ", "Finalizado");
            } else {
                fijar_color(14); printf("%-14s ", "En espera");
            }
            restaurar_color();

            printf("%-10s %5.1f%%   %5.1f%%    %3d/%-3d\n",
                   texto_prioridad(p->prioridad), p->cpu_uso, p->mem_uso,
                   p->tiempo_ejecucion, p->tiempo_rafaga);
        }
    } else {
        /* VISTA_ANDROID */
        printf("==================================================================================\n");
        fijar_color(10);
        printf("  ANDROID TASK MANAGER  -  Servicios en Ejecucion y Segundo Plano\n");
        restaurar_color();
        printf("----------------------------------------------------------------------------------\n");
        printf("  RAM Sistema: "); dibujar_barra_con_color(ram_porcentaje);
        printf(" %.2f GB usada | Libre: %.2f GB\n", ram_usada_gb, 7.39f - ram_usada_gb);
        printf("  Primer plano: %d | 2do plano: %d | Doze/Pausa: %d | Planif: %s | Ciclo #%d\n",
               total_ejecutando, cola->tamano, total_espera, texto_planificador(planificador), ciclo);
        printf("==================================================================================\n\n");

        fijar_color(10);
        printf("  PID    Paquete / App              Tipo               RAM%%    CPU%%    Prio\n");
        restaurar_color();
        printf("----------------------------------------------------------------------------------\n");

        for (int i = 0; i < limite && arreglo != NULL; i++) {
            Proceso* p = arreglo[i];
            printf(" %5d   com.sys.%-18.18s ", p->pid, p->nombre);

            if (p->estado == EJECUCION) {
                fijar_color(10); printf("%-17s ", "Primer plano");
            } else if (p->estado == ESPERA) {
                fijar_color(11); printf("%-17s ", "En pausa (Doze)");
            } else if (p->estado == TERMINADO) {
                fijar_color(8);  printf("%-17s ", "Detenida");
            } else {
                fijar_color(14); printf("%-17s ", "Segundo plano");
            }
            restaurar_color();

            printf("%5.1f%%  %5.1f%%    %d\n", p->mem_uso, p->cpu_uso, p->prioridad);
        }
    }

    printf("\n==================================================================================\n");
    printf(" OPCIONES DEL SISTEMA (Usa las teclas numericas):\n");
    printf(" [1] Buscar   (Binaria/BST/Hash/Secuencial) | [4] Planificador (RR/Prioridad/SJF)\n");
    printf(" [2] Ordenar  (Quicksort/Shellsort/Radix)   | [5] Cambiar Vista (Linux/Win/Android)\n");
    printf(" [3] Procesos (Crear/Suspender/Reanudar/Matar) | [6] Salir\n");
    printf("==================================================================================\n");

    if (arreglo != NULL) free(arreglo);
}
