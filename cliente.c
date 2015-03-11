/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo del Cliente
 *	cliente.c
 *
 *	Descripción:	Este módulo contiene todos los procedimientos relacionados 
 * 					al Cliente en la conexión a través de sockets.
 *
 *	Compilar con: make
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
#include <signal.h>
#include "extras.h"


/* VARIABLES */
pthread_t idthread;
int idfd;

/* METODOS */

/*
	Nombre: salidaForzada
	Descripción: Se encarga de anunciarle al usuario cuando intenta salir del
					programa ejecutando ctrl + c que es incorrecto y que si
					desea salir del programa ejecute el comando "salir".
*/
void salidaForzada(){
    printf("servidor@servidor: Salida inválida. Intente el comando 'salir'.\n");
}

/*	Nombre: hiloUsuario
	Descripción: Se encarga de llamar el hilo de un usuario y así esperar los 
					mensajes destinados a ese usuario.
	Parámetros:	- arg: Cast para cumplir con los requerimientos de un hilo.
*/
void *hiloUsuario(void *arg){
	char *msj;
	int *i = (int *) arg;
	int s = *i, idfd = *i, servidorActivo = 42;

	while(42){
		leerSocket(s, &msj);
		servidorActivo = verificar(&msj);
		if(!servidorActivo) break;
		if (msj == NULL) break;
		printf("%s\n",msj);  
		free(msj);
	}

	if(servidorActivo)
		pthread_exit(0);

	exit(0);
}


