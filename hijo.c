
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

/* Rutina manejadora vacia para despertar al proceso tras recibir SIGALRM */
void rutina_alarma(int sig) {
    (void)sig; // Evita warning de parametro no utilizado
}

int main(int argc, char *argv[]) {
    int idx, num_verticales, num_hojas, id_shm_vert, id_shm_hoj;
    int *pids_vert, *pids_hoj;
    pid_t pid_hijo, pid_raiz;

    /* Validacion de parametros de entrada */
    if (argc != 3) {
        printf("Error en argumentos\n");
    } else {
        pid_raiz = getpid();
        num_verticales = atoi(argv[1]);
        num_hojas = atoi(argv[2]);

        /* Vector para almacenar los identificadores de los procesos verticales (x) */
        id_shm_vert = shmget(IPC_PRIVATE, sizeof(int) * num_verticales, IPC_CREAT | 0666);
        pids_vert = (int *) shmat(id_shm_vert, 0, 0);

        /* Vector para almacenar los identificadores de los procesos horizontales/hojas (y) */
        id_shm_hoj = shmget(IPC_PRIVATE, sizeof(int) * num_hojas, IPC_CREAT | 0666);
        pids_hoj = (int *) shmat(id_shm_hoj, 0, 0);

        /* Creacion de la rama vertical de procesos */
        for (idx = 1; idx <= num_verticales; idx++) {
            pid_hijo = fork();
            if (pid_hijo != 0) {
                /* El proceso padre espera a su hijo vertical */
                wait(NULL);
                break;
            } else {
                /* El nuevo proceso guarda su PID en la memoria compartida */
                pids_vert[idx - 1] = getpid();
                printf("Soy el proceso %d. Mis padres son: ", getpid());
                printf("%d", pid_raiz);

                /* Muestra los PIDs de todos los ancestros creados anteriormente */
                for (int idx_ancestro = 0; idx_ancestro < idx - 1; idx_ancestro++) {
                    printf(", %d", pids_vert[idx_ancestro]);
                }
                printf("\n");
            }
        }

        if (idx == 1) {
            /* Muestra los PIDs de las hojas finales (ejecutado por el superpadre raiz) */
            printf("Soy el super padre %d, mis hijos finales son: ", getpid());
            for (idx = 0; idx < num_hojas; idx++) {
                printf("%d", pids_hoj[idx]);
                if (idx != num_hojas - 1) {
                    printf(", ");
                }
            }
            printf("\n");

            /* Liberacion de los segmentos de memoria compartida */
            shmdt((char *)pids_vert);
            shmdt((char *)pids_hoj);
            shmctl(id_shm_vert, IPC_RMID, NULL);
            shmctl(id_shm_hoj, IPC_RMID, NULL);
        } else {
            /* El ultimo hijo vertical crea los 'y' hijos horizontales */
            if (idx == num_verticales + 1) {
                for (idx = 1; idx <= num_hojas; idx++) {
                    pid_hijo = fork();
                    if (pid_hijo == 0) {
                        /* El hijo guarda su PID, programa la alarma y espera */
                        pids_hoj[idx - 1] = getpid();
                        signal(SIGALRM, rutina_alarma);
                        alarm(10);
                        pause();
                        break;
                    }
                }
                if (idx == num_hojas + 1) {
                    /* Espera a que finalicen todos los hijos horizontales */
                    for (idx = 1; idx <= num_hojas; idx++) {
                        wait(NULL);
                    }
                }
            }
        }
    }
    return 0;
}