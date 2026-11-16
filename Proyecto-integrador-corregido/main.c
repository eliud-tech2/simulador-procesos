#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "consola.h"
#include "proceso.h"
#include "estructuras.h"
#include "planificador.h"
#include "generador.h"
#include "ordenamiento.h"
#include "busqueda.h"
#include "interfaz.h"

static void limpiar_buffer_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void mostrar_detalle_proceso(Proceso* p) {
    const char* est = "DESCONOCIDO";
    if (p->estado == LISTO) est = "LISTO";
    else if (p->estado == EJECUCION) est = "EJECUCION";
    else if (p->estado == ESPERA) est = "ESPERA";
    else if (p->estado == TERMINADO) est = "TERMINADO";

    printf("\n[PROCESO ENCONTRADO]\n");
    printf(" - PID: %d\n", p->pid);
    printf(" - Nombre: %s\n", p->nombre);
    printf(" - CPU: %.1f%%\n", p->cpu_uso);
    printf(" - RAM: %.1f%%\n", p->mem_uso);
    printf(" - Estado: %s\n", est);
    printf(" - Ciclos procesados: %d / %d\n", p->tiempo_ejecucion, p->tiempo_rafaga);
}

int main() {
    srand((unsigned int)time(NULL));

    ListaProcesos mi_lista;
    inicializar_lista(&mi_lista);
    ColaListos mi_cola_rr;
    inicializar_cola(&mi_cola_rr);
    ArbolPID mi_arbol;
    inicializar_arbol(&mi_arbol);
    TablaHash mi_hash;
    inicializar_tabla_hash(&mi_hash);

    int quantum_sistema = 20;
    int ciclo = 1;
    int contador_pid = 100;
    Proceso* proceso_en_cpu = NULL;

    AlgoritmoOrden orden_algoritmo = ORDEN_QUICKSORT;
    CriterioOrden orden_criterio = CRITERIO_MEMORIA;

    /* Semilla inicial de procesos para que la tabla arranque poblada */
    for (int i = 0; i < 5; i++) {
        intentar_crear_proceso_sistema(&mi_lista, &mi_cola_rr, &mi_arbol, &mi_hash, &contador_pid);
    }

    while (1) {
        /* 1. Logica invisible del Sistema Operativo (Planificador Round Robin) */
        ejecutar_planificador_rr(&mi_cola_rr, quantum_sistema, &proceso_en_cpu);
        intentar_crear_proceso_sistema(&mi_lista, &mi_cola_rr, &mi_arbol, &mi_hash, &contador_pid);

        /* 2. Visualizacion en pantalla (solo modo Linux htop) */
        dibujar_interfaz_htop(&mi_lista, &mi_cola_rr, ciclo,
                              orden_algoritmo, orden_criterio, quantum_sistema);

        /* 3. Pausa interactiva de 1.5 segundos que vigila si presionas 1..6 */
        int tiempo_espera = 0;
        while (tiempo_espera < 1500) {
            if (consola_kbhit()) { // Detecta si presionaste alguna tecla
                char tecla = (char)consola_getch();

                if (tecla == '6') {
                    consola_limpiar();
                    printf("Liberando memoria y saliendo de la simulacion de forma segura...\n");
                    liberar_cola(&mi_cola_rr);
                    liberar_arbol(mi_arbol.raiz);
                    liberar_tabla_hash(&mi_hash);
                    liberar_lista(&mi_lista, 1);
                    return 0;
                }
                else if (tecla == '1') {
                    consola_limpiar();
                    printf("\n========================================\n");
                    printf("            MENU DE BUSQUEDA            \n");
                    printf("========================================\n");
                    printf("1. Buscar por Nombre (Tabla Hash)\n");
                    printf("2. Buscar por PID (Busqueda Binaria + Radix)\n");
                    printf("Elige metodo: ");
                    int op_busq = 0;
                    if (scanf("%d", &op_busq) != 1) op_busq = 0;
                    limpiar_buffer_entrada();

                    clock_t t_ini = clock();
                    if (op_busq == 1) {
                        char nombre_buscado[50];
                        printf("Introduce el nombre exacto (ej. spotify, gcc, python): ");
                        if (fgets(nombre_buscado, sizeof(nombre_buscado), stdin) != NULL) {
                            nombre_buscado[strcspn(nombre_buscado, "\r\n")] = '\0';
                            t_ini = clock();
                            buscar_hash_nombre(&mi_hash, nombre_buscado);
                        }
                    } else if (op_busq == 2) {
                        int pid_buscado = 0;
                        printf("Introduce el PID exacto a buscar: ");
                        if (scanf("%d", &pid_buscado) == 1) {
                            limpiar_buffer_entrada();
                            t_ini = clock();
                            Proceso** arr = convertir_lista_a_arreglo(&mi_lista);
                            radix_sort_por_pid(arr, mi_lista.tamano);
                            Proceso* res = busqueda_binaria_pid(arr, mi_lista.tamano, pid_buscado);
                            if (arr != NULL) free(arr);
                            if (res != NULL) mostrar_detalle_proceso(res);
                            else printf("\nNo se encontro ningun proceso con el PID %d.\n", pid_buscado);
                        }
                    } else {
                        printf("Opcion no valida.\n");
                    }
                    clock_t t_fin = clock();
                    double ms = ((double)(t_fin - t_ini) / CLOCKS_PER_SEC) * 1000.0;
                    printf("\n[Rendimiento] Tiempo de busqueda: %.4f ms\n", ms);
                    printf("Presiona cualquier tecla para volver al monitor...");
                    consola_getch();
                }
                else if (tecla == '2') {
                    consola_limpiar();
                    printf("\n========================================\n");
                    printf("            ORDENAMIENTO               \n");
                    printf("========================================\n");
                    printf("Algoritmo:\n 1. Quicksort\n 2. Radix Sort\nElige algoritmo: ");
                    int op_alg = 1;
                    if (scanf("%d", &op_alg) == 1) {
                        if (op_alg == 1) orden_algoritmo = ORDEN_QUICKSORT;
                        else if (op_alg == 2) orden_algoritmo = ORDEN_RADIX;
                    }
                    limpiar_buffer_entrada();

                    printf("\nCriterio:\n 1. RAM%%\n 2. CPU%%\n 3. Tiempo ejecutado\n 4. PID\nElige criterio: ");
                    int op_crit = 1;
                    if (scanf("%d", &op_crit) == 1) {
                        if (op_crit == 1) orden_criterio = CRITERIO_MEMORIA;
                        else if (op_crit == 2) orden_criterio = CRITERIO_CPU;
                        else if (op_crit == 3) orden_criterio = CRITERIO_TIEMPO;
                        else if (op_crit == 4) orden_criterio = CRITERIO_PID;
                    }
                    limpiar_buffer_entrada();

                    Proceso** arr = convertir_lista_a_arreglo(&mi_lista);
                    clock_t t_ini = clock();
                    if (arr != NULL && mi_lista.tamano > 0) {
                        ordenar_procesos(arr, mi_lista.tamano, orden_algoritmo, orden_criterio);
                    }
                    clock_t t_fin = clock();
                    if (arr != NULL) free(arr);

                    double ms = ((double)(t_fin - t_ini) / CLOCKS_PER_SEC) * 1000.0;
                    printf("\nTabla ordenada (%d procesos) en %.4f ms.\n", mi_lista.tamano, ms);
                    printf("Presiona cualquier tecla para volver al monitor...");
                    consola_getch();
                }
                else if (tecla == '3') {
                    consola_limpiar();
                    printf("\n========================================\n");
                    printf("        GESTION DE PROCESOS            \n");
                    printf("========================================\n");
                    printf("1. Suspender proceso (Pasar a ESPERA)\n");
                    printf("2. Reanudar proceso (Pasar a LISTO)\n");
                    printf("3. Forzar cierre de proceso (Terminar)\n");
                    printf("Elige accion: ");
                    int op_proc = 0;
                    if (scanf("%d", &op_proc) != 1) op_proc = 0;
                    limpiar_buffer_entrada();

                    if (op_proc >= 1 && op_proc <= 3) {
                        int pid_obj = 0;
                        printf("Introduce el PID del proceso: ");
                        if (scanf("%d", &pid_obj) == 1) {
                            limpiar_buffer_entrada();
                            Proceso* objetivo = buscar_arbol_pid(mi_arbol.raiz, pid_obj);
                            if (objetivo == NULL) {
                                objetivo = busqueda_secuencial_pid(&mi_lista, pid_obj);
                            }
                            if (objetivo == NULL) {
                                printf("\nError: No existe un proceso con el PID %d.\n", pid_obj);
                            } else if (op_proc == 1) {
                                suspender_proceso(objetivo);
                                if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                                printf("\nProceso PID %d (%s) suspendido (ESPERA).\n", pid_obj, objetivo->nombre);
                            } else if (op_proc == 2) {
                                if (objetivo->estado == ESPERA) {
                                    reanudar_proceso(objetivo);
                                    if (!esta_en_cola(&mi_cola_rr, objetivo->pid)) {
                                        encolar(&mi_cola_rr, objetivo);
                                    }
                                    printf("\nProceso PID %d (%s) reanudado (LISTO).\n", pid_obj, objetivo->nombre);
                                } else {
                                    printf("\nEl proceso PID %d no esta en estado ESPERA.\n", pid_obj);
                                }
                            } else if (op_proc == 3) {
                                terminar_proceso(objetivo);
                                eliminar_de_cola(&mi_cola_rr, pid_obj);
                                if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                                printf("\nExito: Forzado el cierre del proceso PID %d (%s).\n", pid_obj, objetivo->nombre);
                            }
                        }
                    } else {
                        printf("Opcion no valida.\n");
                    }
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    consola_getch();
                }
                else if (tecla == '4') {
                    consola_limpiar();
                    printf("\n========================================\n");
                    printf("       AJUSTAR QUANTUM (ROUND ROBIN)   \n");
                    printf("========================================\n");
                    printf("Quantum actual: %d ciclos\n", quantum_sistema);
                    printf("Introduce nuevo quantum (5 - 50): ");
                    int nq = quantum_sistema;
                    if (scanf("%d", &nq) == 1 && nq >= 5 && nq <= 50) {
                        quantum_sistema = nq;
                        printf("\nQuantum actualizado a %d.\n", quantum_sistema);
                    } else {
                        printf("\nValor invalido. Se mantiene el quantum en %d.\n", quantum_sistema);
                    }
                    limpiar_buffer_entrada();
                    printf("Presiona cualquier tecla para volver al monitor...");
                    consola_getch();
                }
                else if (tecla == '5') {
                    consola_limpiar();
                    printf("\n========================================\n");
                    printf("     HISTORIAL DE PROCESOS TERMINADOS   \n");
                    printf("========================================\n");
                    busqueda_secuencial_estado(&mi_lista, TERMINADO);
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    consola_getch();
                }
            }
            consola_sleep(50);
            tiempo_espera += 50;
        }

        ciclo++;
    }

    return 0;
}
