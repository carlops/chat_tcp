/*
 * Servidor
 *
 */

#define PSOCK_ADDR (struct sockaddr *)

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
//#include "extras.h"

void *manejadorHilo(void *arg); 

struct infoUsr {
    pthread_t id;
    int fd;
    
};

int main(int argc, char *argv[]) {

    struct sockaddr_in my_addr, client_addr; 
	int listenfd, e, client_size, newfd, n;
    pthread_t hilo;
    struct infoUsr *arg;

    // Inicializacion Socket
    listenfd = socket(AF_INET,SOCK_STREAM,0);
	
    if (listenfd== -1) perror("socket(init)");
    
    bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    my_addr.sin_port = htons(1672);
    e = bind(listenfd,PSOCK_ADDR &my_addr, sizeof(my_addr));
    if (e==-1) perror("bind");

    e = listen(listenfd,1024);
    if (e==-1) perror("listen");

    //Aqui se inicializan las variables globales como cantidad de salas de chats,de usuarios activos, etc



    //Espera por clientes
    while(1) {
        client_size= sizeof(client_addr);
        newfd = accept(listenfd,PSOCK_ADDR &client_addr,&client_size);

        if (!(arg=(struct infoUsr*) malloc(sizeof(struct infoUsr))))
            perror("asignacion memoria");
        arg->id=NULL;
        arg->fd=newfd;

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
    while (1) {
        send(usr->fd,"username: ",sizeof("username: "),0);
        //send(usr->fd,"username: ",1000,0);
        n = recv(usr->fd,msg,100,0);
        printf("-------------------------------------------------------\n");
        msg[n] = 0;
        printf("Received the following:\n");
        printf("%s",msg);
        printf("-------------------------------------------------------\n");
    }
}


