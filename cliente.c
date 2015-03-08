/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo del Cliente
 *	cliente.c
 *
 *	Descripción:	Este módulo contiene todos los procedimientos relacionados 
 * 					al Cliente en la conexión a través de sockets.
 *
 *	Compilar con: make cliente
 *
 *	Correr: scs_cli -d <IP-servidor> -p <puerto-servidor> [-l <puerto_local>]
 *				(Lo que se encuentra entre [] es opcional)
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "extras.h"


/* Variables */
pthread_t idthread;
int idfd;

/*
 * hilo_usuario
 *  Rutina utilizada en el hilo de usuario, para
 *   esperar los mensajes a este usuario
 */
void *hilo_usuario(void *arg){

		int *aux = (int *) arg;
		int fd = *aux, idfd=*aux, servidorcontinue=1;
		char *buffer;

		while(42){

				leer_socket(fd, &buffer);
				servidorcontinue = leer_verificacion(&buffer);
				if(!servidorcontinue) break;
				if(buffer==NULL) break;
				printf("%s\n",buffer);  
				free(buffer);
		}


		if(servidorcontinue)
				pthread_exit(0);

		exit(0);
}

/* MAIN PRINCIPAL */
int main(int argc, char *argv[]) {

		char *host = obtenerParametro("-d",argv,argc);
		char *puerto = obtenerParametro("-p",argv,argc);
		char *puerto_local = obtenerParametro("-l",argv,argc);

		if ((argc%2!=1)||(argc>7)||!host||!puerto){
				perror("Parametros invalidos.");
				printf("Ingrese: scs_cli -d <nombre_módulo_atención> -p ");
				printf("<puerto_scs_svr> [-l <puerto_local>]\n");
				exit(1);
		}


		int s;					  // El socket de conexion
		struct sockaddr_in local; // Direccion local
		struct sockaddr_in serv;  // Direccion del servidor
		struct hostent *servidor; // Para gethostbyname() o gethostbyaddr()
		//  segun sea el caso
		struct in_addr inaddr;	  // Utilizado cuando es suministrada el dominio
		//  como direccion IP


		/* Creacion del socket.
		   El dominio es AF_INET, que representa IPv4.
		   El tipo es SOCK_STREAM, que indica el servicio fiable orientado a la 
		   conexion.
		   El protocolo es TCP, que se indica con 0.
		   */
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s<0) {
				perror("Error creando el socket en el Cliente.\n");
				exit(2);
		}

		/* Asignacion de direccion local al socket.
		   Se usa AF_INET para indicar la version IPv4.
		   En el caso del cliente, el numero de puerto local es suministrado con 
		   la bandera '-l', este parametro es opcional. 
		   En el caso de que no sea suministrado, podemos dejar que lo elija el 
		   sistema, dandole el valor cero. 
		   La direccion IP local se obtiene con INADDR_ANY.
		   */
		local.sin_family=AF_INET;
		if (puerto_local)
				local.sin_port=htons(atoi(puerto_local));
		else
				local.sin_port=htons(0);
		local.sin_addr.s_addr = htonl(INADDR_ANY);

		/* Asignacion de la direccion al socket creado.
		   No es necesario ejecutar el bind() en el cliente cuando no es 
		   suministrado el puerto local. Solo es necesario cuando te lo dan, ya
		   que si se intenta hacer directamente connect(), sin haber realizado 
		   previamente bind(), el sistema asignara automaticamente una direccion 
		   local elegida por el. 
		   Pero sin importar el caso, siempre se ejecutara.
		   */
		if (bind(s, (struct sockaddr *) &local, sizeof local) < 0) {
				perror("Error creando el bind en el Cliente.\n");
				exit(3);
		}

		/* Iniciando la conexion.
		   Se necesita la direccion IP del servidor y se obtiene mediante a una
		   llamada a gethosbyname() o con gethostbyaddr() segun sea el caso.
		   Se usa AF_INET para indicar la version IPv4.
		   Se copia el host en el struct del servidor.
		   Se indica el puerto por donde se escuchara al servidor.
		   */
		if (inet_aton(host,&inaddr))
				servidor = gethostbyaddr((char *) &inaddr, sizeof(inaddr), AF_INET);
		else
				servidor = gethostbyname(host);
		if (servidor == NULL) {
				perror("Error obteniendo nombre o direccion del dominio.\n");
				exit(4);
		}
		bzero(&serv,sizeof(serv));
		serv.sin_family = AF_INET;
		memcpy(&serv.sin_addr, servidor->h_addr_list[0], servidor->h_length);
		serv.sin_port = htons(atoi(puerto));

		/* Conectando con el Servidor. */
		if (connect(s, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
				perror("Error en la conexion al servidor.\n");
				exit(5);
		}

		/* Se realizo la conexion con el Servidor. */

		/* Se le pide al cliente el nombre de su usuario.
		   El tamaño del nombre de usuario es hasta 50 caracteres.
		   */
		printf("Introduzca su nombre de usuario:\n");
		char *usuario =  (char *) malloc (sizeof(char)*50);
		scanf("%s",usuario);

		/* Se procede a mandar el primer mensaje de creacion */
		escribir_socket(s, usuario);
		char* buffer;  
		int ok=1, finalizacion=0;
		leer_socket(s, &buffer);
		if (buffer==NULL) ok=0;

		/* Se recibio la senal de finalizacion */
		if (!ok){
				leer_socket(s, &buffer);
				printf("%s",buffer);
				finalizacion = close(s);
				if(finalizacion==-1){ 
						perror("No se cerro correctamente el socket");
						exit(6);
				}
				free(host);
				free(puerto);
				free(usuario);
				free(puerto_local);
				exit(0);

		}

		free(buffer);

		/* signal en caso de haber recibido una senal de salida forzada (cntrl + c) */
		//signal(SIGINT,salidaForzadaCliente);

		/* Creacion de hilo para usuario
		 * Este hilo es para escuchar los mensajes dirigidos a este usuario
		 */
		int rc=0;
		if(rc=pthread_create(&idthread,NULL,hilo_usuario, (void *) &s)){
				perror("Error en la creacion del hilo en usuario");
				exit(7);
		}

		/*
		 * Ciclo infinito donde el usuario interactua mediante comandos
		 * los cuales son enviados al servidor invocando la funcion
		 * escribir_socket.
		 * 
		 * Se finaliza el ciclo cuando el usuario escribe el comando 'salir'
		 */
		char *comando, **sep;
		while(ok){

				comando = (char *) malloc(sizeof(char)*200);
				if(comando==NULL){
						perror("Solicitud de memoria denegada");
						exit(8);
				}
				sleep(1);
				printf("comando@comando~: "); 
				scanf(" %[^\n]\0", comando);
				if(comando==NULL) continue;
				escribir_socket(s, comando);
				sep = split(comando);
				ok = strcmp(sep[0],"salir");
				free(comando);
				free(sep[0]);
				free(sep[1]);
				free(sep);
		}

		/* Se termina la conexion y se cierra el socket para terminar. */
		sleep(1);  
		free(host);
		free(puerto);
		free(usuario);
		free(puerto_local);
		finalizacion = pthread_join(idthread, 0);
		if(finalizacion!=0){ 
				perror("No se pudo eliminar el hilo.");
				exit(9);
		}
		finalizacion = close(s);
		if(finalizacion==-1){ 
				perror("No se cerro correctamente el socket");
				exit(9);
		}
		exit(0);
}
