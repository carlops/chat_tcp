/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo del Servidor
 *	servidor.c
 *
 *	Descripción:	Este módulo contiene todos los procedimientos relacionados 
 * 					al Servidor en la conexión a través de sockets.
 *
 *	Compilar con: make servidor
 *
 *	Correr: scs_svr -l <puerto-servidor(local)> -b <archivo_bitácora>
 *
 * 	Fecha:	11/03/2015
 *
 *	Autores:	09-10329 María Gabriela Giménez
 *				09-10672 Carlo Polisano
 *				09-10971 Alejandro Guevara
 *				10-10534 Jesús Adolfo Parra
 *
 */



/* LIBRERIAS */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "extras.h"


/* CONSTANTES */
#define TIMEFORMAT "%d/%m/%y-%I:%M:%S : "
#define MAXTAM 10


/*
	Nombre: obtenerFechaHora
	Descripción: Se encarga de buscar la fecha y hora actual.
	Retorna: String con la fecha y hora.
*/
char *obtenerFechaHora(){
    char *tiempo = (char *) malloc(sizeof(char)*25);
    if (tiempo == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(1);
	}

	time_t t;
    time(&t);
    struct Tiempo *timeInfo = localtime(&t);
    strftime(tiempo,25,TIMEFORMAT,timeInfo);
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


/* VARIABLES */
int cantusr, cantsalas, maxindexsala, maxindexusr;
Usuario **arrayusr;
char **salas;
pthread_mutex_t mutexsala  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexusr  = PTHREAD_MUTEX_INITIALIZER;


/* MAIN PRINCIPAL */
int main(int argc, char *argv[]) {
	
	/* Obtención de parámetros.
		Se obtiene los parámetros por consola y se almacena el puerto del 
		Servidor y el nombre y dirección absoluta o relativa de archivo de
		texto que realiza operaciones de bitácora.
	*/
    char *puerto = obtenerParametro("-l",argv,argc);
	char *bitacora = obtenerParametro("-b",argv,argc);

    if ((argc!=5)||!bitacora||!puerto)
        perror("Parámetros inválidos.\n");
		printf("Ingrese: scs_svr -l <puerto-servidor(local)> -b ");
		printf("<archivo_bitácora> \n");
		exit(1);

   	/* Se abre el archivo de la bitácora. */
   	FILE *fd;
    fd = fopen(bitacora,"a");
    if (fd == NULL){
	   	perror("Error abriendo la bitácora.\n");
		exit(2);
	}

	/*
		- s: 			Socket de conexión del Servidor.
   	    - my_addr:		Dirección local del Servidor.
   	    - client_addr:	
   */
   	int s;
	struct sockaddr_in my_addr;
	struct sockaddr_in client_addr;

    /* Creación del socket.
		El dominio es AF_INET, que representa IPv4.
	   	El tipo es SOCK_STREAM, que indica el servicio fiable orientado a la 
	   	conexión.
	   	El protocolo es TCP, que se indica con 0.
	*/
    s = socket(AF_INET,SOCK_STREAM,0);	
    if (s < 0){
	   	perror("Error creando el socket del Servidor.\n");
		exit(3);
	}

	/* Asignación de la dirección al socket.
	   Se usa AF_INET para indicar la version IPv4.
	   La dirección IP local se obtiene con INADDR_ANY.
	   Y se le asigna el puerto local.
	*/
    bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    my_addr.sin_port = htons(atoi(puerto));

    /* Asignación de la dirección al socket creado. */
    if ((bind(s, (struct sockaddr *) &my_addr, sizeof(my_addr))) < 0){
    	perror("Error creando el bind el Servidor.\n");
		exit(4);
    }

    /* Comienza a escuchar esperando peticiones del Cliente.
    	Puede escuchar hasta un máximo de 1024.
     */
    if ((listen(s,1024)) < 0){
    	perror("Error escuchando el socket.\n");
		exit(5);	
    }

    /* Se obtiene la fecha y hora.
    	Y se indica tanto en la bitácora como en la pantalla del Servidor.
    */
    char *tiempo = obtenerFechaHora;
    fprintf(fd,"%s Socket abierto en el puerto %s y esperando conexión..\n",tiempo,puerto);
    printf("%s Socket abierto en el puerto %s y esperando conexión..\n",tiempo,puerto);

    /*
    	- cantusr:
    	- cantsalas:
    	- arrayusr:
    	- salas:
    	- maxindexsala:
    	- maxindexusr:
    */
	cantusr = 0;
	cantsalas = 1;
	arrayusr = (infoUsr **) malloc(sizeof(infoUsr *));
	if (arrayusr == NULL){
	   	perror("Error, solicitud de memoria denegada.\n");
		exit(6);
	}
	salas = (char **) malloc(sizeof(char *));
	if (salas == NULL) {
		perror("Error, solicitud de memoria denegada.\n");
		exit(7);
	}
	maxindexsala = MAXTAM;
	maxindexusr = MAXTAM;
	salas[0] = "SALA_1";
	free(puerto);

	/* En caso de haber recibido una señal de salida forzada (cntrl + c) */
	//signal(SIGINT, salidaForzada);	


    //pthread_t hilo;
    //struct infoUsr *arg;
    //struct lista userdb

    int client_size, newfd, n;

	//Espera por clientes
	while(42) {
		client_size = sizeof(client_addr);
		newfd = accept(s,PSOCK_ADDR &client_addr,&client_size);
		if (newfd<0) {
				perror("No se pudo establecer la coneccion");
				exit(5);
		}
		tiempo = obtenerFechaHora();
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
