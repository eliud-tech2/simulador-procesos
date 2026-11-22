#ifndef CONSOLA_H_INCLUDED
#define CONSOLA_H_INCLUDED

/* Capa de portabilidad de consola para Windows (Code::Blocks) y sistemas POSIX */
void consola_sleep(int ms);
int consola_kbhit(void);
int consola_getch(void);
void consola_limpiar(void);

#endif
