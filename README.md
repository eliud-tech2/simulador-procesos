Documentación: Simulador de Administrador de Procesos en C
1. ¿Qué es este programa?
Este programa es una simulación visual de cómo el núcleo de un sistema operativo (como Linux) administra las tareas o "procesos".

En lugar de crear procesos reales en la computadora (lo cual podría saturar la memoria y colapsar la máquina real), el programa simula un entorno controlado usando un Ciclo de Eventos (Tick-based loop). Se dibuja una tabla en la consola que se actualiza en tiempo real, mostrando procesos que nacen, se procesan mostrando una barra de progreso, y mueren liberando espacio.

2. Conceptos Clave
Para entender el código, hay que conocer sus componentes principales:

A. La Configuración (Los #define)
Al principio del código están los ajustes principales. Al usar #define, hacemos que el código sea muy fácil de modificar sin tener que buscar entre toda la lógica:

LIMITE_PROCESOS: Define el tamaño de la "memoria" (cuántos procesos caben al mismo tiempo).

ANCHO_BARRA: El tamaño visual de la barra de progreso en la consola.

VELOCIDAD_MINIMA y VELOCIDAD_MAXIMA: Controlan qué tan rápido avanza el porcentaje de un proceso en cada actualización.

B. El Bloque de Control de Proceso (El struct Proceso)
En los sistemas operativos, cada proceso tiene un archivo o "ficha médica" con sus datos. Aquí lo simulamos con un struct que guarda:

pid: El número de identificación único del proceso (Process ID).

nombre: El nombre de la tarea (ej. nginx_1002).

usuario: Quién ejecuta la tarea (root o manuel).

progreso: Un número de 0 a 100 que indica cuánto le falta para terminar.

activo: Es un "interruptor". Si es 1, el espacio está ocupado. Si es 0, el espacio está libre y puede ser usado por un nuevo proceso.

3. ¿Cómo funciona el "Motor" del simulador?
El corazón del programa es un bucle infinito while(1). Piensa en este bucle como el reloj del procesador. Cada vez que da una vuelta (cada medio segundo), realiza exactamente 4 pasos lógicos:

Paso 1: Limpiar la pantalla y contar
El código usa un comando especial de consola (\033[H\033[J) que borra todo el texto al instante. Esto evita que la pantalla parpadee.

Luego, recorre la tabla de procesos y cuenta cuántos tienen el estado activo == 1.

Paso 2: Mostrar el Estado de Saturación
Si los procesos activos son iguales al LIMITE_PROCESOS, el programa anuncia que está SATURADO. Esto significa que la "memoria" está llena.

Mientras el sistema esté saturado, se le prohíbe crear procesos nuevos.

Paso 3: Nacimiento de nuevos procesos (Aleatoriedad)
Si hay espacio, el sistema tira unos "dados" virtuales usando rand() % 100 < 30. Esto significa que en cada ciclo, hay un 30% de probabilidad de que llegue un proceso nuevo.

Si los dados caen a favor, el programa busca el primer espacio que tenga activo == 0, lo cambia a 1 y le asigna un nombre y usuario al azar de las listas predefinidas.

Paso 4: Actualizar el progreso y Dibujar la Interfaz
Aquí es donde ocurre la magia visual. El programa revisa uno por uno los espacios de la tabla:

Avanzar: Si el proceso está activo, le suma un porcentaje aleatorio (basado en la velocidad mínima y máxima que configuraste arriba).

Morir: Si el progreso llega o supera el 100%, cambia su estado activo a 0. En el siguiente ciclo, ese espacio estará libre y desaparecerá de la pantalla.

Dibujar: Para los procesos que siguen vivos, usa matemáticas simples para saber cuántos signos de gato (#) imprimir y cuántos guiones (-) imprimir, formando la barra visual [#####-----].

El Latido (usleep)
Al final del ciclo, está la función usleep(500000). Esto hace que el programa "duerma" durante 500,000 microsegundos (medio segundo). Sin esta línea, el bucle giraría millones de veces por segundo y no podrías ver absolutamente nada en la pantalla. Este usleep es lo que le da la fluidez de "fotogramas por segundo" a las barras de carga.
