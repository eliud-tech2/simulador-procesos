#ifndef PROCESO_H_INCLUDED
#define PROCESO_H_INCLUDED

/* Enumeración estricta para el ciclo de vida del proceso */
typedef enum {
    LISTO,
    EJECUCION,
    ESPERA,
    TERMINADO
} EstadoProceso;

/* TDA Proceso: Estructura base del simulador */
typedef struct {
    int pid;
    char nombre[50];
    int prioridad;
    float cpu_uso;
    float mem_uso;
    EstadoProceso estado;
    int tiempo_ejecucion;
} Proceso;

/* Prototipos de las funciones que manipulan al proceso */
Proceso* crear_proceso(int pid, const char* nombre, int prioridad, float mem_uso);
void suspender_proceso(Proceso* p);
void reanudar_proceso(Proceso* p);
void terminar_proceso(Proceso* p);
void destruir_proceso(Proceso* p); /* Crítico en C para no dejar fugas de memoria */

#endif
