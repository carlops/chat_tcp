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

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "extras.h"
#include <time.h>

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

void *manejadorHilo(void *arg); 

struct infoUsr {
    pthread_t id;
    int fd;
    void *salas;//falta el tipo

};

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

    char *puerto = obtener_parametros("-l",argv,argc);
	char *bitacora = obtener_parametros("-b",argv,argc);
    if ((argc!=5)||!bitacora||!puerto)
        {perror("Parametros invalidos"); exit(-1);}
   
    fd = fopen(bitacora,"a");
    if (fd==NULL) perror("abriendo bitacora");

    // Inicializacion Socket
    listenfd = socket(AF_INET,SOCK_STREAM,0);	
    if (listenfd== -1) perror("socket(init)");
    
    bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    my_addr.sin_port = htons(atoi(puerto));
    e = bind(listenfd,PSOCK_ADDR &my_addr, sizeof(my_addr));
    if (e==-1) perror("bind");

    e = listen(listenfd,1024);
    if (e==-1) perror("listen");

    tiempo = get_time();
    fprintf(fd,"%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);
    printf("%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);

    //Aqui se inicializan las variables globales como cantidad de salas de chats,de usuarios activos, etc
    


    //Espera por clientes
    while(1) {
        client_size= sizeof(client_addr);
        newfd = accept(listenfd,PSOCK_ADDR &client_addr,&client_size);
        tiempo = get_time();
        fprintf(fd,"%sNuevo usuario conectado al servidor\n",tiempo);
        printf("%sNuevo usuario conectado al servidor\n",tiempo);

        if (!(arg=(struct infoUsr*) malloc(sizeof(struct infoUsr))))
            perror("asignacion memoria");
        arg->id=0;
        arg->fd=newfd;
        //conectarse
        //login();
        char username[100];
        n = recv(newfd,username,100,0);
        //buscar en userdb   
        

        if (pthread_create(&hilo,NULL,&manejadorHilo,(void *) arg))
            perror("creacion hilo");
        
        //close(newfd);
    } 

}

void *manejadorHilo(void *arg) {
    int n;
    char msg[1000];
    struct infoUsr *usr= (struct infoUsr *) arg;
    //usr->id=pthreadself();
    send(usr->fd,"user:",sizeof(char)*5,0);
    n = recv(usr->fd,msg,1000,0);
    char *resp=malloc(sizeof(char)*(strlen(msg)+11));
    strcpy(resp,"Recibido: ");
    strcat(resp,msg);
    send(usr->fd,resp,sizeof(char)*(strlen(msg)+11),0);
    while (1) {
        n = recv(usr->fd,msg,1000,0);
        printf("-------------------------------------------------------\n");
        msg[n] = 0;
        printf("Received the following:\n");
        printf("%s",msg);
        printf("-------------------------------------------------------\n");
    }
}

void crearUsuario() {
//ver si ya existe

//agregarlo a userdb

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
