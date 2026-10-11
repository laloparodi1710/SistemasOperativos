#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


/* Declaracion de la funcion encargada de dividir el archivo */
void fragmentar_fichero(char *ruta_fichero, int tam_bloque);

int main(int argc, char *argv[]) {
    /* Validacion de argumentos recibidos desde la consola */
    if (argc != 3) {
        printf("Error. Uso: %s fichero tam_trozo\n", argv[2]);
    } else {
        /* Inicia el proceso de division pasando la ruta y el tamano del bloque */
        fragmentar_fichero(argv[1], atoi(argv[2]));
    }
    return 0;
}

void fragmentar_fichero(char ruta_fichero[], int tam_bloque) {
    int idx_fragmento;
    int tubo[2]; /* tubo de comunicacion entre padre e hijos */
    char nombre_fragmento[50];
    int fd_origen, fd_destino;
    struct stat info_fichero;
    struct stat *p_info = &info_fichero; /* Puntero a la estructura de estado del archivo */
    int total_fragmentos;
    int bytes_leidos;
    char *buffer_bloque;

    /* Reserva del vector de memoria dinamica para leer los bloques */
    buffer_bloque = (char *) malloc(sizeof(char) * tam_bloque);

    /* Abre el archivo fuente en modo lectura */
    fd_origen = open(ruta_fichero, O_RDONLY);
    if (fd_origen < 0) {
        printf("Error. No existe el fichero\n");
    } else {
        /* Obtiene la informacion del fichero fuente */
        stat(ruta_fichero, p_info);

        /* Calcula el numero total de trozos */
        total_fragmentos = p_info->st_size / tam_bloque;
        if (p_info->st_size % tam_bloque != 0) {
            total_fragmentos++; /* Trozo adicional para los bytes sobrantes */
        }

        /* Bucle para la creacion de hijos y envio de fragmentos */
        for (idx_fragmento = 0; idx_fragmento < total_fragmentos; idx_fragmento++) {
            pipe(tubo); /* Crea una tuberia distinta para cada hijo */

            if (fork() != 0) {
                /* PROCESO PADRE: Lee del archivo original y escribe en la tuberia */
                bytes_leidos = read(fd_origen, buffer_bloque, tam_bloque);
                write(tubo[1], buffer_bloque, bytes_leidos);
            } else {
                /* PROCESO HIJO: Escribe en el archivo de salida */
                if (idx_fragmento < 10) {
                    sprintf(nombre_fragmento, "%s.h0%d", ruta_fichero, idx_fragmento);
                } else {
                    sprintf(nombre_fragmento, "%s.h%d", ruta_fichero, idx_fragmento);
                }

                fd_destino = creat(nombre_fragmento, 0666);
                bytes_leidos = read(tubo[0], buffer_bloque, tam_bloque);
                write(fd_destino, buffer_bloque, bytes_leidos);

                close(fd_destino);
                free(buffer_bloque);
                break; /* El hijo rompe el bucle para no seguir creando procesos */
            }
        }

        if (idx_fragmento == total_fragmentos) {
            /* PROCESO PADRE: Libera el buffer y espera a la finalizacion de los hijos */
            free(buffer_bloque);
            for (int k = 0; k < total_fragmentos; k++) {
                wait(NULL);
            }
        }
    }
}