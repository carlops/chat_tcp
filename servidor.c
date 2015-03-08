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

// Funciona para calcular los hashes de los string
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

// Struct para los mensajes del cliente al servidor
struct mensaje_cliente {
	char cmd;
	char *tiempo;
	char *usuario;
	char mensaje[500];
};

// Struct para los mensajes del servidor al cliente
struct mensaje_servidor {
	char cmd;
	char *tiempo;
	char *usuario;
	char ack;
	char mensaje[500];
};

// Alias para los mensajes
typedef struct mensaje_cliente msj_cli;
typedef struct mensaje_servidor msj_srv;

// Struct para los usuarios
struct user {
	char *nombre;
	int socket;
	char *sala;
};

// Struct para los sockets
struct sockets_ser {
	int socket;
	char *nombre;
};


// Alias para los sockets
typedef struct sockets_ser socket_s;

// Alias para los usuarios
typedef struct user usuario;

// Lista enlazada de usuarios
struct lista_enlazada_de_usuarios {
	usuario *user;
	struct lista_enlazada_de_usuarios *sig;
	struct lista_enlazada_de_usuarios *fin;
};

// Alias para la lista enlazada de usuarios
typedef struct lista_enlazada_de_usuarios Lista_usuarios;

// Lista enlazada de sockets
struct lista_enlazada_de_sockets {
	socket_s *socket;
	struct lista_enlazada_de_sockets *sig;
	struct lista_enlazada_de_sockets *fin;
};

// Alias para la lista enlazada de sockets
typedef struct lista_enlazada_de_sockets Lista_sockets;

// Struct para las salas
struct room {
	Lista_sockets sockets;
	char *nombre;
	char status;
};

// Alias para las salas
typedef struct room sala;

// Lista enlzada para las salas
struct lista_enlazada_de_salas {
	sala *room;
	struct lista_enlazada_de_salas *sig;
	struct lista_enlazada_de_salas *fin;
};

// Alias para las listas enlazadas de salas
typedef struct lista_enlazada_de_salas Lista_salas;

// Tabla de hash para usuarios
struct tabla_usuarios {
	Lista_usuarios T[TAM];	
};

// Tabla de hash para sockets
struct tabla_sockets {
	Lista_sockets T[TAM];	
};

// Tabla de hash para salas
struct tabla_salas {
	Lista_salas T[TAM];	
};

// Alias para todas las talbas de hash
typedef struct tabla_usuarios Tabla_Hash_Us;
typedef struct tabla_sockets Tabla_Hash_So;
typedef struct tabla_salas Tabla_Hash_Sa;

// Funcion para buscar un usuario en una lista
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

// Funcion para buscar un usuario en una tabla de hash
usuario * buscarUsuario(Tabla_Hash_Us t, char *string){
	int k;
	usuario *valor;
	k = hash(string,TAM);
	return  buscar_usuario_lista(t.T[k],string);
}

// Funcion para buscar una sala en una lista
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

// Funcion para buscar una sala en una tabla de hash
sala * buscarSala(Tabla_Hash_Sa t, char *string){
	int k;
	k = hash(string,TAM);
	return  buscar_sala_lista(t.T[k],string);
}

// Funcion para buscar un socket en una lista
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

// Funcion para buscar un socket en una tabla de hash
socket_s * buscarSocket(Tabla_Hash_So t, int *n){
	int k;
	char string[50];
	sprintf(string,"%d",n);
	k = hash(string,TAM);
	return  buscar_socket_lista(t.T[k],n);
}

// Funcion para agregar un usuario a una lista
agregar_usuario(Lista_usuarios *L, usuario *user){
	Lista_usuarios *temp = L;
	Lista_usuarios *temp_T;
	
	if (temp->user == NULL){
		temp->user = user;
		temp->fin = temp;
		temp->sig = NULL;
	}
	else{
		temp_T = malloc(sizeof(Lista_usuarios));
		temp_T->user=user;
		temp_T->sig =  NULL;
		temp_T->fin = NULL;
		
		temp->fin->sig=temp_T;
		temp->fin = temp_T;
	}
}

// Funcion para agregar un sala a una lista
agregar_sala(Lista_salas *L, sala *hab){
	Lista_salas *temp = L;
	Lista_salas *temp_T;
	
	if (temp->room == NULL){
		temp->room = hab;
		temp->fin = temp;
		temp->sig = NULL;
	}
	else{
		temp_T = malloc(sizeof(Lista_salas));
		temp_T->room=hab;
		temp_T->sig =  NULL;
		temp_T->fin = NULL;		
		temp->fin->sig=temp_T;
		temp->fin = temp_T;		
	}
}

