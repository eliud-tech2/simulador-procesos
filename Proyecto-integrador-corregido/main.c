#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>   // Libreria clave para detectar teclas sin congelar el programa
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
static void Sleep(int ms) { usleep((useconds_t)ms * 1000); }
static int _kbhit(void) {
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) { ungetc(ch, stdin); return 1; }
    return 0;
}
static int _getch(void) {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif
#include "proceso.h"
#include "estructuras.h"
#include "planificador.h"
#include "generador.h"
#include "ordenamiento.h"
#include "busqueda.h"
#include "interfaz.h"

static void limpiar_consola(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

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
    printf(" - Prioridad: %d\n", p->prioridad);
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

    ModoVista vista_actual = VISTA_LINUX;
    AlgoritmoPlanificacion planificador_actual = ALGORITMO_RR;
    AlgoritmoOrden orden_algoritmo = ORDEN_QUICKSORT;
    CriterioOrden orden_criterio = CRITERIO_MEMORIA;

    /* Semilla inicial de procesos para que la tabla arranque poblada */
    for (int i = 0; i < 5; i++) {
        intentar_crear_proceso_sistema(&mi_lista, &mi_cola_rr, &mi_arbol, &mi_hash, &contador_pid);
    }

    while (1) {
        /* 1. Logica invisible del Sistema Operativo */
        ejecutar_planificador(&mi_cola_rr, planificador_actual, quantum_sistema, &proceso_en_cpu);
        intentar_crear_proceso_sistema(&mi_lista, &mi_cola_rr, &mi_arbol, &mi_hash, &contador_pid);

        /* 2. Visualizacion en pantalla */
        dibujar_interfaz_completa(&mi_lista, &mi_cola_rr, ciclo,
                                  vista_actual, planificador_actual,
                                  orden_algoritmo, orden_criterio, quantum_sistema);

        /* 3. Pausa interactiva de 1.5 segundos que vigila si presionas 1..6 */
        int tiempo_espera = 0;
        while (tiempo_espera < 1500) {
            if (_kbhit()) { // Detecta si presionaste alguna tecla
                char tecla = (char)_getch();

                if (tecla == '6') {
                    limpiar_consola();
                    printf("Liberando memoria y saliendo de la simulacion de forma segura...\n");
                    liberar_cola(&mi_cola_rr);
                    liberar_arbol(mi_arbol.raiz);
                    liberar_tabla_hash(&mi_hash);
                    liberar_lista(&mi_lista, 1);
                    return 0;
                }
                else if (tecla == '1') {
                    limpiar_consola();
                    printf("\n========================================\n");
                    printf("          MENU DE BUSQUEDA             \n");
                    printf("========================================\n");
                    printf("1. Buscar por PID (Busqueda Binaria + Radix)\n");
                    printf("2. Buscar por PID (Arbol Binario de Busqueda)\n");
                    printf("3. Buscar por Nombre (Tabla Hash)\n");
                    printf("4. Buscar por Estado (Busqueda Secuencial)\n");
                    printf("5. Buscar por Prioridad (Busqueda Secuencial)\n");
                    printf("Elige metodo: ");
                    int op_busq = 0;
                    if (scanf("%d", &op_busq) != 1) op_busq = 0;
                    limpiar_buffer_entrada();

                    clock_t t_ini = clock();
                    if (op_busq == 1) {
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
                    } else if (op_busq == 2) {
                        int pid_buscado = 0;
                        printf("Introduce el PID exacto a buscar en el BST: ");
                        if (scanf("%d", &pid_buscado) == 1) {
                            limpiar_buffer_entrada();
                            t_ini = clock();
                            Proceso* res = buscar_arbol_pid(mi_arbol.raiz, pid_buscado);
                            if (res != NULL) mostrar_detalle_proceso(res);
                            else printf("\nNo se encontro el PID %d en el Arbol BST.\n", pid_buscado);
                        }
                    } else if (op_busq == 3) {
                        char nombre_buscado[50];
                        printf("Introduce el nombre exacto (ej. spotify, gcc, python): ");
                        if (fgets(nombre_buscado, sizeof(nombre_buscado), stdin) != NULL) {
                            nombre_buscado[strcspn(nombre_buscado, "\r\n")] = '\0';
                            t_ini = clock();
                            buscar_hash_nombre(&mi_hash, nombre_buscado);
                        }
                    } else if (op_busq == 4) {
                        int est = 0;
                        printf("Estado (0=LISTO, 1=EJECUCION, 2=ESPERA, 3=TERMINADO): ");
                        if (scanf("%d", &est) == 1 && est >= 0 && est <= 3) {
                            limpiar_buffer_entrada();
                            t_ini = clock();
                            busqueda_secuencial_estado(&mi_lista, (EstadoProceso)est);
                        }
                    } else if (op_busq == 5) {
                        int prio = 0;
                        printf("Prioridad a buscar (0=Baja, 1=Normal, 2=Alta, 3=Critica): ");
                        if (scanf("%d", &prio) == 1) {
                            limpiar_buffer_entrada();
                            t_ini = clock();
                            busqueda_secuencial_prioridad(&mi_lista, prio);
                        }
                    } else {
                        printf("Opcion no valida.\n");
                    }
                    clock_t t_fin = clock();
                    double ms = ((double)(t_fin - t_ini) / CLOCKS_PER_SEC) * 1000.0;
                    printf("\n[Rendimiento] Tiempo de busqueda: %.4f ms\n", ms);
                    printf("Presiona cualquier tecla para volver al monitor...");
                    _getch();
                }
                else if (tecla == '2') {
                    limpiar_consola();
                    printf("\n========================================\n");
                    printf("            ORDENAMIENTO               \n");
                    printf("========================================\n");
                    printf("Algoritmo:\n 1. Quicksort\n 2. Shellsort\n 3. Radix Sort\nElige algoritmo: ");
                    int op_alg = 1;
                    if (scanf("%d", &op_alg) == 1) {
                        if (op_alg == 1) orden_algoritmo = ORDEN_QUICKSORT;
                        else if (op_alg == 2) orden_algoritmo = ORDEN_SHELLSORT;
                        else if (op_alg == 3) orden_algoritmo = ORDEN_RADIX;
                    }
                    limpiar_buffer_entrada();

                    printf("\nCriterio:\n 1. RAM%%\n 2. CPU%%\n 3. Prioridad\n 4. Tiempo ejecutado\n 5. PID\nElige criterio: ");
                    int op_crit = 1;
                    if (scanf("%d", &op_crit) == 1) {
                        if (op_crit == 1) orden_criterio = CRITERIO_MEMORIA;
                        else if (op_crit == 2) orden_criterio = CRITERIO_CPU;
                        else if (op_crit == 3) orden_criterio = CRITERIO_PRIORIDAD;
                        else if (op_crit == 4) orden_criterio = CRITERIO_TIEMPO;
                        else if (op_crit == 5) orden_criterio = CRITERIO_PID;
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
                    _getch();
                }
                else if (tecla == '3') {
                    limpiar_consola();
                    printf("\n========================================\n");
                    printf("        GESTION DE PROCESOS            \n");
                    printf("========================================\n");
                    printf("1. Crear nuevo proceso manual\n");
                    printf("2. Suspender proceso (Pasar a ESPERA)\n");
                    printf("3. Reanudar proceso (Pasar a LISTO)\n");
                    printf("4. Matar / Terminar proceso\n");
                    printf("Elige accion: ");
                    int op_proc = 0;
                    if (scanf("%d", &op_proc) != 1) op_proc = 0;
                    limpiar_buffer_entrada();

                    if (op_proc == 1) {
                        char nom[50];
                        int prio = 1;
                        float ram = 5.0f;
                        printf("Nombre del proceso: ");
                        if (fgets(nom, sizeof(nom), stdin) != NULL) {
                            nom[strcspn(nom, "\r\n")] = '\0';
                            if (strlen(nom) == 0) strcpy(nom, "proceso_usr");
                        }
                        printf("Prioridad (0=Baja, 1=Normal, 2=Alta, 3=Critica): ");
                        if (scanf("%d", &prio) != 1) prio = 1;
                        printf("Uso inicial de RAM%% (ej. 8.5): ");
                        if (scanf("%f", &ram) != 1) ram = 5.0f;
                        limpiar_buffer_entrada();

                        Proceso* p = crear_proceso_manual(&mi_lista, &mi_cola_rr, &mi_arbol, &mi_hash, &contador_pid, nom, prio, ram);
                        if (p != NULL) {
                            printf("\nExito: Proceso '%s' creado con PID %d.\n", p->nombre, p->pid);
                        }
                    } else if (op_proc >= 2 && op_proc <= 4) {
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
                            } else if (op_proc == 2) {
                                suspender_proceso(objetivo);
                                if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                                printf("\nProceso PID %d (%s) suspendido (ESPERA).\n", pid_obj, objetivo->nombre);
                            } else if (op_proc == 3) {
                                if (objetivo->estado == ESPERA) {
                                    reanudar_proceso(objetivo);
                                    if (!esta_en_cola(&mi_cola_rr, objetivo->pid)) {
                                        encolar(&mi_cola_rr, objetivo);
                                    }
                                    printf("\nProceso PID %d (%s) reanudado (LISTO).\n", pid_obj, objetivo->nombre);
                                } else {
                                    printf("\nEl proceso PID %d no esta en estado ESPERA.\n", pid_obj);
                                }
                            } else if (op_proc == 4) {
                                terminar_proceso(objetivo);
                                eliminar_de_cola(&mi_cola_rr, pid_obj);
                                if (proceso_en_cpu == objetivo) proceso_en_cpu = NULL;
                                printf("\nExito: El proceso PID %d (%s) fue terminado.\n", pid_obj, objetivo->nombre);
                            }
                        }
                    } else {
                        printf("Opcion no valida.\n");
                    }
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    _getch();
                }
                else if (tecla == '4') {
                    limpiar_consola();
                    printf("\n========================================\n");
                    printf("       ALGORITMO DE PLANIFICACION      \n");
                    printf("========================================\n");
                    printf("1. Round Robin (RR)\n");
                    printf("2. Planificacion por Prioridad\n");
                    printf("3. Shortest Job First (SJF)\n");
                    printf("4. Cambiar Quantum (actual: %d)\n", quantum_sistema);
                    printf("Elige opcion: ");
                    int op_plan = 0;
                    if (scanf("%d", &op_plan) == 1) {
                        if (op_plan == 1) planificador_actual = ALGORITMO_RR;
                        else if (op_plan == 2) planificador_actual = ALGORITMO_PRIORIDAD;
                        else if (op_plan == 3) planificador_actual = ALGORITMO_SJF;
                        else if (op_plan == 4) {
                            printf("Nuevo quantum (5 - 50): ");
                            int nq = 20;
                            if (scanf("%d", &nq) == 1 && nq >= 5 && nq <= 50) {
                                quantum_sistema = nq;
                            }
                        }
                    }
                    limpiar_buffer_entrada();
                    printf("\nConfiguracion de planificador actualizada.\n");
                    printf("Presiona cualquier tecla para volver al monitor...");
                    _getch();
                }
                else if (tecla == '5') {
                    limpiar_consola();
                    printf("\n========================================\n");
                    printf("         SELECCIONAR INTERFAZ          \n");
                    printf("========================================\n");
                    printf("1. Linux (top / htop)\n");
                    printf("2. Windows (Administrador de tareas)\n");
                    printf("3. Android (Procesos y segundo plano)\n");
                    printf("Elige vista: ");
                    int op_v = 1;
                    if (scanf("%d", &op_v) == 1) {
                        if (op_v == 1) vista_actual = VISTA_LINUX;
                        else if (op_v == 2) vista_actual = VISTA_WINDOWS;
                        else if (op_v == 3) vista_actual = VISTA_ANDROID;
                    }
                    limpiar_buffer_entrada();
                }
            }
            Sleep(50);
            tiempo_espera += 50;
        }

        ciclo++;
    }

    return 0;
}
