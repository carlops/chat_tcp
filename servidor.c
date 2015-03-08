/*
 * Servidor
 * compilar con: (-g es opcional para debuggear)
 * gcc -pthread -g -o scs_svr servidor.c extras.c
 *
 * Se corre asi:
 * scs_svr -l <puerto-servidor(local)> -b <archivo_bitácora>
 *
 */

#define PSOCK_ADDR (struct sockaddr *)
#define TIMEFORMAT "%d/%m/%y-%I:%M:%S : "
#define EXTRA_ARREGLO 10

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "extras.h"
#include <time.h>

/* Variables Globales */
int cantusr, cantsalas, maxindexsala, maxindexusr;
Usuario **arrayusr;
char **salas;
pthread_mutex_t mutexsala  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexusr  = PTHREAD_MUTEX_INITIALIZER;

char *get_time(){
    time_t t;
    char *tiempo;
    struct tm * timeinfo;

    tiempo=(char *) malloc(sizeof(char)*25);
    time(&t);
    timeinfo = localtime(&t);
    strftime(tiempo,25,TIMEFORMAT,timeinfo);
    return tiempo;
}

struct Usuario {
    pthread_t id;
    int fd;
	char *nombre;
	int posarray;
    char **salas;//falta el tipo
};

typedef struct Usuario infoUsr;

void *manejadorHilo(void *arg); 

struct lista{
    char name[50];
    struct userdb *sig;
};