// Funcion para agregar un socket a una lista
agregar_socket(Lista_sockets *L, socket_s *socket){
	Lista_sockets *temp = L;
	Lista_sockets *temp_T;
	
	if (temp->socket== NULL){
		temp->socket = user;
		temp->fin = temp;
		temp->sig = NULL;
	}
	else{
		temp_T = malloc(sizeof(Lista_sockets);
		temp_T->socket=socket;
		temp_T->sig =  NULL;
		temp_T->fin = NULL;
		temp->fin->sig=temp_T;
		temp->fin = temp_T;		
	}
}


// Funcion para inicializar una lista de usuarios
ini_usuario(Lista_usuarios *L){
	L->user = NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para inicializar una lista de salas
ini_sala(Lista_salas *L, sala *hab){
	L->room = NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para inicializar una lista de sockets
ini_socket(Lista_sockets *L, socket_s *socket){
	L->socket= NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para eliminar un usuario de una lista de usuarios

elim_usuario(Lista_usuarios *L, char *string){
	Lista_usuarios *temp = L;
	Lista_usuarios *temp_T;
	
	if (temp==NULL){
		return;
	}
	else{
			while (temp != NULL){
			if (strcmp(temp->user->nombre,string)==0){
				temp_T = temp;
				if (temp_T->sig != NULL){
					temp_T->sig->fin = temp_T->fin;
				}
				*L = temp_T->sig;
				free(temp->sala);
				free(temp->user);
				free(temp);
			}
			temp = temp->sig;
		}
	}
	return
}
// Funcion para eliminar un sala de una lista de salas
elim_sala(Lista_salas *L, char *string){
	Lista_salas *temp = L;
	Lista_salas *temp_T;
	
	if (temp==NULL){
		return;
	}
	else{
			while (temp != NULL){
			if (strcmp(temp->room->nombre,string)==0){
				temp_T = temp;
				if (temp_T->sig != NULL){
					temp_T->sig->fin = temp_T->fin;
				}
				*L = temp_T->sig;
				free(temp->room);
				//Faltaria liberar la lista de sockets 
				free(temp);
			}
			temp = temp->sig;
		}
	}
	return
}

// Funcion para eliminar un socket de una lista de sockets
elim_socket(Lista_sockets*L, int *n){
	Lista_sockets *temp = L;
	Lista_sockets *temp_T;
	
	if (temp==NULL){
		return;
	}
	else{
			while (temp != NULL){
			if (temp->socket->socket == *n){
				temp_T = temp;
				if (temp_T->sig != NULL){
					temp_T->sig->fin = temp_T->fin;
				}
				*L = temp_T->sig;
				free(temp->socket);
				free(temp->nombre);
				free(temp);
			}
			temp = temp->sig;
		}
	}
	return
}

// Funcion para agregar un usuario a una tabla de hash
agregarUsuario(Tabla_Hash_Us t, usuario *user){
	int k;
	k = hash(user->nombre,TAM);
	return  agregar_usuario(t.T[k],user);
}

// Funcion para agregar una sala a una tabla de hash
agregarSala(Tabla_Hash_Sa t, sala *room){
	int k;
	k = hash(room->nombre,TAM);
	return  agregar_sala(t.T[k],room);
}

// Funcion para agregar un socket a una tabla de hash
agregarSocket(Tabla_Hash_So t, socket *sock){
	int k;
	char string[50];
	sprintf(string,"%d",&(sock->socket));
	k = hash(string,TAM);
	return  agregar_socket(t.T[k],sock);
}

// Funcion para eliminar un usuario de una tabla de hash
eliminarUsuario(Tabla_Hash_Us t, char *string){
	int k;
	k = hash(string,TAM);
	elim_usuario(t.T[k],string);
}

// Funcion para eliminar una sala de una tabla de hash
eliminarSala(Tabla_Hash_Sa t, char *string){
	int k;
	k = hash(string,TAM);
	elim_sala(t.T[k],string);
}

// Funcion para eliminar un socket de una tabla de hash
eliminarSocket(Tabla_Hash_So t, int * n){
	int k;
	char string[50];
	sprintf(string,"%d",n));
	k = hash(string,TAM);
	elim_socket(t.T[k],n);
}

// Funcion para inicializar tabla de hash
inicializar_tablas(void *t, int c){
	// cambiar a switch
	int i=0;
	if (c == '0'){
		tabla_usuarios *T;
		T = (tabla_usuarios *) t;
		for (i;i<TAM;i++){
			ini_usuario(T.T[i]);
		}
	}
	else if (c == '1'){
		tabla_salas *T;
		T = (tabla_salas *) t;
		for (i;i<TAM;i++){
			ini_sala(T.T[i]);
		}
	}
	else {
		tabla_sockets *T;
		T = (tabla_sockets *) t;
		for (i;i<TAM;i++){
			ini_socket(T.T[i]);
		}
	}
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
