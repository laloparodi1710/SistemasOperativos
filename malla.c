//Gonzalo de la Cueva del Rio
//48784462J
#include <stdio.h>
#include <sys/wait.h> //wait()
#include <unistd.h> // para el fork(),
// alarm()y pause()
#include <stdlib.h>//atoi() y exit()

void vacio(){
}
//argc numero total de argumentos 
//argv vector con los argumentos
int main(int argc, char *argv[]){
	int x, y; //numero de filas y columnas
	int pid; // resultado fork()
	
	if(argc == 3){
		int i, j; // iteradores
		x = atoi(argv[1]);
		y = atoi(argv[2]); //pasamos de cadenas a enteros
		if(x > 0 && y > 0){ //vemos que son valores positivos
			for(i = 1; i <= y; i++){
				//el primer proceso padre ya existe en memoria( programa ejecutado en la terminal)
				//el malla de la explicación
				pid = fork(); //crear hijo con valor 0
				if(pid == 0){ // para el hijo recien creado
					for(j = 1; j <= x - 1; j++){
						pid = fork(); // se ejecutan dos procesos independientes
						// padre con pid != 0 y e hijo con el pid == 0
						if(pid !=  0){ // entra el padre
							wait(NULL); //esperamos a que el hijo acabe su proceso
							exit(0);
						}
						// el hijo no entra y en la siguiente iteracion
						// se convierte en padre (en caso de que tengamos que tener mas hijos)
					}
					//final de la cadena vertical
					if(j == x){
						signal(SIGALRM, vacio); //para cuando se de el aviso
						//funcion nada() encargada de manejar la señal SIGALRM
						alarm(10); //temporizador 10 segundos
						pause(); // ejecucion del proceso en espera pasiva
						//pasan 10 seg, reaccion en cadena muriendo hacia arriba
						exit(0);
					}
				}
			}
			if(i == y + 1){
				for(i = 1; i <= y; i++){
					wait(NULL);
				}
			}
		}
	}

	return 0;
}


