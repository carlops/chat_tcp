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
	if (bind(s, (struct sockaddr *) &local, sizeof local)<0) {
		perror("Error creando el bind en el Cliente.\n");
		exit(3);
	}

	/* Iniciando la conexion.
		Se necesita la direccion IP del servidor y se obtiene mediante a una
		llamada a gethosbyname() o con gethostbyaddr() segun sea el caso.
	 */
	if (inet_aton(host,&inaddr))
			servidor = gethostbyaddr((char *) &inaddr, sizeof(inaddr), AF_INET);
	else
			servidor = gethostbyname(host);
	if (servidor==NULL) {
		perror("Error obteniendo nombre de dominio o direccion.\n");
		exit(4);
	}
	bzero(&serv,sizeof(serv));
	serv.sin_family = AF_INET;
	memcpy(&serv.sin_addr, servidor->h_addr_list[0], servidor->h_length);
	/* Llenando los ultimos campos de serv */
	serv.sin_port = htons(atoi(puerto));
	/* Conectando */
	if (connect(s, (struct sockaddr *) &serv, sizeof(serv))<0) {
			perror("Error en la conexion al servidor.\n");
			exit(5);
	}
	/* Si hemos llegado hasta aqui, la conexion esta establecida */
	//if (write(s, DATOS, sizeof DATOS)<0) {
	//		perror("escribiendo el socket:");
	//		exit(3);
	//}
	/* Todo bien. Podemos cerrar el socket y terminar */
	close(s);
	exit(0);
}