/* MAIN PRINCIPAL */
int main(int argc, char *argv[]) {

	/* Obtención de parámetros.
		Se obtiene los parámetros por consola y se almacena
		el host, el puerto del Servidor y el puerto local del Cliente.
		Siendo este último opcional.
	*/
	char *host = obtenerParametro("-d",argv,argc);
	char *puerto = obtenerParametro("-p",argv,argc);
	char *puerto_local = obtenerParametro("-l",argv,argc);

	if(((argc!=7)||!host||!puerto||!puerto_local)&&((argc!=5)||!host||!puerto)){
		perror("Parámetros inválidos.\n");
		printf("Ingrese: scs_cli -d <nombre_módulo_atención> -p ");
		printf("<puerto_scs_svr> [-l <puerto_local>]\n");
		exit(1);
	}

	/*
		- s: 		Socket de conexión del Cliente.
		- local: 	Dirección local del Clinete.
		- serv: 	Dirección del Servidor.
		- servidor: Utilizado para obtener el la dirección del Servidor.
		- IPDir:	Utilizado cuando se suministra el dominio como dirección
					IP.
	*/
	int s;
	struct sockaddr_in local;
	struct sockaddr_in serv;
	struct hostent *servidor;
	struct in_addr IPDir;

	/* Creación del socket.
	   	El dominio es AF_INET, que representa IPv4.
	   	El tipo es SOCK_STREAM, que indica el servicio fiable orientado a la 
	   	conexión.
	   	El protocolo es TCP, que se indica con 0.
	*/
	s = socket(AF_INET,SOCK_STREAM,0);
	if (s < 0) {
		perror("Error creando el socket del Cliente.\n");
		exit(2);
	}

	/* Asignación de la dirección local al socket.
	   	Se usa AF_INET para indicar la version IPv4.
	   	En el caso del Cliente, el número de puerto local es suministrado con 
	   	la bandera '-l', este parámetro es opcional. 
	   	En el caso de que no sea suministrado, podemos dejar que lo elija el 
	   	sistema, dandole el valor cero. 
	   	La dirección IP local se obtiene con INADDR_ANY.
	*/
	local.sin_family = AF_INET;
	if (puerto_local)
		local.sin_port = htons(atoi(puerto_local));
	else
		local.sin_port = htons(0);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Asignación de la dirección al socket creado.
	   	No es necesario ejecutar el bind() en el Cliente cuando no es 
	   	suministrado el puerto local. Solo es necesario cuando te lo dan, ya
	   	que si se intenta hacer directamente connect(), sin haber realizado 
	   	previamente bind(), el sistema asignará automaticamente una dirección 
	   	local elegida por él. 
	   	Pero sin importar el caso, siempre se ejecutará.
	*/
	if (bind(s, (struct sockaddr *) &local, sizeof local) < 0) {
		perror("Error creando el bind en el Cliente.\n");
		exit(3);
	}

	/* Iniciando la conexión.
	   	Se necesita la dirección IP del Servidor y se obtiene mediante a una
	   	llamada a gethosbyname() o con gethostbyaddr() según sea el caso.
	   	Se usa AF_INET para indicar la versión IPv4.
	   	Se copia el host en el struct del Servidor.
	   	Se indica el puerto por donde se escuchará al Servidor.
	*/
	if (inet_aton(host,&IPDir))
		servidor = gethostbyaddr((char *) &IPDir, sizeof(IPDir), AF_INET);
	else
		servidor = gethostbyname(host);
	if (servidor == NULL) {
		perror("Error obteniendo nombre o dirección del dominio.\n");
		exit(4);
	}
	bzero(&serv,sizeof(serv));
	serv.sin_family = AF_INET;
	memcpy(&serv.sin_addr, servidor->h_addr_list[0], servidor->h_length);
	serv.sin_port = htons(atoi(puerto));

	/* Conectando con el Servidor. */
	if (connect(s, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
		perror("Error en la conexión al servidor.\n");
		exit(5);
	}

	/* Se realizó la conexión con el Servidor. */


	/* Se le pide al usuario su nombre de usuario en la sala de chat.
	   	El tamaño del nombre de usuario es hasta 50 caracteres.
	*/
	printf("Introduzca su nombre de usuario:\n");
	char *usuario = (char *) malloc (sizeof(char)*50);
	scanf("%s",usuario);

	/* Se manda el primer mensaje de creación por el socket.
		Se envia primero el tamaño del nombre del usuario y luego el
		nombre del usuario. 
	*/
	escribirSocket(s,usuario);

	/* Se lee el primer mensaje del socket.
		Lee primero el tamaño del mensaje y luego mensaje. 
	*/
	char* msj;  
	int ok = 1, finalizacion = 0;
	leerSocket(s,&msj);
	if (msj == NULL) ok = 0;

	/* Se recibió la señal de finalización.
		Esto ocurre cuando el mensaje (msj) recibido a través del socket
		del Cliente tiene un tamaño igual a 0. 
	*/
	if (!ok){
		leerSocket(s,&msj);
		printf("%s",msj);
		finalizacion = close(s);
		if (finalizacion == -1){ 
			perror("Error, no se cerró correctamente el socket del Cliente.\n");
			exit(6);
		}
		free(host);
		free(puerto);
		free(usuario);
		free(puerto_local);
		exit(0);
	}

	free(msj);

	/* En caso de haber recibido una señal de salida forzada (cntrl + c) */
	signal(SIGINT, salidaForzada);

	/* Creación de hilo para el usuario.
	 	Este hilo es para escuchar los mensajes dirigidos a este usuario.
	*/
	if (pthread_create(&idthread,NULL,hiloUsuario, (void *) &s)){
		perror("Error, en la creación del hilo en usuario.\n");
		exit(7);
	}

	/* Ciclo infinito donde el usuario interactua mediante comandos.
	 	Estos comandos son enviados al servidor invocando la función 
	 	escribirSocket.
	 	Se finaliza el ciclo cuando el usuario escribe el comando 'salir'
	*/
	char *comando, **nombreComando;
	while (ok){
		comando = (char *) malloc(sizeof(char)*200);
		if (comando == NULL){
			perror("Error, solicitud de memoria denegada.\n");
			exit(8);
		}
		sleep(1);
		scanf(" %[^\n]\0",comando);
		if (comando == NULL) continue;
		escribirSocket(s,comando);
		nombreComando = separar(comando);
		ok = strcmp(nombreComando[0],"salir");
		free(comando);
		free(nombreComando[0]);
		if (nombreComando[1] != NULL) free(nombreComando[1]);
		free(nombreComando);
	}

	/* Se termina la conexión y se cierra el socket para terminar. */
	sleep(1);  
	free(host);
	free(puerto);
	free(usuario);
	free(puerto_local);

	finalizacion = pthread_join(idthread, 0);
	if (finalizacion != 0){ 
		perror("Error, no se pudo eliminar el hilo.\n");
		exit(9);
	}

	finalizacion = close(s);
	if (finalizacion == -1){ 
		perror("Error, no se cerró correctamente el socket del Cliente.\n");
		exit(9);
	}
	exit(0);
}
