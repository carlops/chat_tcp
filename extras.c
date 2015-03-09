/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo Extra
 *	extras.c
 *
 *	Descripción:	Este módulo contiene métodos extras que facilitan la
 * 					ejecución de los módulos del Cliente y Servidor.
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
#include <string.h>
#include "extras.h"


/*
    Nombre: obternerParametro
    Descripción: Se encarga de obtener el argumento que corresponde a un cierto 
                    flag de la entrada.
    Argumentos: - flag: String que puede ser -d, -p, -l.
                - argv: Arreglo que representa los argumentos introducidos.
                - argc: Entero que representa el número total de argumentos.
    Retorno: String con el argumento correspondiente al flag indicado.
*/
char *obtenerParametro(char *flag,char *argv[], int argc){
    int cont = 1;
    while ((cont < argc) && (strcmp(flag,argv[cont]))) cont+=2;
    if (cont >= argc) return NULL;
    char *param = (char *) malloc(sizeof(char)*strlen(argv[cont+1]));
    if (param == NULL){
    	perror("Error, solicitud de memoria denegada.\n");
    	exit(1);
    }
    strcpy(param,argv[cont+1]);
    return param;
}


/*
	Nombre: escribirSocket
	Descripción: Se encarga de escribir el tamaño del mensaje y el mensaje en
					el socket del Cliente.
					Si el tamaño de mensaje es 0, se toma como finzalización
					forzada debido al comando ctrl + c.
	Argumentos:	- s: File descriptor del socket del Cliente.
				- msj: String que representa el mensaje que se desea enviar.
*/
void escribirSocket(int s,char* msj){
 	char *tamMsj = (char *) malloc(sizeof(char)*5);
	if (tamMsj == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(2);
	}
	int tam = strlen(msj);
	sprintf(tamMsj,"%d",tam);
	int ok;
	ok = write(s,tamMsj,4);
	if (ok < 0){
		perror("Error, no se pudo escribir en el socket.\n");
		exit(3);
	}
	ok = write(s,msj,tam); 
	if(ok < 0){
		perror("Error, no se pudo escribir en el socket.\n");
		exit(4);
	}
	free(tamMsj);
}


/*	Nombre: leerSocket
	Descripción: Se encarga de leer un mensaje del socket del Cliente.
					Lee primero el tamaño del mensaje y despues el mensaje.
	Argumentos:	- s: File descriptor del socket del Cliente.
				- msj: Arreglo de Strings que representa el mensaje leido.
*/
void leerSocket(int s,char** msj){
	*msj = NULL;
	int tam, ok;
	char *tamMsj = (char *) malloc(sizeof(char)*5);
	if (tamMsj == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(5);
	}
	ok = read(s, tamMsj, 4);
	if (ok < 0){
		perror("Error, no se pudo leer del socket.\n");
		exit(6);
	}
	sscanf(tamMsj,"%d",&tam);
	free(tamMsj);
	if (tam == 0) return;

	*msj = (char *) malloc(sizeof(char)* tam);		
	if (*msj == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(7);
	}
	ok = read(s, *msj, tam);
	if (ok < 0){
		perror("Error, no se pudo leer del socket.\n");
		exit(8);
	}
}


/*	Nombre: verificar
	Descripción: Se encarga de tomar el mensaje leido del socket del Cliente y
					verificar si es el mensaje de culminación del programa o no.
	Parámetros:	- mjs: Arreglo de Strings que representa el mensaje a ser
						enviado.
	Retorno: Entero que representa si se termina el programa o no.

*/
int verificar(char** msjLeido){
	int salidafinal = 42;
	char *i = *msjLeido;
	char*  msj = (char *) malloc(sizeof(char)* strlen(*msjLeido));
	if (msj == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(9);
	}
	sscanf(*msjLeido, " %d %s ", &salidafinal, msj);
	strcpy(*msjLeido, *msjLeido+3);
	if(!strlen(*msjLeido)) {
		free(i);
		*msjLeido = NULL;
	}
	free(msj);
	return salidafinal;
}


/*
	Nombre: separar
	Descripción: Se encarga de separar el comando introducido en el nombre
					del comando y su argumento.
	Parámetros:	- comando : String que representa el comando introducido.
	Retorno: Arreglo de Strings en donde el primer elemento es el comando y el
				segundo es el argumento.
*/
char **separar (char *comando){
	char **comandoSep;
	comandoSep = (char **) malloc(sizeof(char *)*2);
	if (comandoSep == NULL){
		perror("Error, solicitud de memoria denegada.\n");
		exit(10);
	}

	int cont = 0;
	char *elem;
	elem = strtok (comando," ");
	comandoSep[0] = (char *) malloc(sizeof(char)*strlen(elem));
	if (comandoSep[0] == NULL) {
		perror("Error, solicitud de memoria denegada.\n");
		exit(11); 
	}
	strcpy(comandoSep[0],elem);
	comandoSep[1] = NULL;
	elem = strtok (comando+4,"\n");
	if (elem == NULL) return comandoSep;
	while (elem[cont]==' ') cont++;
	char *elem2 = &elem[cont];
	while (elem[cont] != 0) cont++;
	cont--;
	while (elem[cont] == ' ') {
		elem[cont]= 0;
		cont--;
	}
	comandoSep[1] = (char *) malloc(sizeof(char)*strlen(elem2));
	if (comandoSep[1] == NULL) {
		perror("Error, solicitud de memoria denegada.\n");
		exit(12);
	}
	strcpy(comandoSep[1], elem2);
	return comandoSep;
}

/*
	Nombre: error_handler
	Descripción: Se encarga de decirnos los posibes errores que se producen
					durante la ejecución del programa.
	Parámetros: - error: Entero que representa el código del error.
	Retorno: String que contiene el mensaje con el error ocurrido.
*/
char *error_handler(int error){
	char *msj = (char *) calloc(sizeof(char), 100);
	if(msj == NULL){ 
		perror("Error, solicitud de memoria denegada.\n");
		exit(1);
	}

	switch (error) {
		case 0:
		    sprintf(msj,"servidor@servidor: Comando inexistente.\n");
		    break;
		case 1:
		    sprintf(msj,"servidor@servidor: La sala ya existe.\n");
		    break;
		case 2:
		   sprintf(msj,"servidor@servidor: La sala no existe.\n");
		   break;
		case 3:
		   sprintf(msj,"servidor@servidor: Ya esta suscrito a esta sala.\n");
		   break;
		case 4:
		   sprintf(msj,"servidor@servidor: No puede eliminar la sala por defecto.\n");
		   break;
		case 5:
		   sprintf(msj,"servidor@servidor: Parámetros incompletos del comando.\n");
		   break;
		case 6:
		   sprintf(msj,"servidor@servidor: Nombre de usuario ya existente.\n");
		   break;
	}
	return msj;
}


/*
	Nombre: incluir_verificacion
	Descripción: Se encarga de escribir un mensaje en el socket del Cliente para
					indicarle que si es el último mensaje a transmitir (0) o
					aún falta (1).
	Parámetros:	- continuar: Entero que representa si aún falta por enviar
							mensajes o es el útlimo.
				-msj: Arreglo de String que representa el mensaje a ser enviado.
*/
void incluir_verificacion(int continuar, char** msj){
    char* msjNuevo = (char *) calloc(sizeof(char), strlen(*msj));
    if (msjNuevo == NULL){
    	perror("Error, solicitud de memoria denegada.\n");
    	error(1);
    }
    sprintf(msjNuevo," %d %s ", continuar, *msj);
    free(*msj);
    *msj = msjNuevo;
}