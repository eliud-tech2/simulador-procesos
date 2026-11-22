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

static void mostrar_detalle_proceso(Proceso* p, int ciclo_actual) {
    const char* est = "S (Sleeping / Listo)";
    if (p->estado == EJECUCION) est = "R (Running / Ejecutando)";
    else if (p->estado == ESPERA) est = "T (Stopped / Suspendido)";
    else if (p->estado == TERMINADO) {
        if (p->motivo_fin == FIN_OOM_KILLER) est = "TERMINADO [OOM-KILLER]";
        else if (p->motivo_fin == FIN_RECHAZADO_MEM) est = "TERMINADO [RECHAZADO MEM]";
        else if (p->motivo_fin == FIN_FORZADO_KILL) est = "TERMINADO [KILL MANUAL]";
        else est = "TERMINADO [COMPLETADO]";
    }

    int fin_ref = (p->ciclo_fin > 0) ? p->ciclo_fin : ciclo_actual;
    int retorno = fin_ref - p->ciclo_llegada;
    if (retorno < 0) retorno = 0;

    printf("\n[PROCESO LOCALIZADO]\n");
    printf(" - PID: %d | Cola: Q%d | Command: /usr/bin/%s | State: %s\n",
           p->pid, p->prioridad, p->nombre, est);
    printf(" - CPU%%: %.1f%% | MEM%%: %.1f%% | Ciclos CPU: %d/%d | Quantums usados: %d\n",
           p->cpu_uso, p->mem_uso, p->tiempo_ejecucion, p->tiempo_rafaga, p->quantums_usados);
    printf(" - Llegada: Ciclo #%d | Fin: %s%d | Tiempo en sistema: %d ciclos\n",
           p->ciclo_llegada, (p->ciclo_fin > 0) ? "Ciclo #" : "Activo en #", fin_ref, retorno);
}

