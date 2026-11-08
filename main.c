#include <stdio.h>
#include "proceso.h" /* Solo llamamos al menú (.h), nunca al .c */

int main() {
    printf("Iniciando Simulador del Administrador de Procesos...\n");

    /* Creamos un proceso de prueba: PID 1, nombre "init", prioridad 0, mem 15.5% */
    Proceso* p1 = crear_proceso(1, "init", 0, 15.5f);

    if (p1 != NULL) {
        printf("\n--- Proceso Creado con Exito ---\n");
        printf("PID: %d\n", p1->pid);
        printf("Nombre: %s\n", p1->nombre);
        printf("Estado Inicial: %d (0 significa LISTO)\n", p1->estado);
        printf("Uso de Memoria: %.2f%%\n", p1->mem_uso);

        /* Modificamos el estado para probar la funcion del .c */
        suspender_proceso(p1);
        printf("Nuevo estado tras suspender: %d (2 significa ESPERA)\n", p1->estado);

        /* Regla de oro: siempre destruir lo que se crea con malloc */
        destruir_proceso(p1);
        printf("\nMemoria del proceso liberada correctamente.\n");
    }

    return 0;
}
