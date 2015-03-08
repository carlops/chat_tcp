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
    Descripcion: Se encarga de obtener el argumento que corresponde a un cierto 
                    flag de la entrada.
    Argumentos: -flag: String que puede ser -d, -p, -l.
                -argv: Arreglo que representa los argumentos introducidos.
                -argc: Entero que representa el numero total de argumentos.
    Return: String con el argumento correspondiente al flag indicado.
*/
char *obtenerParametro(char *flag,char *argv[], int argc){
    int cont = 1;
    while((cont < argc) && (strcmp(flag,argv[cont]))) cont+=2;
    if (cont >= argc) return NULL;
    char *param = (char *) malloc(sizeof(char)*strlen(argv[cont+1]));
    if(param == NULL){
    	perror("Error, solicitud de memoria denegada.");
    	exit(1);
    }
    strcpy(param,argv[cont+1]);
    return param;
}

/* Escribe el tamanio del mensaje y el mensaje.
 * Si el tamanio del mensaje es 0, se toma como finalizacion
 * forzada debido al comando cntrl + c 
 */
void escribir_socket(int fd,char* buffer){
	 	char *mensaje = (char *) malloc(sizeof(char)*5);
		if(mensaje==NULL){
				perror("Solicitud de memoria denegada");
				exit(2);
		}
		int tam = strlen(buffer);
		int ok;
		sprintf(mensaje,"%d",tam);
		ok = write(fd,mensaje, 4);
		if(ok<0){
				perror("No se pudo escribir en el socket");
				exit(2);
		}
		ok = write(fd,buffer, tam); 
		if(ok<0){
				perror("No se pudo escribir en el socket");
				exit(3);
		}
		free(mensaje);
}

/* Leer socket.
 * Lee primero el tamanio del mensaje y despues el mensaje
 */
void leer_socket(int fd,char** buffer){
		int tam, ok;
		*buffer = NULL;
		char *mensaje = (char *) malloc(sizeof(char)*5);
		if(mensaje==NULL){
				perror("Solicitud de memoria denegada");
				exit(4);
		}
		ok = read(fd, mensaje, 4);
		if(ok<0){
				perror("No se pudo leer del socket");
				exit(4);
		}
		sscanf(mensaje,"%d",&tam);
		free(mensaje);
		if (tam==0) return;
		*buffer = (char *) malloc(sizeof(char)* tam);		
		if(*buffer==NULL){
				perror("Solicitud de memoria denegada");
				exit(5);
		}
		ok = read(fd, *buffer, tam);
		if(ok<0){
				perror("No se pudo leer del socket");
				exit(5);
		}
}

char **split (char *st){

		char **desc;
		desc = (char **) malloc(sizeof(char *)*2);
		if(desc==NULL){
				perror("Solicitud de memoria denegada");
				exit(6);
		}
		int i=0;
		char *pch;

		pch = strtok (st," ");
		desc[0] = (char *) malloc(sizeof(char)*strlen(pch));
		if(desc[0]==NULL) {
				perror("Solicitud de memoria denegada");
				exit(7); 
		}
		strcpy(desc[0],pch);
		desc[1] = NULL;
		pch = strtok (st+4, "\0");
		if(pch==NULL) return desc;
		while (pch[i]==' ') i++;
		char *first = &pch[i];
		while (pch[i]!=0) i++;
		i--;
		while (pch[i]==' ') {
				pch[i]= 0;
				i--;
		}
		desc[1] = (char *) malloc(sizeof(char)*strlen(first));
		if(desc[1]==NULL) {
				perror("Solicitud de memoria denegada");
				exit(8);
		}
		strcpy(desc[1], first);

		return desc;
}

int leer_verificacion(char** mensaje){

		int salidaf= 2;
		char *aux = *mensaje;
		char*  buffer = (char *) malloc(sizeof(char)* strlen(*mensaje));
		if(buffer==NULL){
				perror("Solicitud de memoria denegada");
				exit(9);
		}
		sscanf(*mensaje, " %d %s ", &salidaf, buffer);
		strcpy(*mensaje, *mensaje+3); //3 es la distancia que ocupa la verificacion 
		if(!strlen(*mensaje)) {
				free(aux);
				*mensaje = NULL;
		}
		free(buffer);

		return salidaf;
}