int main() {
    srand((unsigned int)time(NULL));

    ListaProcesos mi_lista;
    inicializar_lista(&mi_lista);

    /* 3 Colas FIFO para Planificación de Colas Múltiples con Prioridad (0=Alta, 1=Media, 2=Baja) */
    ColaListos colas[NUM_COLAS_PRIORIDAD];
    for (int i = 0; i < NUM_COLAS_PRIORIDAD; i++) {
        inicializar_cola(&colas[i]);
    }

    ArbolPID mi_arbol;
    inicializar_arbol(&mi_arbol);
    TablaHash mi_hash;
    inicializar_tabla_hash(&mi_hash);

    int quantum_sistema = 10;       /* Quantum base de Q0 (Q1=20, Q2=40) */
    int ciclos_envejecimiento = 10; /* Cada 10 ciclos esperando sin turno sube un nivel de prioridad */
    int ciclo = 1;
    int contador_pid = 100;
    Proceso* proceso_en_cpu = NULL;
    CriterioOrden orden_criterio = CRITERIO_MEMORIA;

    float ram_gb = 0.80f;
    float swp_gb = 0.00f;
    int admision_abierta = 1;
    char mensaje_oom[120] = "";

    /* Semilla inicial de procesos para que la tabla arranque poblada */
    for (int i = 0; i < 5; i++) {
        intentar_crear_proceso_sistema(&mi_lista, colas, &mi_arbol, &mi_hash, &contador_pid, ciclo, 1);
    }

    while (1) {
        /* 1. Llegadas y envejecimiento ANTES de despachar (prioridad estricta Q0->Q1->Q2) */
        intentar_crear_proceso_sistema(&mi_lista, colas, &mi_arbol, &mi_hash, &contador_pid,
                                       ciclo, admision_abierta);
        aplicar_envejecimiento(&mi_lista, colas, ciclos_envejecimiento);
        ejecutar_planificador_multicola(colas, quantum_sistema, &proceso_en_cpu, ciclo);
        admision_abierta = verificar_memoria_y_oom(&mi_lista, colas, &proceso_en_cpu, ciclo,
                                                   &ram_gb, &swp_gb, mensaje_oom, sizeof(mensaje_oom));

        /* 2. Visualizacion en pantalla (modo Linux htop) */
        dibujar_interfaz_htop(&mi_lista, colas, ciclo, orden_criterio, quantum_sistema,
                              ram_gb, swp_gb, admision_abierta, mensaje_oom);

        /* 3. Pausa interactiva de 1.5 segundos que vigila las teclas de la barra htop */
        int tiempo_espera = 0;
        while (tiempo_espera < 1500) {
            if (consola_kbhit()) { // Detecta si presionaste alguna tecla
                char tecla = (char)consola_getch();

                if (tecla == '0') {
                    consola_limpiar();
                    printf("Liberando memoria y saliendo de htop de forma segura...\n");
                    for (int i = 0; i < NUM_COLAS_PRIORIDAD; i++) {
                        liberar_cola(&colas[i]);
                    }
                    liberar_arbol(mi_arbol.raiz);
                    liberar_tabla_hash(&mi_hash);
                    liberar_lista(&mi_lista, 1);
                    return 0;
                }
                else if (tecla == '1') {
                    printf("\n[Search] 1=Nombre(Hash)  2=PID(Binaria+Heapsort): ");
                    int op_busq = 0;
                    if (scanf("%d", &op_busq) != 1) op_busq = 0;
                    limpiar_buffer_entrada();

                    clock_t t_ini = clock();
                    if (op_busq == 1) {
                        char nombre_buscado[50];
                        printf("Nombre exacto (ej. spotify, gcc, python): ");
                        if (fgets(nombre_buscado, sizeof(nombre_buscado), stdin) != NULL) {
                            nombre_buscado[strcspn(nombre_buscado, "\r\n")] = '\0';
                            t_ini = clock();
                            buscar_hash_nombre(&mi_hash, nombre_buscado);
                        }
                    } else if (op_busq == 2) {
                        int pid_buscado = 0;
                        printf("PID exacto a buscar: ");
                        if (scanf("%d", &pid_buscado) == 1) {
                            limpiar_buffer_entrada();
                            t_ini = clock();
                            Proceso** arr = convertir_lista_a_arreglo(&mi_lista);
                            heapsort_procesos(arr, mi_lista.tamano, CRITERIO_PID);
                            Proceso* res = busqueda_binaria_pid(arr, mi_lista.tamano, pid_buscado);
                            if (arr != NULL) free(arr);
                            if (res != NULL) mostrar_detalle_proceso(res, ciclo);
                            else printf("\nNo se encontro ningun proceso con el PID %d.\n", pid_buscado);
                        }
                    } else {
                        printf("Opcion no valida.\n");
                    }
                    clock_t t_fin = clock();
                    double ms = ((double)(t_fin - t_ini) / CLOCKS_PER_SEC) * 1000.0;
                    printf("[Rendimiento] Tiempo de busqueda: %.4f ms | Presiona una tecla...", ms);
                    consola_getch();
                    break;
                }
                else if (tecla == '2') {
                    printf("\n[SortBy - Heapsort] Columna (1=MEM%%, 2=CPU%%, 3=TIME+, 4=PID): ");
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
                        heapsort_procesos(arr, mi_lista.tamano, orden_criterio);
                    }
                    clock_t t_fin = clock();
                    if (arr != NULL) free(arr);

                    double ms = ((double)(t_fin - t_ini) / CLOCKS_PER_SEC) * 1000.0;
                    printf("Heapsort (%d procesos) en %.4f ms | Presiona una tecla...", mi_lista.tamano, ms);
                    consola_getch();
                    break;
                }
                else if (tecla == '3') {
                    printf("\n[State] 1=Suspender(T)  2=Reanudar(S): ");
                    int op_est = 0;
                    if (scanf("%d", &op_est) != 1) op_est = 0;
                    limpiar_buffer_entrada();

                    if (op_est == 1 || op_est == 2) {
                        int pid_obj = 0;
                        printf("PID del proceso: ");
                        if (scanf("%d", &pid_obj) == 1) {
                            limpiar_buffer_entrada();
                            Proceso* objetivo = buscar_arbol_pid(mi_arbol.raiz, pid_obj);
                            if (objetivo == NULL) objetivo = busqueda_secuencial_pid(&mi_lista, pid_obj);

                            if (objetivo == NULL) {
                                printf("Error: No existe el PID %d.\n", pid_obj);
                            } else if (op_est == 1) {
                                eliminar_de_cola(&colas[objetivo->prioridad], objetivo->pid);
                                suspender_proceso(objetivo);
                                if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                                printf("PID %d (%s) -> State T (Suspendido).\n", pid_obj, objetivo->nombre);
                            } else if (op_est == 2) {
                                if (objetivo->estado == ESPERA) {
                                    reanudar_proceso(objetivo);
                                    objetivo->ciclos_esperando = 0;
                                    if (!esta_en_cola(&colas[objetivo->prioridad], objetivo->pid)) {
                                        encolar(&colas[objetivo->prioridad], objetivo);
                                    }
                                    printf("PID %d (%s) -> State S (Reanudado al final de Q%d).\n",
                                           pid_obj, objetivo->nombre, objetivo->prioridad);
                                } else {
                                    printf("El PID %d no esta suspendido.\n", pid_obj);
                                }
                            }
                        }
                    }
                    printf("Presiona una tecla para continuar...");
                    consola_getch();
                    break;
                }
                else if (tecla == '4') {
                    printf("\n[Quantum Base Q0] Actual=%d (Q1=%d, Q2=%d) | Nuevo quantum base (5 - 25): ",
                           quantum_sistema, quantum_sistema * 2, quantum_sistema * 4);
                    int nq = quantum_sistema;
                    if (scanf("%d", &nq) == 1 && nq >= 5 && nq <= 25) {
                        quantum_sistema = nq;
                        printf("Quantums actualizados -> Q0=%d, Q1=%d, Q2=%d.\n",
                               quantum_sistema, quantum_sistema * 2, quantum_sistema * 4);
                    } else {
                        printf("Valor invalido. Se mantiene en Q0=%d.\n", quantum_sistema);
                    }
                    limpiar_buffer_entrada();
                    printf("Presiona una tecla para continuar...");
                    consola_getch();
                    break;
                }
                else if (tecla == '5') {
                    consola_limpiar();
                    printf("==========================================================================\n");
                    printf("HISTORIAL DE PROCESOS FINALIZADOS\n");
                    printf("==========================================================================\n");
                    busqueda_secuencial_estado(&mi_lista, TERMINADO);
                    printf("\nPresiona cualquier tecla para volver a htop...");
                    consola_getch();
                    break;
                }
                else if (tecla == '6') {
                    int pid_kill = 0;
                    printf("\n[Kill - SIGTERM] Introduce el PID para forzar cierre: ");
                    if (scanf("%d", &pid_kill) == 1) {
                        limpiar_buffer_entrada();
                        Proceso* objetivo = buscar_arbol_pid(mi_arbol.raiz, pid_kill);
                        if (objetivo == NULL) objetivo = busqueda_secuencial_pid(&mi_lista, pid_kill);

                        if (objetivo != NULL && objetivo->estado != TERMINADO) {
                            terminar_proceso(objetivo, ciclo, FIN_FORZADO_KILL);
                            eliminar_de_cola(&colas[objetivo->prioridad], pid_kill);
                            if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                            printf("Senal SIGTERM enviada: PID %d (%s) finalizado.\n", pid_kill, objetivo->nombre);
                        } else {
                            printf("Error: No existe un proceso activo con PID %d.\n", pid_kill);
                        }
                    } else {
                        limpiar_buffer_entrada();
                    }
                    printf("Presiona una tecla para continuar...");
                    consola_getch();
                    break;
                }
                else if (tecla == '7') {
                    int agregados = inyectar_carga_estres(&mi_lista, colas, &mi_arbol, &mi_hash,
                                                          &contador_pid, ciclo, 8);
                    printf("\n[Stress] Inyectados %d procesos pesados para probar Swap y OOM Killer...\n", agregados);
                    consola_sleep(700);
                    break;
                }
            }
            consola_sleep(50);
            tiempo_espera += 50;
        }

        ciclo++;
    }

    return 0;
}
