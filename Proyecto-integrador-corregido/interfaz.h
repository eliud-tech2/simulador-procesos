#ifndef INTERFAZ_H_INCLUDED
#define INTERFAZ_H_INCLUDED
#include "estructuras.h"
#include "ordenamiento.h"

/* Dibuja la pantalla completa estilo htop usando la lista actual */
void dibujar_interfaz(ListaProcesos* lista, ColaListos* cola, int ciclo);

/* Dibuja la interfaz estilo htop de Linux con el criterio Heapsort y quantum activos */
void dibujar_interfaz_htop(ListaProcesos* lista, ColaListos* cola, int ciclo,
                           CriterioOrden crit_orden, int quantum);

#endif
