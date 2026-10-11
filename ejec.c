/*
Gonzalo de la Cueva del Rio
48784462J
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


 
//  Variables globales
 

pid_t pidejec;
pid_t pida;
pid_t pidb;
pid_t pidx;
pid_t pidy;
pid_t pidz;

char car_proceso_objetivo;
int segundos;


//prototipos
void analizar_args(int argc, char *argv[]);

void man_ini_destruccion(int s);

void crear_proc_a(void);
void ejec_proc_a(void);
void man_a_tarea(int s);
void man_a_destruir(int s);

void crear_proc_b(void);
void ejec_proc_b(void);
void man_b_tarea(int s);
void man_b_destruir(int s);

void crear_proc_x(void);
void ejec_proc_x(void);
void man_x_tarea(int s);
void man_x_destruir(int s);

void crear_proc_y(void);
void ejec_proc_y(void);
void man_y_tarea(int s);
void man_y_destruir(int s);

void crear_proc_z(void);
void ejec_proc_z(void);
void man_z_alarma(int s);
void man_z_destruir(int s);

/*
 * Comprueba que se reciben dos argumentos y guarda 
 * el proceso objetivo como el numero de segundos en variables globales.
 */
void analizar_args(int argc, char *argv[]) {
    //comprobamos el numero correcto de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <proceso> <segundos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // guardamos la letra y el número de segundos
    car_proceso_objetivo = argv[1][0];
    segundos = atoi(argv[2]);
    //vemos si ha introducido correctamente el nombre del proceso
    if (car_proceso_objetivo != 'A' && car_proceso_objetivo != 'B' &&
        car_proceso_objetivo != 'X' && car_proceso_objetivo != 'Y') {
        fprintf(stderr, "Proceso objetivo invalido.\n");
        exit(EXIT_FAILURE);
    }
}


// el proceso ejec es el padre de todos
int main(int argc, char *argv[]) {
    analizar_args(argc, argv);
    //getpid() funcion de llamada al sistema
    pidejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pidejec);

    /* SIGUSR2 indica que la tarea ha terminado y comienza el apagado. */
    signal(SIGUSR2, man_ini_destruccion);	// ejec no ejecuta tareas, solo espera para la destruccion.
    //(tipo de señal, funcion a ejecutar)
    /* Crea A y espera a que toda su rama termine. */
    crear_proc_a();
    //cuando A muera
    printf("Soy ejec (%d) y muero\n", pidejec);
    return 0;
}


/*
ejec recibe SIGUSR2 cuando el proceso objetivo ha terminado de ejecutar
su comando. La destruccion se propaga comenzando por A donde luego hara una
eliminacion en cascada, primero los hijos y luego los padres.
 */
void man_ini_destruccion(int s) {
    (void)s;
    kill(pida, SIGUSR2);
}

//PROCESO A
/*
 * Crea A. El proceso ejec queda esperando hasta que A haya terminado,
 * garantizando que el padre no muera antes que su hijo.
 */
void crear_proc_a(void) {	// este codigo lo hace ejec
    switch (pida = fork()) {
        case 0:
            ejec_proc_a();
            exit(0);

        default:
        //para congelar al padre (ejec)
            wait(NULL);	// aqui espera ejec
    }
}


/*
 * Inicializacion y vida de A. A instala los manejadores de ejecucion y
 * destruccion y continua la construccion del arbol creando B.
 */
void ejec_proc_a(void) {
    pida = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", pida, pidejec);
    // me preparo para dos formas de despertarme.
    signal(SIGUSR1, man_a_tarea);
    signal(SIGUSR2, man_a_destruir);
    crear_proc_b();
}


/*
 * SIGUSR1 en A: crea un hijo temporal para ejecutar pstree.
 * A permanece intacto. Al terminar el comando se notifica a ejec mediante
 * SIGUSR2 para iniciar la fase de destruccion.
 */
void man_a_tarea(int s) {
    pid_t pid;
    //entero del numero de señal de kernel
    //(void) porque no lo vamos a utilizar
    (void)s;
    printf("Soy el proceso A con %d, he recibido la senyal.\n", pida);
    pid = fork();
    if (pid == 0) {
        //sustituye la memoria del hijo por pstree
        execlp("pstree", "pstree", (char *)NULL);
        //en caso de que execlp falle
        exit(EXIT_FAILURE); 
    }
    
    wait(NULL); // espero a que termine mi hijo (reconvertido a comando)
    kill(pidejec, SIGUSR2); //  ejec empezara la destruccion.
}


/*
 * A propaga SIGUSR2 a B y no termina hasta que B haya finalizado.
 */
void man_a_destruir(int s) {
    (void)s;
    //kill --> envia una señal , no es matar como cual
    kill(pidb, SIGUSR2);
    wait(NULL);
    //cuando B muere se desbloquea el wait y muere A
    printf("Soy A (%d) y muero\n", pida);
    exit(0);
}

//PROCESO B
/*
 * Crea B. A espera a que B termine, igual que ejec espera a A.
 */
void crear_proc_b(void) {
    switch (pidb = fork()) {
        case 0:
            ejec_proc_b();
            exit(0);

        default:
        //para congelar al padre(en este cas a ) hasta que el hijo muera
            wait(NULL);
    }
}


