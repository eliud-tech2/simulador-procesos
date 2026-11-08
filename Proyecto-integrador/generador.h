#ifndef GENERADOR_H_INCLUDED
#define GENERADOR_H_INCLUDED
#include "estructuras.h" /* Necesita conocer la Lista y la Cola para inyectar los procesos */

/* Simula usuarios o programas abriendo aplicaciones al azar */
void intentar_crear_proceso_nuevo(ListaProcesos* lista, ColaListos* cola, int* contador_pid);

#endif
