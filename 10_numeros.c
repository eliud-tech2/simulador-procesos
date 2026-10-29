#include <stdio.h>

int main(void) {
    double numero, suma = 0.0, mayor, menor;

    printf("Ingresa el numero 1: ");
    if (scanf("%lf", &numero) != 1) {
        printf("Error: Entrada invalida.\n");
        return 1;
    }

    mayor = numero;
    menor = numero;
    suma += numero;

    for (int i = 2; i <= 10; i++) {
        printf("Ingresa el numero %d: ", i);
        if (scanf("%lf", &numero) != 1) {
            printf("Error: Entrada invalida.\n");
            return 1;
        }

        suma += numero;

        if (numero > mayor) {
            mayor = numero;
        }
        if (numero < menor) {
            menor = numero;
        }
    }

    double promedio = suma / 10;

    printf("\n--- Resultados ---\n");
    printf("Suma:     %.2lf\n", suma);
    printf("Promedio: %.2lf\n", promedio);
    printf("Mayor:    %.2lf\n", mayor);
    printf("Menor:    %.2lf\n", menor);

    return 0;
}
