#ifndef PROCESO_H_INCLUDED
#define PROCESO_H_INCLUDED

/* Enumeración estricta para el ciclo de vida del proceso */
typedef enum {
    LISTO,
    EJECUCION,
    ESPERA,
    TERMINADO
} EstadoProceso;

/* Motivo por el cual finalizó o fue rechazado el proceso */
typedef enum {
    FIN_NINGUNO,
    FIN_COMPLETADO,     /* Terminó normalmente su ráfaga de CPU */
    FIN_FORZADO_KILL,   /* Cerrado manualmente por el usuario (6Kill) */
    FIN_OOM_KILLER,     /* Eliminado automáticamente por falta crítica de memoria */
    FIN_RECHAZADO_MEM   /* No logró entrar por Control de Admisión (memoria saturada) */
} MotivoFin;

/* TDA Proceso: Estructura base del simulador */
typedef struct {
    int pid;
    char nombre[50];
    int prioridad;        /* 0 = Alta (Cola 0), 1 = Media (Cola 1), 2 = Baja (Cola 2) */
    float cpu_uso;
    float mem_uso;
    EstadoProceso estado;
    MotivoFin motivo_fin;
    int tiempo_ejecucion;
    int tiempo_rafaga;    /* Tiempo total estimado de CPU requerido */
    int quantums_usados;  /* Cantidad de turnos (quantums) recibidos en CPU */
    int ciclos_esperando; /* Contador para envejecimiento (Aging) en colas bajas */
    int ciclo_llegada;    /* Ciclo global en el que nacio el proceso */
    int ciclo_fin;        /* Ciclo global en el que paso a TERMINADO (0 si sigue activo) */
} Proceso;

/* Prototipos de las funciones que manipulan al proceso */
Proceso* crear_proceso(int pid, const char* nombre, int prioridad, float mem_uso, int ciclo_actual);
void suspender_proceso(Proceso* p);
void reanudar_proceso(Proceso* p);
void terminar_proceso(Proceso* p, int ciclo_actual, MotivoFin motivo);
void destruir_proceso(Proceso* p); /* Crítico en C para no dejar fugas de memoria */

#endif