/*
 * B instala sus manejadores y crea sus tres hijos X, Y y Z.
 * Despues queda suspendido esperando senales.
 */
void ejec_proc_b(void) {
    pidb = getpid();

    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           pidb, pida, pidejec);

    signal(SIGUSR1, man_b_tarea);
    signal(SIGUSR2, man_b_destruir);

    crear_proc_x();
    crear_proc_y();
    crear_proc_z();

    while (1) {
        pause();
    }
}


/*
 * SIGUSR1 en B: ejecuta pstree mediante un hijo temporal.
 */
void man_b_tarea(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso B con %d, he recibido la senyal.\n", pidb);

    pid = fork();

    if (pid == 0) {
        execlp("pstree", "pstree", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pidejec, SIGUSR2);
}


/*
 * B es padre de X, Y y Z. Para reproducir el orden de destruccion mostrado
 * en el enunciado, destruye y espera primero a Z, despues a Y y por ultimo
 * a X. Solo entonces termina B.
 */
void man_b_destruir(int s) {
    (void)s;

    kill(pidz, SIGUSR2);
    wait(NULL);

    kill(pidy, SIGUSR2);
    wait(NULL);

    kill(pidx, SIGUSR2);
    wait(NULL);

    printf("Soy B (%d) y muero\n", pidb);
    exit(0);
}

//PROCESO X
/*
 * B crea X y continua inmediatamente para poder crear tambien Y y Z.
 * No se hace wait() aqui porque los tres hijos deben coexistir.
 */
void crear_proc_x(void) {
    switch (pidx = fork()) {
        case 0:
            ejec_proc_x();
            exit(0);
    }
}


/*
 * X queda a la espera de SIGUSR1 (ejecutar ls) o SIGUSR2 (terminar).
 */
void ejec_proc_x(void) {
    pidx = getpid();

    printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n",
           pidx, pidb, pida);

    signal(SIGUSR1, man_x_tarea);
    signal(SIGUSR2, man_x_destruir);

    while (1) {
        pause();
    }
}


/*
 * SIGUSR1 en X: ejecuta ls en un hijo temporal y, al finalizar,
 * notifica a ejec con SIGUSR2.
 */
void man_x_tarea(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso X con %d, he recibido la senyal.\n", pidx);

    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pidejec, SIGUSR2);
}


/* X es una hoja: al recibir SIGUSR2 simplemente termina. */
void man_x_destruir(int s) {
    (void)s;

    printf("Soy X (%d) y muero\n", pidx);
    exit(0);
}

//PROCESO Y
/* B crea Y y continua inmediatamente para crear Z. */
void crear_proc_y(void) {
    switch (pidy = fork()) {
        case 0:
            ejec_proc_y();
            exit(0);
    }
}


/*
 * Y queda a la espera de SIGUSR1 (ejecutar ls) o SIGUSR2 (terminar).
 */
void ejec_proc_y(void) {
    pidy = getpid();

    printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n",
           pidy, pidb, pida);

    signal(SIGUSR1, man_y_tarea);
    signal(SIGUSR2, man_y_destruir);

    while (1) {
        pause();
    }
}


/* SIGUSR1 en Y: ejecuta ls mediante un hijo temporal. */
void man_y_tarea(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso Y con %d, he recibido la senyal.\n", pidy);

    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pidejec, SIGUSR2);
}


/* Y es una hoja: al recibir SIGUSR2 simplemente termina. */
void man_y_destruir(int s) {
    (void)s;

    printf("Soy Y (%d) y muero\n", pidy);
    exit(0);
}

//PROCESO Z
/*
 * Z se crea despues de X e Y. De esta forma hereda de B los PID ya
 * almacenados de X e Y, ademas de los PID de A y B.
 */
void crear_proc_z(void) {
    //para crear el proceso
    switch (pidz = fork()) {
        case 0:
            ejec_proc_z();
            exit(0);
    }
}


/*
 * Z implementa la temporizacion sin sleep(). Registra SIGALRM, programa
 * alarm(segundos) y se suspende con pause() hasta recibir senales.
 */
void ejec_proc_z(void) {
    pidz = getpid();

    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n", pidz, pidb, pida);

    signal(SIGALRM, man_z_alarma);
    signal(SIGUSR2, man_z_destruir);

    alarm(segundos);

    while (1) {
        pause();
    }
}


/*
 * Cuando vence la alarma, Z envia SIGUSR1 al proceso indicado por el
 * primer argumento del programa.
 */
void man_z_alarma(int s) {
    (void)s;
    //La letra que nos han pasado por argumento
    switch (car_proceso_objetivo) {
        case 'A':
        //SIGUSR1 --> Para ejecutar la tarea
            kill(pida, SIGUSR1);
            break;

        case 'B':
            kill(pidb, SIGUSR1);
            break;

        case 'X':
            kill(pidx, SIGUSR1);
            break;

        case 'Y':
            kill(pidy, SIGUSR1);
            break;
    }
}


/* Z es una hoja: al recibir SIGUSR2 simplemente termina. */
void man_z_destruir(int s) {
    (void)s;

    printf("Soy Z (%d) y muero\n", pidz);
    exit(0);
}