int main(int argc, char *argv[]) {
    
    struct sockaddr_in my_addr, client_addr; 
	int listenfd, e, client_size, newfd, n;
    pthread_t hilo;
    struct infoUsr *arg;
	FILE *fd;
    char *tiempo;
    struct lista userdb;  

    char *puerto = obtenerParametro("-l",argv,argc);
	char *bitacora = obtenerParametro("-b",argv,argc);
    if ((argc!=5)||!bitacora||!puerto)
        {perror("Parametros invalidos"); exit(-1);}
   
    fd = fopen(bitacora,"a");
    if (fd==NULL){
		   	perror("Error abriendo bitacora");
			exit(1);
	}

    // Inicializacion Socket
    listenfd = socket(AF_INET,SOCK_STREAM,0);	
    if (listenfd== -1){
		   	perror("Error abriendo socket");
			exit(2);
	}
    
    bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    my_addr.sin_port = htons(atoi(puerto));
    e = bind(listenfd,PSOCK_ADDR &my_addr, sizeof(my_addr));
    if (e!=0){
		   	perror("Error enlazando el socket");
			exit(3);
	}
    e = listen(listenfd,1024);
    if (e==-1){
		   	perror("Error escuchando el socket");
			exit(4);
	}

    tiempo = get_time();
    fprintf(fd,"%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);
    printf("%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);

    //Aqui se inicializan las variables globales como cantidad de salas de chats,de usuarios activos, etc
	cantusr = 0;
	cantsalas = 1;
	arrayusr = (infoUsr **) malloc(sizeof(infoUsr *));
	if(arrayusr==NULL){
		   	perror("Error en solicitud de memoria");
			exit(12);
	}
	salas = (char **) malloc(sizeof(char *));
	if(salas==NULL) {
			perror("Error en solicitud de memoria");
			exit(13);
	}
	maxindexsala = EXTRA_ARREGLO;
	maxindexusr = EXTRA_ARREGLO;
	salas[0] = "Sala01";
	free(puerto);
	//signal(SIGINT, salidaForzadaServidor);	


	//Espera por clientes
	while(1) {
			client_size= sizeof(client_addr);
			newfd = accept(listenfd,PSOCK_ADDR &client_addr,&client_size);
			if (newfd<0) {
					perror("No se pudo establecer la coneccion");
					exit(5);
			}
			tiempo = get_time();
			fprintf(fd,"%sNuevo usuario conectado al servidor\n",tiempo);
			printf("%sNuevo usuario conectado al servidor\n",tiempo);

			if (!(arg=(struct infoUsr*) malloc(sizeof(struct infoUsr)))){
					perror("Error en asignacion de memoria");
					exit(6);
			}
		//	arg->id=0;
			arg->fd=newfd;
			//conectarse
			//login();
		//	char username[100];
		//	n = recv(newfd,username,100,0);
			//buscar en userdb   


			if (pthread_create(&(arg->id),NULL,&manejadorHilo,(void *) arg)){
					perror("Error en la creacion del hilo");
					exit(7);
			}
			//close(newfd);
	} 

}

void *manejadorHilo(void *arg) {
		infoUsr *usr = (infoUsr *) arg;
		pthread_t id = (*usr).id;
		crearUsuario(usr);
		char* buffer;
		char** sep; 
		int ok=0;

		while (1){ //espera por el input del cliente

				leer_socket((*usr).fd,  &buffer);
				if(buffer==NULL) continue;
				sep = split(buffer);
				free(buffer);
				ejecutar_peticion((*usr).posarray,sep);
				if(!strcmp(sep[0],"fue")) break;
				free(sep[0]);
				free(sep);

		}

		free(sep[0]);
		free(sep);
		pthread_cancel(id);
		if(ok!=0) {
				perror("No se pudo cancelar el hilo.");
				exit(7);
		}
}

void crearUsuario(infoUsr *usr) {

		ed_mutex_lock(&mutexusr);

		char *buffer, *error_encontrado;
		int  tam=0, encontrado=0, i=0, ok;
		leer_socket(usr->fd, &buffer);

		while((i<cantusr)&&(!encontrado)){

				encontrado = !strcmp(buffer,(*arrayusr[i]).nombre);
				i++;
		}

		if(encontrado){

				error_encontrado = (char *) malloc(sizeof(char)* 5);
				if(error_encontrado==NULL){ 
						perror("Error en solicitud de memoria");
						exit(8);
				}
				escribir_socket(usr->fd, error_encontrado);
				free(error_encontrado); 
				error_encontrado = error_handler(6);
				escribir_socket(usr->fd, error_encontrado);
				free(error_encontrado);
				ok = close(usr->fd);
				if (ok<0){
						perror("No se cerro socket correctamente");
						exit(8);
				}
				pthread_mutex_unlock(&mutexusr);
				ok = pthread_cancel(usr->id);
				if(ok!=0){ 
						perror("No se pudo cancelar el hilo");
						exit(8);
				}

		}  

		escribir_socket(usr->fd, "1");
		usr->nombre = buffer;
		usr->salas = (char **) malloc(sizeof(char *));
		if(usr->salas==NULL){
				perror("Error en solicitud de memoria");
				exit(9);
		}
		usr->salas[0] = (char *) malloc(sizeof(char)*strlen(salas[0]));
		if (usr->salas[0]==NULL){
				perror("Error en solicitud de memoria");
				exit(10);
		}
		strcpy(usr->salas[0], salas[0]);
		//usr->cantsalas = 1;
		//usr->maxindexsala = EXTRA_ARREGLO;

		if(cantusr<maxindexusr){

				arrayusr[cantusr]= usr;
				usr->posarray = cantusr++;

		} else {

				arrayusr = (infoUsr **) realloc(arrayusr,sizeof(infoUsr *)*
								(EXTRA_ARREGLO+maxindexusr));
				maxindexusr += EXTRA_ARREGLO;
				arrayusr[cantusr]= usr;
				usr->posarray = cantusr++;

		}

		char *aviso = (char *) malloc(sizeof(char)*100);
		if(aviso==NULL){
				perror("Error en solicitud de memoria");
				exit(11);
		}
		sprintf(aviso,"Su solicitud ha sido aceptada, ha ingresado a la sala '%s' por defecto\0",salas[0]);
		incluir_verificacion(1, &aviso);
		escribir_socket(usr->fd, aviso);
		free(aviso);
		pthread_mutex_unlock(&mutexusr);
}

void eliminarUsuario() {

}

void crearSala(){
		//ver si ya existe

		//agregar a saladb
}

void eliminarSala(){
		//expulsar usuarios activos en la sala

		//eliminar de saladb
}

void verSala(){

}

void verUsuarios(){

}

void habilitarSala(){

}

void deshabilitarSala(){

}

void verBitacora(){

}
