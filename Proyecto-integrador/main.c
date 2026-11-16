#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>   // Libreria clave para detectar teclas sin congelar el programa
#include "proceso.h"
#include "estructuras.h"
#include "planificador.h"
#include "generador.h"
#include "ordenamiento.h"
#include "busqueda.h"
#include "interfaz.h"

int main() {
    srand(time(NULL));

    ListaProcesos mi_lista;
    inicializar_lista(&mi_lista);
    ColaListos mi_cola_rr;
    inicializar_cola(&mi_cola_rr);

    int quantum_sistema = 20;
    int ciclo = 1;
    int contador_pid = 100;

    while(1) {
        /* 1. Logica invisible del Sistema Operativo */
        ejecutar_round_robin(&mi_cola_rr, quantum_sistema);
        intentar_crear_proceso_nuevo(&mi_lista, &mi_cola_rr, &contador_pid);

        /* 2. Visualizacion en pantalla */
        dibujar_interfaz(&mi_lista, &mi_cola_rr, ciclo);

        /* 3. Pausa interactiva de 1.5 segundos que vigila si presionas 1, 2, 3 o 4 */
        int tiempo_espera = 0;
        while (tiempo_espera < 1500) {
            if (_kbhit()) { // Detecta si presionaste alguna tecla
                char tecla = _getch();

                if (tecla == '4') {
                    system("cls");
                    printf("Saliendo de la simulacion de forma segura...\n");
                    exit(0);
                }
                else if (tecla == '1') {
                    system("cls");
                    printf("\n========================================\n");
                    printf("          MENU DE BUSQUEDA             \n");
                    printf("========================================\n");
                    printf("Introduce el PID exacto a buscar: ");
                    int pid_buscado;
                    scanf("%d", &pid_buscado);

                    Nodo* act = mi_lista.cabeza;
                    int encontrado = 0;
                    while (act != NULL) {
                        if (act->proceso->pid == pid_buscado) {
                            printf("\n[PROCESO ENCONTRADO]\n");
                            printf(" - PID: %d\n", act->proceso->pid);
                            printf(" - Nombre: %s\n", act->proceso->nombre);
                            printf(" - RAM: %.1f%%\n", act->proceso->mem_uso);
                            printf(" - Ciclos procesados: %d\n", act->proceso->tiempo_ejecucion);
                            encontrado = 1;
                            break;
                        }
                        act = act->siguiente;
                    }
                    if (!encontrado) {
                        printf("\nNo se encontro ningun proceso con el PID %d.\n", pid_buscado);
                    }
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    _getch();
                }
                else if (tecla == '2') {
                    system("cls");
                    printf("\n========================================\n");
                    printf("            ORDENAMIENTO               \n");
                    printf("========================================\n");
                    printf("La tabla ya esta organizada dinamicamente\n");
                    printf("por consumo critico de RAM (Quicksort).\n");
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    _getch();
                }
                else if (tecla == '3') {
                    system("cls");
                    printf("\n========================================\n");
                    printf("        FORZAR CIERRE DE PROCESO       \n");
                    printf("========================================\n");
                    printf("Introduce el PID del proceso a terminar: ");
                    int pid_matar;
                    scanf("%d", &pid_matar);

                    Nodo* act = mi_lista.cabeza;
                    int encontrado = 0;
                    while (act != NULL) {
                        if (act->proceso->pid == pid_matar) {
                            act->proceso->estado = TERMINADO;
                            act->proceso->cpu_uso = 0.0f;
                            encontrado = 1;
                            printf("\nExito: El proceso PID %d (%s) fue terminado.\n", pid_matar, act->proceso->nombre);
                            break;
                        }
                        act = act->siguiente;
                    }
                    if (!encontrado) {
                        printf("\nError: No existe un proceso activo con el PID %d.\n", pid_matar);
                    }
                    printf("\nPresiona cualquier tecla para volver al monitor...");
                    _getch();
                }
            }
            Sleep(50);
            tiempo_espera += 50;
        }

        ciclo++;
    }

    return 0;
}
