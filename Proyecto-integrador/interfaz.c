#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "interfaz.h"
#include "ordenamiento.h"

static void restaurar_color() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Blanco/Gris por defecto
}

static void dibujar_barra_con_color(float porcentaje) {
    int total_bloques = 20;
    int bloques_llenos = (int)((porcentaje / 100.0f) * total_bloques);
    if (bloques_llenos > total_bloques) bloques_llenos = total_bloques;
    if (bloques_llenos < 0) bloques_llenos = 0;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (porcentaje < 40.0f) {
        SetConsoleTextAttribute(hConsole, 10); // Verde suave
    } else if (porcentaje < 70.0f) {
        SetConsoleTextAttribute(hConsole, 14); // Amarillo
    } else if (porcentaje < 88.0f) {
        SetConsoleTextAttribute(hConsole, 6);  // Naranja
    } else {
        SetConsoleTextAttribute(hConsole, 12); // Rojo peligro
    }

    printf("[");
    for (int i = 0; i < total_bloques; i++) {
        if (i < bloques_llenos) printf("|");
        else printf(" ");
    }
    printf("]");
    restaurar_color();
}

void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo) {
    system("cls");

    float cpu_total_1 = 0.0f;
    float cpu_total_2 = 0.0f;
    float ram_usada_gb = 0.8f;

    Nodo* temp = lista->cabeza;
    int activos = 0;
    while (temp != NULL) {
        if (temp->proceso->estado != TERMINADO) {
            ram_usada_gb += (temp->proceso->mem_uso / 100.0f) * 1.2f;
            if (temp->proceso->estado == EJECUCION) {
                if (activos % 2 == 0) cpu_total_1 += temp->proceso->cpu_uso;
                else cpu_total_2 += temp->proceso->cpu_uso;
            }
        }
        temp = temp->siguiente;
        activos++;
    }

    if (cpu_total_1 > 98.5f) cpu_total_1 = 52.2f + (rand() % 35);
    if (cpu_total_1 < 5.0f)  cpu_total_1 = 15.4f + (rand() % 12);
    if (cpu_total_2 > 98.5f) cpu_total_2 = 38.1f + (rand() % 20);
    if (cpu_total_2 < 3.0f)  cpu_total_2 = 10.5f + (rand() % 10);
    if (ram_usada_gb > 7.3f) ram_usada_gb = 7.15f;

    printf("========================================================================\n");
    printf(" CPU 1 "); dibujar_barra_con_color(cpu_total_1); printf(" %5.1f%%\n", cpu_total_1);
    printf(" CPU 2 "); dibujar_barra_con_color(cpu_total_2); printf(" %5.1f%%\n", cpu_total_2);

    float ram_porcentaje = (ram_usada_gb / 7.39f) * 100.0f;
    printf(" MEM  "); dibujar_barra_con_color(ram_porcentaje); printf("  %.2fG/7.39G (%5.1f%%)\n", ram_usada_gb, ram_porcentaje);
    printf("------------------------------------------------------------------------\n");
    printf(" Procesos Activos: %d | En Ejecucion (RR): %d | Reloj: Ciclo #%d\n", lista->tamano, cola->tamano, ciclo);
    printf("========================================================================\n\n");

    Proceso** arreglo = convertir_lista_a_arreglo(lista);
    if (arreglo != NULL && lista->tamano > 0) {
        quicksort_por_memoria(arreglo, 0, lista->tamano - 1);
    }

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
    printf("  PID    USUARIO     CPU%%    RAM%%    TIEMPO PROCESADO    ESTADO        COMANDO\n");
    restaurar_color();
    printf("------------------------------------------------------------------------\n");

    int limite = (lista->tamano > 20) ? 20 : lista->tamano;
    for (int i = 0; i < limite; i++) {
        if (arreglo == NULL) break;
        Proceso* p = arreglo[i];

        const char* user = (p->pid % 2 == 0) ? "root" : "eliud";

        printf("%5d    %-9s  %5.1f   %5.1f    %3d ciclos       ",
               p->pid, user, p->cpu_uso, p->mem_uso, p->tiempo_ejecucion);

        if (p->estado == EJECUCION) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
            printf("%-13s ", "[EJECUTANDO]");
        } else if (p->estado == TERMINADO) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
            printf("%-13s ", "[TERMINADO]");
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
            printf("%-13s ", "[LISTO]");
        }
        restaurar_color();

        printf("%s\n", p->nombre);
    }

    printf("\n========================================================================\n");
    printf(" OPCIONES DEL SISTEMA (Usa las teclas numericas):\n");
    printf(" [1] Buscar   - Localiza un proceso por su nombre o PID exacto.\n");
    printf(" [2] Ordenar  - Reorganiza la tabla por consumo critico de RAM.\n");
    printf(" [3] Matar    - Termina un proceso de forma forzada.\n");
    printf(" [4] Salir    - Apaga la simulacion de forma segura.\n");
    printf("========================================================================\n");

    if (arreglo != NULL) free(arreglo);
}
