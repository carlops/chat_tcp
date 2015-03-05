/* Funciones de Ayuda */
#include "extras.h"

char *obtener_parametros(char *flag,char *argv[], int argc){

    int i = 1;
    while((i<argc)&&(strcmp(flag,argv[i]))) i+=2;
    if (i>=argc) 
		return NULL;
    char *res = (char *) malloc(sizeof(char)*strlen(argv[i+1]));
    if(res==NULL)
        {perror("Solicitud de memoria denegada");exit(1);}
    strcpy(res,argv[i+1]);
    return res;

}

