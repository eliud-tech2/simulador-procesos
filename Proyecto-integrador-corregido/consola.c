#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include "consola.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>   // Libreria clave para detectar teclas sin congelar el programa

void consola_sleep(int ms) {
    Sleep(ms);
}

int consola_kbhit(void) {
    return _kbhit();
}

int consola_getch(void) {
    return _getch();
}

void consola_limpiar(void) {
    system("cls");
}
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

void consola_sleep(int ms) {
    usleep((useconds_t)ms * 1000);
}

int consola_kbhit(void) {
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int consola_getch(void) {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void consola_limpiar(void) {
    printf("\033[2J\033[H");
}
#endif
