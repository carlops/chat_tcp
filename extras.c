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