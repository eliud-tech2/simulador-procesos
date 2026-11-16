#ifndef GENERADOR_H_INCLUDED
#define GENERADOR_H_INCLUDED
#include "estructuras.h" /* Necesita conocer la Lista y la Cola para inyectar los procesos */
#include "busqueda.h"

/* Simula usuarios o programas abriendo aplicaciones al azar */
void intentar_crear_proceso_nuevo(ListaProcesos* lista, ColaListos* cola, int* contador_pid);

/* Versión integrada: registra el proceso en Lista, Cola, Árbol BST y Tabla Hash */
void intentar_crear_proceso_sistema(ListaProcesos* lista, ColaListos* cola, ArbolPID* arbol, TablaHash* th, int* contador_pid);

#endif
