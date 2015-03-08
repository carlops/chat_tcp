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
#define TAM 1024

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "extras.h"
#include <time.h>
#include <getopt.h>
#include <string.h>

int hash(char *string, int tam) {
	int l;
	int i=0;
	int c=0;
	l = strlen(string);
	for (i; i<l; i++){
		c = (char) string[i] + c;
	};
	return (c % tam);	
}

struct user {
	char *nombre;
	int socket;
	char *sala;
};

struct sockets_ser {
	int socket;
	char *nombre;
};

typedef struct sockets_ser socket_s;

typedef struct user usuario;

struct lista_enlazada_de_usuarios {
	usuario *user;
	struct lista_enlazada_de_usuarios *sig;
};

typedef struct lista_enlazada_de_usuarios Lista_usuarios;

struct lista_enlazada_de_sockets {
	socket_s *socket;
	struct lista_enlazada_de_sockets *sig;
};

typedef struct lista_enlazada_de_sockets Lista_sockets;

struct room {
	Lista_sockets sockets;
	char *nombre;
	char status;
};

typedef struct room sala;

struct lista_enlazada_de_salas {
	sala *room;
	struct lista_enlazada_de_salas *sig;
};

typedef struct lista_enlazada_de_salas Lista_salas;
struct tabla_usuarios {
	Lista_usuarios T[TAM];	
};

struct tabla_sockets {
	Lista_sockets T[TAM];	
};

struct tabla_salas {
	Lista_salas T[TAM];	
};

typedef struct tabla_usuarios Tabla_Hash_Us;
typedef struct tabla_sockets Tabla_Hash_So;
typedef struct tabla_salas Tabla_Hash_Sa;


usuario * buscar_usuario_lista(Lista_usuarios L, char * string){
	
	Lista_usuarios *temp = &L;
	usuario *tempu ;	
	
	while (temp != NULL) {
		tempu = temp->user;
		if (strcmp(tempu->nombre,string)==0)
			return tempu;
		else {
			temp = temp->sig;
		}
	}
	return NULL;
	
}

usuario * buscarUsuario(Tabla_Hash_Us t, char *string){
	int k;
	usuario *valor;
	k = hash(string,TAM);
	return  buscar_usuario_lista(t.T[k],string);
}

sala * buscar_sala_lista(Lista_salas L, char * string){
	
	Lista_salas *temp = &L;
	sala *tempu ;	
	
	while (temp != NULL) {
		tempu = temp->room;
		if (strcmp(tempu->nombre,string)==0)
			return tempu;
		else {
			temp = temp->sig;
		}
	}
	return NULL;
	
}

sala * buscarSala(Tabla_Hash_Sa t, char *string){
	int k;
	k = hash(string,TAM);
	return  buscar_sala_lista(t.T[k],string);
}

socket_s * buscar_socket_lista(Lista_sockets L, int * n){
	
	Lista_sockets *temp = &L;
	socket_s *tempu ;	
	
	while (temp != NULL) {
		tempu = temp->socket;
		if (tempu->socket == *n )
			return tempu;
		else {
			temp = temp->sig;
		}
	}
	return NULL;
	
}

socket_s * buscarSocket(Tabla_Hash_So t, int *n){
	int k;
	char string[50];
	sprintf(string,"%d",n);
	k = hash(string,TAM);
	return  buscar_socket_lista(t.T[k],n);
}

// char *get_time(){
//     time_t t;
//     char *tiempo;
//     struct tm * timeinfo;
// 
//     tiempo=(char *) malloc(sizeof(char)*25);
//     time(&t);
//     timeinfo = localtime(&t);
//     strftime(tiempo,25,TIMEFORMAT,timeinfo);
//     return tiempo;
// }
// 
// void *manejadorHilo(void *arg); 
// 
// struct infoUsr {
//     pthread_t id;
//     int fd;
//     void *salas;//falta el tipo
// 
// };
// 
// struct lista{
//     char name[50];
//     struct userdb *sig;
// };

