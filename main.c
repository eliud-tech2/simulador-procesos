#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Para usar usleep()
#include <time.h>

// ==========================================
// CONFIGURACIÓN DEL ADMINISTRADOR
// ==========================================
#define LIMITE_PROCESOS 50 // <--- CAMBIA ESTE NÚMERO PARA SATURAR MÁS RÁPIDO O MÁS LENTO
#define ANCHO_BARRA 20    // Tamaño de la barra de progreso
// ==========================================

const char *nombres[] = {"bash", "mysql", "nginx", "python", "java", "ssh"};
const char *usuarios[] = {"root", "manuel"};

typedef struct {
    int pid;
    char nombre[20];
    char usuario[20];
    int progreso;
    int activo;
} Proceso;

int main() {
    srand(time(NULL));

    // Creamos la tabla de procesos basada en tu configuración de arriba
    Proceso tabla[LIMITE_PROCESOS];

    for (int i = 0; i < LIMITE_PROCESOS; i++) {
        tabla[i].activo = 0;
    }

    int pid_actual = 1000;

    while (1) {
        printf("\033[H\033[J");
        printf("=== SIMULADOR DE PROCESOS ===\n\n");

        // 1. CONTAR CUÁNTOS PROCESOS ESTÁN ACTIVOS
        int procesos_activos = 0;
        for (int i = 0; i < LIMITE_PROCESOS; i++) {
            if (tabla[i].activo == 1) {
                procesos_activos++;
            }
        }

        // 2. MOSTRAR ESTADO DE SATURACIÓN
        if (procesos_activos == LIMITE_PROCESOS) {
            printf("[!] ESTADO: SATURADO (%d/%d) - Esperando liberacion de memoria...\n\n", procesos_activos, LIMITE_PROCESOS);
        } else {
            printf("[OK] ESTADO: Estable (%d/%d)\n\n", procesos_activos, LIMITE_PROCESOS);
        }

        printf("PID    | USUARIO  | PROCESO      | PROGRESO\n");
        printf("----------------------------------------------------------\n");

        // 3. CREAR PROCESO ALEATORIO (Solo si no está saturado)
        if (procesos_activos < LIMITE_PROCESOS && rand() % 100 < 30) {
            for (int i = 0; i < LIMITE_PROCESOS; i++) {
                if (tabla[i].activo == 0) {
                    tabla[i].pid = pid_actual++;
                    tabla[i].progreso = 0;
                    tabla[i].activo = 1;

                    int total_nombres = sizeof(nombres) / sizeof(nombres[0]);
                    sprintf(tabla[i].nombre, "%s_%d", nombres[rand() % total_nombres], tabla[i].pid);

                    int total_usuarios = sizeof(usuarios) / sizeof(usuarios[0]);
                    sprintf(tabla[i].usuario, "%s", usuarios[rand() % total_usuarios]);

                    break;
                }
            }
        }

        // 4. ACTUALIZAR Y DIBUJAR
        for (int i = 0; i < LIMITE_PROCESOS; i++) {
            if (tabla[i].activo == 1) {

                tabla[i].progreso += (rand() % 1) + 1;

                if (tabla[i].progreso >= 100) {
                    tabla[i].progreso = 100;
                    tabla[i].activo = 0; // Libera el espacio cuando termina
                }

                printf("%-6d | %-8s | %-12s | [", tabla[i].pid, tabla[i].usuario, tabla[i].nombre);

                int marcas = (tabla[i].progreso * ANCHO_BARRA) / 100;
                for (int b = 0; b < ANCHO_BARRA; b++) {
                    if (b < marcas) printf("0");
                    else printf("-");
                }
                printf("] %d%%\n", tabla[i].progreso);
            }
        }

        usleep(500000);
    }

    return 0;
}
