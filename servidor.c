/*
 * Servidor
 *
 */

#define PSOCK_ADDR (struct sockaddr *)

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "extras.h"

int main(int argc, char *argv[]) {

    struct sockaddr_in my_addr, client_addr; 
	int listenfd, e, client_size, newfd, n;
    char msg[1000];
	pid_t childpid;

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

    while(1) {
        client_size= sizeof(client_addr);
        newfd = accept(listenfd,PSOCK_ADDR &client_addr,&client_size);

        // crea un hilo o proceso para el nuevo fd y ahi empieza la comunicacion
        if ((childpid = fork()) == 0)
        {
            close (listenfd);

            for(;;)
            {
                n = recvfrom(newfd,msg,1000,0,PSOCK_ADDR &client_addr,&client_size);
                sendto(newfd,msg,n,0,PSOCK_ADDR &client_addr,sizeof(client_addr));
                printf("-------------------------------------------------------\n");
                msg[n] = 0;
                printf("Received the following:\n");
                printf("%s",msg);
                printf("-------------------------------------------------------\n");
            }

        }
        close(newfd);
    }

}
