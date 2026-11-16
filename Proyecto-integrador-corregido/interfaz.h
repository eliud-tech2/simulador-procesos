#ifndef INTERFAZ_H_INCLUDED
#define INTERFAZ_H_INCLUDED
#include "estructuras.h"
#include "planificador.h"
#include "ordenamiento.h"

/* Modos de visualización inspirados en Linux, Windows y Android */
typedef enum {
    VISTA_LINUX,
    VISTA_WINDOWS,
    VISTA_ANDROID
} ModoVista;

/* Dibuja la pantalla completa estilo htop usando la lista actual */
void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo);

/* Dibuja la interfaz en el modo seleccionado (Linux / Windows / Android) */
void dibujar_interfaz_completa(ListaProcesos* lista, ColaListos* cola, int ciclo,
                               ModoVista vista, AlgoritmoPlanificacion planificador,
                               AlgoritmoOrden alg_orden, CriterioOrden crit_orden,
                               int quantum);

#endif