int main(int argc, char *argv[]) {
    
    struct sockaddr_in my_addr, client_addr; 
	int listenfd, e, client_size, newfd, n;
//     pthread_t hilo;
//     struct infoUsr *arg;
// 	FILE *fd;
//     char *tiempo;
//     struct lista userdb;  
	int c;
	char *puerto;
	char *bitacora;
	
	usuario *us;
	usuario *ad;
	Lista_usuarios lista;
	

	while ((c = getopt(argc,argv,"l:b:")) != -1 ){
		switch(c){
			case 'l':
				puerto = optarg;
				printf("hola1\n");
				break;
			case 'b' :
				bitacora = optarg;
				printf("hola2 abriendo bitacora\n");
				break;
			default:
				printf("parametro desconocido\n");
				exit(-1);
		}
	}	
   /*
    fd = fopen(bitacora,"a");
    if (fd==NULL) perror("abriendo bitacora");*/
	
	lista.sig=NULL;
	us = malloc(sizeof(usuario));
	us->nombre="adolfo";
	us->socket=25;
	us->sala="cuarto de adolfo";
	lista.user = us;
	
	ad = buscar_usuario_lista(lista,"adolfo2");
	if (ad)
		printf("se encontro al usuario: %s\n",ad->nombre);
	else
		printf("NO\n");
	
	
    exit(0);
//     // Inicializacion Socket
//     listenfd = socket(AF_INET,SOCK_STREAM,0);	
//     if (listenfd== -1) perror("socket(init)");
//     
//     bzero(&my_addr,sizeof(my_addr));
// 	my_addr.sin_family = AF_INET;
// 	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
//     my_addr.sin_port = htons(atoi(puerto));
//     e = bind(listenfd,PSOCK_ADDR &my_addr, sizeof(my_addr));
//     if (e==-1) perror("bind");
// 
//     e = listen(listenfd,1024);
//     if (e==-1) perror("listen");
// 
//     tiempo = get_time();
//     fprintf(fd,"%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);
//     printf("%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);
// 
//     //Aqui se inicializan las variables globales como cantidad de salas de chats,de usuarios activos, etc
//     
// 
// 
//     //Espera por clientes
//     while(1) {
//         client_size= sizeof(client_addr);
//         newfd = accept(listenfd,PSOCK_ADDR &client_addr,&client_size);
//         tiempo = get_time();
//         fprintf(fd,"%sNuevo usuario conectado al servidor\n",tiempo);
//         printf("%sNuevo usuario conectado al servidor\n",tiempo);
// 
//         if (!(arg=(struct infoUsr*) malloc(sizeof(struct infoUsr))))
//             perror("asignacion memoria");
//         arg->id=0;
//         arg->fd=newfd;
//         //conectarse
//         //login();
//         char username[100];
//         n = recv(newfd,username,100,0);
//         //buscar en userdb   
//         
// 
//         if (pthread_create(&hilo,NULL,&manejadorHilo,(void *) arg))
//             perror("creacion hilo");
//         
//         //close(newfd);
//     } 
// 
// }
// 
// void *manejadorHilo(void *arg) {
//     int n;
//     char msg[1000];
//     struct infoUsr *usr= (struct infoUsr *) arg;
//     //usr->id=pthreadself();
//     send(usr->fd,"user:",sizeof(char)*5,0);
//     n = recv(usr->fd,msg,1000,0);
//     char *resp=malloc(sizeof(char)*(strlen(msg)+11));
//     strcpy(resp,"Recibido: ");
//     strcat(resp,msg);
//     send(usr->fd,resp,sizeof(char)*(strlen(msg)+11),0);
//     while (1) {
//         n = recv(usr->fd,msg,1000,0);
//         printf("-------------------------------------------------------\n");
//         msg[n] = 0;
//         printf("Received the following:\n");
//         printf("%s",msg);
//         printf("-------------------------------------------------------\n");
//     }
// }
// 
// void crearUsuario() {
// //ver si ya existe
// 
// //agregarlo a userdb
// 
// }
// 
// void eliminarUsuario() {
// 
// }
// 
// void crearSala(){
// //ver si ya existe
// 
// //agregar a saladb
// }
// 
// void eliminarSala(){
// //expulsar usuarios activos en la sala
//  
// //eliminar de saladb
// }
// 
// void verSala(){
// 
// }
// 
// void verUsuarios(){
// 
// }
// 
// void habilitarSala(){
// 
// }
// 
// void deshabilitarSala(){
// 
// }
// 
// void verBitacora(){
// 
}
