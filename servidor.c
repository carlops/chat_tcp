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

// Alias para todas las tablas de hash
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
agregar_socket(Lista_sockets *L, socket_s *sock){
	Lista_sockets *temp = L;
	Lista_sockets *temp_T;
	
	if (temp->socket== NULL){
		temp->socket = sock;
		temp->fin = temp;
		temp->sig = NULL;
	}
	else{
		temp_T = malloc(sizeof(Lista_sockets));
		temp_T->socket=sock;
		temp_T->sig =  NULL;
		temp_T->fin = NULL;
		temp->fin->sig=temp_T;
		temp->fin = temp_T;		
	};
}


// Funcion para inicializar una lista de usuarios
ini_usuario(Lista_usuarios *L){
	L->user = NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para inicializar una lista de salas
ini_sala(Lista_salas *L){
	L->room = NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para inicializar una lista de sockets
ini_socket(Lista_sockets *L){
	L->socket= NULL;
	L->sig= NULL;
	L->fin = NULL;
}

// Funcion para eliminar un usuario de una lista de usuarios

elim_usuario(Lista_usuarios *L, char *string){
	Lista_usuarios *temp = L;
	Lista_usuarios *temp_T = temp;
	
	if (temp==NULL){
		return;
	}
	else{
			if (strcmp(temp->user->nombre,string)==0){
				if (temp->sig != NULL){
						temp->sig->fin = temp->fin;
						*L = *temp->sig;
						free(temp->user);
						free(temp);
				}
				else{
					L->user =NULL;
					L->sig = NULL;
					L->fin = NULL;
						free(temp->user);
						free(temp);
				}
			}
			
			while (temp->sig != NULL){

				temp_T = temp;
				temp = temp->sig;

				if (strcmp(temp->user->nombre,string)==0){
				
					if (temp->sig != NULL){
						temp_T->sig = temp->sig;
					}
					free(temp->user);
					free(temp);
				}
		}
	}
	return;
}
// Funcion para eliminar un sala de una lista de salas
// FALTA LIBERAR LOS SOCKETS
elim_sala(Lista_salas *L, char *string){
	Lista_salas *temp = L;
	Lista_salas *temp_T;
	
	if (temp==NULL){
		return;
	}
	else{
			if (strcmp(temp->room->nombre,string)==0){
				if (temp->sig != NULL){
						temp->sig->fin = temp->fin;
						*L = *temp->sig;
						free(temp->room);
						free(temp);
				}
				else{
					L->room = NULL;
					L->sig = NULL;
					L->fin = NULL;
					
						free(temp->room);
						free(temp);
				}
			}
			
			while (temp->sig != NULL){

				temp_T = temp;
				temp = temp->sig;

				if (strcmp(temp->room->nombre,string)==0){
				
					if (temp->sig != NULL){
						temp_T->sig = temp->sig;
					}
					free(temp->room);
					free(temp);
				}
		}
	}
	return;
}

// Funcion para eliminar un socket de una lista de sockets
elim_socket(Lista_sockets*L, int *n){
	Lista_sockets *temp = L;
	Lista_sockets *temp_T;
	
	if (temp==NULL){
		return;
	}
	else{
			if (temp->socket->socket == *n){
				if (temp->sig != NULL){
						temp->sig->fin = temp->fin;
						*L = *temp->sig;
						free(temp->socket);
						free(temp);
				}
				else{
					L->socket = NULL;
					L->sig = NULL;
					L->fin = NULL;
						free(temp->socket);
						free(temp);
				}
			}
			
			while (temp->sig != NULL){

				temp_T = temp;
				temp = temp->sig;

				if (temp->socket->socket == *n){
				
					if (temp->sig != NULL){
						temp_T->sig = temp->sig;
					}
					free(temp->socket);
					free(temp);
				}
		}
	}
	return;
}

// Funcion para agregar un usuario a una tabla de hash
agregarUsuario(Tabla_Hash_Us t, usuario *user){
	int k;
	k = hash(user->nombre,TAM);
	return  agregar_usuario(&t.T[k],user);
}

// Funcion para agregar una sala a una tabla de hash
agregarSala(Tabla_Hash_Sa t, sala *room){
	int k;
	k = hash(room->nombre,TAM);
	return  agregar_sala(&t.T[k],room);
}

// Funcion para agregar un socket a una tabla de hash
agregarSocket(Tabla_Hash_So t, socket_s *sock){
	int k;
	char string[50];
	sprintf(string,"%d",&sock->socket);
	k = hash(string,TAM);
	return  agregar_socket(&t.T[k],sock);
}

// Funcion para eliminar un usuario de una tabla de hash
eliminarUsuario(Tabla_Hash_Us t, char *string){
	int k;
	k = hash(string,TAM);
	elim_usuario(&t.T[k],string);
}

// Funcion para eliminar una sala de una tabla de hash
eliminarSala(Tabla_Hash_Sa t, char *string){
	int k;
	k = hash(string,TAM);
	elim_sala(&t.T[k],string);
}

// Funcion para eliminar un socket de una tabla de hash
eliminarSocket(Tabla_Hash_So t, int * n){
	int k;
	char string[50];
	sprintf(string,"%d",n);
	k = hash(string,TAM);
	elim_socket(&t.T[k],n);
}

// Funcion para inicializar tabla de hash
inicializar_tablas(void *t, int c){
	// cambiar a switch
	int i=0;
	if (c == '0'){
		Tabla_Hash_Us *T;
		T = (Tabla_Hash_Us *) t;
		for (i;i<TAM;i++){
			ini_usuario(&T->T[i]);
		}
	}
	else if (c == '1'){
		Tabla_Hash_Sa *T;
		T = (Tabla_Hash_Sa *) t;
		for (i;i<TAM;i++){
			ini_sala(&T->T[i]);
		}
	}
	else {
		Tabla_Hash_So *T;
		T = (Tabla_Hash_So *) t;
		for (i;i<TAM;i++){
			ini_socket(&T->T[i]);
		}
	}
}


	
// Mutex
pthread_mutex_t mutex_sockets = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_usuarios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_salas = PTHREAD_MUTEX_INITIALIZER;

	
	Tabla_Hash_Sa tabla_de_salas;
	Tabla_Hash_So tabla_de_sockets;
	Tabla_Hash_Us tabla_de_usuarios;



void *Hilo_cliente(void *arg) {
   int *sock = (int *) arg;
   int res;
   msj_cli *recibido;
   
   socket_s *socket;
   socket = malloc(sizeof(socket_s));
   socket->socket = *sock;
   socket->nombre = NULL;
   
   pthread_mutex_lock(&mutex_sockets);
   agregarSocket(tabla_de_sockets,socket);
   pthread_mutex_unlock(&mutex_sockets);
   
   recibido=malloc(sizeof(msj_cli)) ;
		res = read(*sock,recibido,sizeof(*recibido));
		if (res){
			printf("Hemos recibido algo de un cliente:\n");
			printf("cmd : %c, tiempo: %s, usuario: %s\n",recibido->cmd,recibido->tiempo,recibido->usuario);
			printf("mensaje: %s\n",recibido->mensaje);
			
		}
   close(*sock);
   pthread_exit(NULL);
   
}






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


int main(int argc, char *argv[]) {
    
    struct sockaddr_in my_addr, client_addr; 
	int sock_maestro, resultado, client_size, n;
	int *sock_cliente;
    pthread_t hilo;
	FILE *fd;
    char *tiempo;

	
	inicializar_tablas(&tabla_de_usuarios,'0');
	inicializar_tablas(&tabla_de_salas,'1');
	inicializar_tablas(&tabla_de_sockets,'2');
	
	int c;
	char *puerto;
	char *bitacora;
	msj_cli *recibido;

	
	puerto = NULL;
	bitacora = NULL;

	while ((c = getopt(argc,argv,"l:b:")) != -1 ){
		switch(c){
			case 'l':
				puerto = optarg;
				break;
			case 'b' :
				bitacora = optarg;
				break;
			default:
				printf("parametro desconocido\n");
				exit(-1);
		}
	}	
   
	if (bitacora == NULL){
		printf("Se necesita una bitacora para el funcionamiento del servidor\n");
		printf("Ejecute nuevamente el comando indicandole el archivo de la");
		printf(" bitacora\n");
		exit(-1);
	}
	if (puerto == NULL){
		printf("Se necesita un puerto para el funcionamiento del servidor\n");
		printf("Ejecute nuevamente el comando indicandole el puerto\n");
		exit(-1);
	}
    fd = fopen(bitacora,"a");
    if (fd==NULL) perror("abriendo bitacora");

    // Inicializacion Socket
    sock_maestro = socket(AF_INET,SOCK_STREAM,0);	
    if (sock_maestro== -1) perror("socket(init)");
    
    bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    my_addr.sin_port = htons(atoi(puerto));
	
    resultado = bind(sock_maestro,PSOCK_ADDR &my_addr, sizeof(my_addr));
    if (resultado == -1) perror("bind");

    resultado = listen(sock_maestro,1024);
    if (resultado == -1) perror("listen");

    tiempo = get_time();
    fprintf(fd,"%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);
    printf("%sSocket abierto en el puerto %s y esperando conexion..\n",tiempo,puerto);

    //Aqui se inicializan las variables globales como cantidad de salas de chats,de usuarios activos, etc
    
	

    //Espera por clientes
    while(1) {
		sock_cliente  = malloc(sizeof(int));
        client_size= sizeof(client_addr);
        *sock_cliente = accept(sock_maestro,PSOCK_ADDR &client_addr,&client_size);
        tiempo = get_time();
        fprintf(fd,"%sNuevo usuario conectado al servidor\n",tiempo);
        printf("%sNuevo usuario conectado al servidor\n",tiempo);
		fflush(fd);
		resultado = pthread_create(&hilo,NULL, &Hilo_cliente,(void *) sock_cliente);
		if (resultado == 0){
			printf("Creacion correcta de un hilo\n");}
		sock_cliente = NULL;
    } 
		
		
		close(sock_maestro);
		fclose(fd);
		exit(0);
}

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

