/* 
 * CLIENTE
 * por ahora el 1er arg es host
 * 	el 2do es el servidor
 *
 */
//#include <string.h>
//#include <unistd.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <sys/types.h>

#define PSOCK_ADDR (struct sockaddr *)

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
//#include "extras.h"

int main(int argc, char *argv[]) {

	struct sockaddr_in my_addr, server_addr; 
	int sockfd,s;
    char sendline[1000],recvline[1000];
	
    if (argc !=2){
        printf("usage: cliente <SERVER IP ADDRESS>\n");
        exit(-1);
    }

    sockfd=socket(AF_INET,SOCK_STREAM,0);

    if (sockfd== -1) perror("socket(init)");
    
    bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); 
    server_addr.sin_port = htons(1672);
	
    s = connect(sockfd, PSOCK_ADDR &server_addr, sizeof(server_addr));
    if (s != 0)  perror("conect");

    while (1) // (fgets(sendline, 10000,stdin) != NULL)
    {
        s = recv(sockfd, recvline, strlen(recvline), 0);
        if (s==-1) perror("receiving");
        
        //n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
        recvline[s]='\0';
        fputs(recvline,stdout);
        
        
        //sendto(sockfd, sendline, strlen(sendline), 0,
        //    PSOCK_ADDR &servaddr, sizeof(servaddr));
        fgets(sendline, 100,stdin);
        s = send(sockfd, sendline, strlen(sendline), 0);
        if (s==-1)  perror("sending");
    }
    //if (bind(sockfd,(struct sockaddr *) &my_addr,
	//		sizeof(struct sockaddr_in))==-1)
	//	perror("bind");
	
	//connect
	//read
	//write
	//close
	
    return 0;
}

/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500

int
main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Obtain address(es) matching host/port 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // Datagram socket 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          // Any protocol

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully connect(2).
    // If socket(2) (or connect(2)) fails, we (close the socket
    // and) try the next address.  
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  // Success
       close(sfd);
    }

    if (rp == NULL) {               // No address succeeded
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           // No longer needed

    // Send remaining command-line arguments as separate
    // datagrams, and read responses from server

    for (j = 3; j < argc; j++) {
        len = strlen(argv[j]) + 1;
               // +1 for terminating null byte

        if (len + 1 > BUF_SIZE) {
            fprintf(stderr,
                    "Ignoring long message in argument %d\n", j);
            continue;
        }

        if (write(sfd, argv[j], len) != len) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }

        nread = read(sfd, buf, BUF_SIZE);
        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("Received %ld bytes: %s\n", (long) nread, buf);
    }

    exit(EXIT_SUCCESS);
}
*/

