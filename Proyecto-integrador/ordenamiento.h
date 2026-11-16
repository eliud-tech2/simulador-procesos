#ifndef ORDENAMIENTO_H_INCLUDED
#define ORDENAMIENTO_H_INCLUDED
#include "estructuras.h"

/* Puente: Crea un arreglo de punteros a partir de la Lista Enlazada */
Proceso** convertir_lista_a_arreglo(ListaProcesos* lista);

/* 1. Quicksort: Ordena de MAYOR a MENOR uso de Memoria RAM */
void quicksort_por_memoria(Proceso** arreglo, int izquierda, int derecha);

/* 2. Radix Sort: Ordena de MENOR a MAYOR por numero de PID */
void radix_sort_por_pid(Proceso** arreglo, int total);

#endif
