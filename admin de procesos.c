#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Para usar usleep()
#include <time.h>

#define MAX_PROCESOS 50    // Máximo de procesos en pantalla al mismo tiempo
#define ANCHO_BARRA 20    // Tamaño de la barra de progreso

// Nombres base para combinarlos con el PID
const char *nombres[] = {"bash", "mysql", "nginx", "python", "java", "ssh"};

// 1. Estructura súper sencilla
typedef struct {
    int pid;
    char nombre[20];
    int progreso; // Valor de 0 a 100
    int activo;   // 1 = Ocupado, 0 = Libre
} Proceso;

int main() {
    srand(time(NULL));
    Proceso tabla[MAX_PROCESOS];

    // Inicializamos todos los espacios como "libres" (0)
    for (int i = 0; i < MAX_PROCESOS; i++) {
        tabla[i].activo = 0;
    }

    int pid_actual = 1000;

    while (1) {
        // 1. Limpiar la consola (Código ANSI para borrar pantalla)
        printf("\033[H\033[J");
        printf("=== SIMULADOR DE PROCESOS (Simplificado) ===\n\n");

        // 2. Crear un proceso nuevo de forma aleatoria (30% de probabilidad)
        if (rand() % 100 < 30) {
            for (int i = 0; i < MAX_PROCESOS; i++) {
                if (tabla[i].activo == 0) { // Si hay un espacio libre...
                    tabla[i].pid = pid_actual++;
                    tabla[i].progreso = 0;  // Inicia en 0%
                    tabla[i].activo = 1;    // Lo marcamos como ocupado

                    // Elegimos un nombre al azar y le pegamos el PID (Ej: mysql_1002)
                    int total_nombres = sizeof(nombres) / sizeof(nombres[0]);
                    sprintf(tabla[i].nombre, "%s_%d", nombres[rand() % total_nombres], tabla[i].pid);

                    break; // Solo creamos uno por ciclo
                }
            }
        }

        // 3. Actualizar y dibujar los procesos activos
        for (int i = 0; i < MAX_PROCESOS; i++) {
            if (tabla[i].activo == 1) {

                // Aumentamos el progreso aleatoriamente entre 1% y 10%
                tabla[i].progreso += (rand() % 10) + 1;

                // Si ya llegó al 100%, lo liberamos
                if (tabla[i].progreso >= 100) {
                    tabla[i].progreso = 100;
                    tabla[i].activo = 0;
                }

                // --- DIBUJAR LA BARRA ---
                printf("PID: %d | %-12s | [", tabla[i].pid, tabla[i].nombre);

                // Calculamos cuántos "#" debe tener la barra
                int marcas = (tabla[i].progreso * ANCHO_BARRA) / 100;

                for (int b = 0; b < ANCHO_BARRA; b++) {
                    if (b < marcas) {
                        printf("#"); // Parte llena
                    } else {
                        printf("-"); // Parte vacía
                    }
                }
                printf("] %d%%\n", tabla[i].progreso);
            }
        }

        // 4. Pausar medio segundo antes de repetir el ciclo
        usleep(500000);
    }

    return 0;
}
