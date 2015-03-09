/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo del Servidor
 *	servidor.c
 *
 *	Descripción:	Este módulo contiene todos los procedimientos relacionados 
 * 					al Servidor en la conexión a través de sockets.
 *
 *	Compilar con: make servidor
 *
 *	Correr: scs_svr -l <puerto-servidor(local)> -b <archivo_bitácora>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "extras.h"


/* CONSTANTES */
#define TIMEFORMAT "%d/%m/%y-%I:%M:%S : "
#define MAXTAM 10


/* TIPOS */
struct Usuario {
    pthread_t id;
    int fd;
    int totalSalas;
    int tamMaxSala;
    char *nombre;
    int posarray;
    char **salas;//falta el tipo
};
typedef struct Usuario infoUsr;


/* VARIABLES */
/*
   - totalUsr: 		Cantidad de usuarios.
   - totalSalas:	Cantidad de salas.
   - losUsuarios:	Arreglo de usuarios.
   - salas:			Arreglo de salas.
   - tamMaxSala:	Número máximo de salas.
   - tamMaxUsr:		Número máximo de ususarios.
*/
int totalUsr, totalSalas, tamMaxSala, tamMaxUsr;
infoUsr **losUsuarios;
char **salas;

pthread_mutex_t mutexsala  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexusr  = PTHREAD_MUTEX_INITIALIZER;


/* METODOS Y FUNCIONES */

/*
	Nombre: obtenerFechaHora
	Descripción: Se encarga de buscar la fecha y hora actual.
	Retorna: String con la fecha y hora.
*/
char *obtenerFechaHora(){
    char *tiempo = (char *) malloc(sizeof(char)*25);
    if (tiempo == NULL){
        perror("Error, solicitud de memoria denegada.\n");
        exit(1);
    }

    time_t t;
    time(&t);
    struct tm *timeInfo = localtime(&t);
    strftime(tiempo,25,TIMEFORMAT,timeInfo);
    return tiempo;
}

/*
	Nombre: crearUsuario
	Descripción: Se enccarga de incializar todo lo que este relacionado con
					el usuario cuando éste entra en la sala de chat e indica
					el momento en que el usuario es aceptado.
	Parámetros:	- usr: Usuario que desea entrar en la sala de chat.
*/
void crearUsuario(infoUsr *usr) {

    pthread_mutex_lock(&mutexusr);
    char *buffer, *error_encontrado;
    int  tam = 0, encontrado = 0, i = 0, ok;
    leerSocket(usr->fd, &buffer);

    while((i<totalUsr)&&(!encontrado)){
        encontrado = !strcmp(buffer,(*losUsuarios[i]).nombre);
        i++;
    }

    if(encontrado){
        error_encontrado = (char *) malloc(sizeof(char)* 5);
        if(error_encontrado==NULL){ 
            perror("Error, solicitud de memoria denegada.\n");
            exit(1);
        }
        escribirSocket(usr->fd, error_encontrado);
        free(error_encontrado); 
        error_encontrado = error_handler(6);
        escribirSocket(usr->fd, error_encontrado);
        free(error_encontrado);
        ok = close(usr->fd);
        if (ok < 0){
            perror("Error, no se cerró el socket correctamente.\n");
            exit(2);
        }
        pthread_mutex_unlock(&mutexusr);
        ok = pthread_cancel(usr->id);
        if (ok != 0){ 
            perror("Error, no se pudo cancelar el hilo.\n");
            exit(3);
        }
    }  

    escribirSocket(usr->fd,"1");
    usr->nombre = buffer;
    usr->salas = (char **) malloc(sizeof(char *));
    if(usr->salas == NULL){
        perror("Error, solicitud de memoria denegada.\n");
        exit(4);
    }
    usr->salas[0] = (char *) calloc(sizeof(char), strlen(salas[0]));
    if (usr->salas[0]==NULL){
        perror("Error, solicitud de memoria denegada.\n");
        exit(5);
    }
    strcpy(usr->salas[0], salas[0]);
    usr->totalSalas = 1;
    usr->tamMaxSala = MAXTAM;

    if (totalUsr < tamMaxUsr){
        losUsuarios[totalUsr]= usr;
        usr->posarray = totalUsr++;

    } else {
        losUsuarios = (infoUsr **) realloc(losUsuarios,sizeof(infoUsr *)*
                (MAXTAM+tamMaxUsr));
        tamMaxUsr += MAXTAM;
        losUsuarios[totalUsr] = usr;
        usr->posarray = totalUsr++;
    }

    char *aviso = (char *) malloc(sizeof(char)*100);
    if (aviso == NULL){
        perror("Error, solicitud de memoria denegada.\n");
        exit(6);
    }
    sprintf(aviso,"Su solicitud ha sido aceptada, ha ingresado a la sala '%s' por defecto\n",salas[0]);
    incluir_verificacion(1, &aviso);
    escribirSocket(usr->fd, aviso);
    free(aviso);
    pthread_mutex_unlock(&mutexusr);
}

void eliminar_usuario(int posusr){

    int i =0;

    while(i<(*losUsuarios[posusr]).totalSalas){

        free((*losUsuarios[posusr]).salas[i]);
        i++;
    }
    free((*losUsuarios[posusr]).salas);
    free((*losUsuarios[posusr]).nombre);
    free(losUsuarios[posusr]);

}

void crear_sala(char *sala, int posusr){
    //revisar que no exista la sala
    int i =0, encontrado=0;
    char *error_encontrado;

    pthread_mutex_lock(&mutexsala);
    //Si el nombre de la sala deseado no fue ingresado, se arroja el error
    if(sala==NULL){

        error_encontrado = error_handler(5);
        incluir_verificacion(1, &error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        pthread_mutex_unlock(&mutexsala);
        return;

    }

    //Se verifica que la sala a agregar no exista ya
    while((i<totalSalas)&&(!encontrado)){

        encontrado= !strcmp(sala,salas[i]);
        i++;
    }

    if(encontrado){

        error_encontrado = error_handler(1);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        free(sala);
        pthread_mutex_unlock(&mutexsala);
        return;
    }

    if(totalSalas<tamMaxSala){

        salas[totalSalas++] = sala;

    }  else  {

        salas = (char **) realloc(salas,sizeof(char *)*
                (MAXTAM+tamMaxSala));
        tamMaxSala += MAXTAM;
        salas[totalSalas++] = sala;

    }    

    pthread_mutex_unlock(&mutexsala);

}

void suscribir_usuario(int posusr, char* sala){

    int i =0, encontrado=0;
    char *error_encontrado;    

    pthread_mutex_lock(&mutexusr);
    pthread_mutex_lock(&mutexsala); 

    //Se verifica que el usuario haya ingresado, en efecto, un nombre de sala
    if(sala==NULL){

        error_encontrado = error_handler(5);
        incluir_verificacion(1, &error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala);
        return;

    }
    //Se verifica que la sala deseada exista
    while((i<totalSalas)&&(!encontrado)){

        encontrado= !strcmp(sala,salas[i]);
        i++;
    }

    if(!encontrado){

        error_encontrado = error_handler(2);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        free(sala);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala);
        return;
    }    


    i =0;
    encontrado=0;
    //Se verifica que el usuario no este suscrito ya a esa sala
    while((i<(*losUsuarios[posusr]).totalSalas)&&(!encontrado)){

        encontrado= !strcmp(sala,(*losUsuarios[posusr]).salas[i]);
        i++;
    }

    if(encontrado){

        char *error_encontrado;
        error_encontrado = error_handler(3);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        free(sala);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala);
        return;
    }


    if((*losUsuarios[posusr]).totalSalas<(*losUsuarios[posusr]).tamMaxSala){

        (*losUsuarios[posusr]).salas[(*losUsuarios[posusr]).totalSalas++] = sala;

    }  else  {
        (*losUsuarios[posusr]).salas = (char **) realloc((*losUsuarios[posusr]).salas,
                sizeof(char *)*(MAXTAM +
                    (*losUsuarios[posusr]).tamMaxSala));
        (*losUsuarios[posusr]).tamMaxSala += MAXTAM;
        (*losUsuarios[posusr]).salas[(*losUsuarios[posusr]).totalSalas++] = sala;

    }


    pthread_mutex_unlock(&mutexusr);
    pthread_mutex_unlock(&mutexsala); 


}

void eliminar_sala(char *sala, int posusr){

    int i =0, j=0,k=0, tam=0, borrado=0, encontrado=0;
    char *error_encontrado;

    pthread_mutex_lock(&mutexusr);
    pthread_mutex_lock(&mutexsala); 

    //Se verifica que el usuario haya ingresado, en efecto, un nombre de sala
    if(sala==NULL){

        error_encontrado = error_handler(5);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala); 
        return;

    }

    //Se verifica que el usuario no pueda borrar la sala por defecto
    if(!strcmp(sala,salas[0])){

        error_encontrado = error_handler(4);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        free(sala);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala); 
        return;

    }

    //Se verifica que la sala que se desea eliminar exista
    while((i<totalSalas)&&(!encontrado)){

        encontrado= !strcmp(sala,salas[i]);
        i++;
    }

    if(!encontrado){

        error_encontrado = error_handler(2);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        free(sala);
        pthread_mutex_unlock(&mutexusr);
        pthread_mutex_unlock(&mutexsala); 
        return;
    }    

    i=0;
    //revisar que exista la sala y mutex
    while(strcmp(salas[k],sala)) k++;

    while(i<totalUsr){

        j=0;
        borrado=0;
        tam = (*losUsuarios[i]).totalSalas;
        while((j<tam)&&(!borrado)){

            if(!strcmp(sala, (*losUsuarios[i]).salas[j])){

                borrado = 1;
                (*losUsuarios[i]).totalSalas--;
                (*losUsuarios[i]).salas[j]=(*losUsuarios[i]).salas[tam-1];

            }
            j++;
        }
        i++;
    }
    free(sala);
    free(salas[k]);
    totalSalas--;
    salas[k]= salas[totalSalas];

    pthread_mutex_unlock(&mutexusr);
    pthread_mutex_unlock(&mutexsala);

}

/**
Metodo: desuscribir_usario
Descripcion: Satisface la ejecucion del comando "des". De-suscribe al usuario
de todas las salas a las cuales esta suscrito.
@param posusr posicion en el arreglo de usuarios de quien solicito la ejecucion del
comando "posusr"
*/
void desuscribir_usuario(int posusr){

    char **aux;
    int auxcant=0;

    pthread_mutex_lock(&mutexusr);

    aux = (*losUsuarios[posusr]).salas;
    (*losUsuarios[posusr]).salas = (char **) calloc(sizeof(char *), 
            MAXTAM);
    if((*losUsuarios[posusr]).salas==NULL){ 
        perror("Solicitud de memoria denegada");
        exit(20);
    }
    (*losUsuarios[posusr]).totalSalas = 0;
    (*losUsuarios[posusr]).tamMaxSala = MAXTAM;


    pthread_mutex_unlock(&mutexusr);

    int i =1;

    while(i<auxcant){

        free(aux[i]);
        i++;
    }
    free(aux);

}

/**
Metodo: enviar_mensaje
Descripcion: Satisface la ejecucion del comando "men". Envia a todos
los miembros de todas las salas de la cual es miembro
el cliente que lo solicito un mensaje que adjunta en la 
llamada
@param sala es el string que representa la informacion a transmitir
@param posusr posicion en el arreglo de usuarios de quien solicito la ejecucion del
comando "men"
*/
void enviar_mensaje(int posusr, char *mensaje){

    int i =0, j=0,k=0, tamlist=0, tamlist2=0, enviado=0;
    char *buffer, *error_encontrado;

    pthread_mutex_lock(&mutexusr);

    if(mensaje==NULL){

        error_encontrado = error_handler(5);
        incluir_verificacion(1,&error_encontrado);
        escribirSocket((*losUsuarios[posusr]).fd, error_encontrado);
        free(error_encontrado);
        pthread_mutex_unlock(&mutexusr);
        return;

    }


    tamlist = (*losUsuarios[posusr]).totalSalas;

    while(i<totalUsr){
        j=0;
        while(j<tamlist){
            k=0;
            tamlist2 =  (*losUsuarios[i]).totalSalas;
            while(k<tamlist2){

                if(!strcmp((*losUsuarios[posusr]).salas[j], 
                            (*losUsuarios[i]).salas[k])){

                    buffer = (char *) malloc(sizeof(char *)*
                            strlen((*losUsuarios[posusr]).nombre)+
                            strlen((*losUsuarios[posusr]).salas[j])+
                            strlen(mensaje));
                    if(buffer==NULL){
                        perror("Solicitud de memoria denegada");
                        exit(19);
                    }
                    sprintf(buffer,"%s@%s: %s",(*losUsuarios[posusr]).nombre,
                            (*losUsuarios[posusr]).salas[j], mensaje);
                    incluir_verificacion(1,&buffer);
                    escribirSocket((*losUsuarios[i]).fd, buffer);
                    free(buffer);
                }
                k++;

            }
            j++;

        }
        i++;
    }

    free(mensaje);
    pthread_mutex_unlock(&mutexusr);

}

/**
Metodo: salir
Descripcion: Satisface la ejecucion del comando "fue". Culmina la participacion
de un cliente en el servidor.
@param posusr posicion en el arreglo de usuarios de quien solicito la ejecucion del
comando "fue"
*/
void salir(int posusr){

    pthread_mutex_lock(&mutexusr);

    int k=0, tam=0, borrado=0, ok;
    char *tmp = (char *) malloc(sizeof(char *));
    if(tmp==NULL){
        perror("Solicitud de memoria denegada");
        exit(18);
    }
    sprintf(tmp," 1 ");
    escribirSocket((*losUsuarios[posusr]).fd, tmp);
    free(tmp);
    ok = close((*losUsuarios[posusr]).fd);
    if (ok==-1){
        perror("No se cerro socket correctamente.");
        exit(17);
    }

    eliminar_usuario(posusr);
    totalUsr--;
    k=posusr;
    while(k<totalUsr){
        losUsuarios[k]= losUsuarios[k+1];
        (*losUsuarios[k]).posarray--;
        k++;
    }

    pthread_mutex_unlock(&mutexusr);

}

/**
 * Metodo: listar_salas
 * Descripcion: satisface la ejecucion del comando "sal".
 * @param posusr posicion en el arreglo de usuarios de quien solicito la ejecucion del
 *        comando "sal"
 *        */
void listar_salas(int posusr){

    pthread_mutex_lock(&mutexsala);
    int i =0;
    char* buffer = (char *) malloc(sizeof(char));
    if(buffer==NULL){
        perror("Solicitud de memoria denegada");
        exit(16);
    }
    while(i<totalSalas){

        buffer = (char *) realloc(buffer, strlen(buffer) +
                strlen(salas[i]));
        sprintf(buffer,"%s\n%s",buffer,salas[i]);
        i++;
    }
    incluir_verificacion(1,&buffer);
    escribirSocket((*losUsuarios[posusr]).fd, buffer);
    free(buffer);
    pthread_mutex_unlock(&mutexsala);

}

/**
 * Metodo: listar_usuarios
 * Descripcion: satisface la ejecucion del comando "usu"
 * @param posusr posicion en el arreglo de usuarios de quien solicito la ejecucion del
 *        comando "usu"
 */
void listar_usuarios(int posusr){

    pthread_mutex_lock(&mutexusr);
    int i =0;
    char* buffer = (char *) malloc(sizeof(char));
    if(buffer==NULL){
        perror("Solicitud de memoria denegada");
        exit(15);
    }
    while(i<totalUsr){

        buffer = (char *)realloc(buffer, strlen(buffer) +
                strlen((*losUsuarios[i]).nombre));
        sprintf(buffer,"%s\n%s",buffer,(*losUsuarios[i]).nombre);
        i++;
    }
    incluir_verificacion(1, &buffer);
    escribirSocket((*losUsuarios[posusr]).fd, buffer);
    free(buffer);

    pthread_mutex_unlock(&mutexusr);
}

/**
Metodo:ejecutar_peticion
Descripcion: Funcion donde se redirige la peticion hacia la ejecucion
de un comando.
@param posusr posicion en el arreglo de usuarios de quien solicito la 
ejecucion de un comando  
@param peticion: arreglo que contiene el comando y un posible parametro
del mismo
*/
void ejecutar_peticion(int pos, char **peticion){
    if(!strcmp(peticion[0],"ver_salas")){

        listar_salas(pos);

    } else if(!strcmp(peticion[0],"ver_usuarios")){

        listar_usuarios(pos);

    } else if(!strcmp(peticion[0],"env_mensaje")){

        enviar_mensaje(pos, peticion[1]);

    } else if(!strcmp(peticion[0],"entrar")){

        suscribir_usuario(pos,peticion[1]);

    } else if(!strcmp(peticion[0],"des")){

        desuscribir_usuario(pos);

    } else if(!strcmp(peticion[0],"crear_sala")){

        crear_sala(peticion[1], pos);

    } else if(!strcmp(peticion[0],"elim_sala")){

        eliminar_sala(peticion[1], pos);

    } else if(!strcmp(peticion[0],"salir")){

        salir(pos);

    } else {

        char *error_encontrado;
        error_encontrado = error_handler(0);
        incluir_verificacion(1,&error_encontrado);        
        escribirSocket((*losUsuarios[pos]).fd, error_encontrado);
        free(error_encontrado);
        if (peticion[1]!=NULL) free(peticion[1]);

    }
}

/*
	Nombre: hiloServidor
	Descripción: Se encarga de llamar el hilo del servidor para así esperar
					las peticiones de un único cliente asignado.
	Parámetros:	- arg: Apuntador a un usuario para cumplir sus requerimientos
						en la llamada a un hilo.
*/
void *hiloServidor(void *arg) {
    infoUsr *usr = (infoUsr *) arg;
    pthread_t id = (*usr).id;
    crearUsuario(usr);
    char* buffer;
    char** sep; 
    int ok = 0;

    /* Espera por el input del cliente.*/
    while (1){
        leerSocket((*usr).fd,  &buffer);
        if(buffer == NULL) continue;
        sep = separar(buffer);
        free(buffer);
        ejecutar_peticion((*usr).posarray,sep);
        if(!strcmp(sep[0],"fue")) break;
        free(sep[0]);
        free(sep);
    }

    free(sep[0]);
    free(sep);
    pthread_cancel(id);
    if(ok != 0) {
        perror("Error, no se pudo cancelar el hilo.");
        exit(1);
    }
}

/* MAIN PRINCIPAL */
int main(int argc, char *argv[]) {

    /* Obtención de parámetros.
       	Se obtiene los parámetros por consola y se almacena el puerto del 
       	Servidor y el nombre y dirección absoluta o relativa de archivo de
       	texto que realiza operaciones de bitácora.
   	*/
    char *puerto = obtenerParametro("-l",argv,argc);
    char *bitacora = obtenerParametro("-b",argv,argc);

    if ((argc!=5)||!bitacora||!puerto)
        perror("Parámetros inválidos.\n");
    printf("Ingrese: scs_svr -l <puerto-servidor(local)> -b ");
    printf("<archivo_bitácora> \n");
    exit(1);

    /* Se abre el archivo de la bitácora. */
    FILE *fd;
    fd = fopen(bitacora,"a");
    if (fd == NULL){
        perror("Error abriendo la bitácora.\n");
        exit(2);
    }

    /*
       - s: 		Socket de conexión del Servidor.
       - local:		Dirección local del Servidor.
       - cliente:	Direccción del Cliente.
   	*/
    int s;
    struct sockaddr_in local;
    struct sockaddr_in cliente;

    /* Creación del socket.
       	El dominio es AF_INET, que representa IPv4.
       	El tipo es SOCK_STREAM, que indica el servicio fiable orientado a la 
       	conexión.
       	El protocolo es TCP, que se indica con 0.
   	*/
    s = socket(AF_INET,SOCK_STREAM,0);	
    if (s < 0){
        perror("Error creando el socket del Servidor.\n");
        exit(3);
    }

    /* Asignación de la dirección al socket.
       Se usa AF_INET para indicar la version IPv4.
       La dirección IP local se obtiene con INADDR_ANY.
       Y se le asigna el puerto local.
   	*/
    bzero(&local,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY); 
    local.sin_port = htons(atoi(puerto));

    /* Asignación de la dirección al socket creado. */
    if ((bind(s, (struct sockaddr *) &local, sizeof(local))) < 0){
        perror("Error creando el bind el Servidor.\n");
        exit(4);
    }

    /* Comienza a escuchar esperando peticiones del Cliente.
        Puede escuchar hasta un máximo de 1024.
   	*/
    if ((listen(s,1024)) < 0){
        perror("Error escuchando el socket.\n");
        exit(5);	
    }

    /* Se obtiene la fecha y hora.
        Y se indica tanto en la bitácora como en la pantalla del Servidor.
   	*/
    char *tiempo = obtenerFechaHora();
    fprintf(fd,"%s Socket abierto en el puerto %s y esperando conexión..\n",tiempo,puerto);
    printf("%s Socket abierto en el puerto %s y esperando conexión..\n",tiempo,puerto);

    /* Se inicializan los usuarios y las salas.*/
    totalUsr = 0;
    totalSalas = 1;
    losUsuarios = (infoUsr **) malloc(sizeof(infoUsr *));
    if (losUsuarios == NULL){
        perror("Error, solicitud de memoria denegada.\n");
        exit(6);
    }
    salas = (char **) malloc(sizeof(char *));
    if (salas == NULL) {
        perror("Error, solicitud de memoria denegada.\n");
        exit(7);
    }
    tamMaxSala = MAXTAM;
    tamMaxUsr = MAXTAM;
    salas[0] = "SALA_1";
    free(puerto);

    /* En caso de haber recibido una señal de salida forzada (cntrl + c) */
    //signal(SIGINT, salidaForzada);	


    /*
       - clienteTam: 	Tamaño de la dirección del cliente.
       - nuevoS:		Nuevo Socket.
       - usr:			Usuario actual.
   	*/

    int clienteTam, nuevoS;

    /* Ciclo infinito esperando las peticiones del cliente.
       Se van creando los sockets con las peticiones y se va informando
       también en la bitácora.
    */
    while(42) {
        clienteTam = sizeof(cliente);
        nuevoS = accept(s,(struct sockaddr *) &cliente, &clienteTam);
        if (nuevoS < 0) {
            perror("Error, no se pudo establecer la conexión.\n");
            exit(8);
        }
        tiempo = obtenerFechaHora();
        fprintf(fd,"%sNuevo usuario conectado al servidor\n",tiempo);
        printf("%sNuevo usuario conectado al servidor\n",tiempo);
        infoUsr *usr = (infoUsr *) calloc(sizeof(infoUsr),1);
        if (usr == NULL){
            perror("Error, solicitud de memoria denegada.\n");
            exit(9);
        }
        usr->fd = nuevoS;
        if (pthread_create(&(usr->id),NULL,hiloServidor,(void *) usr)){
            perror("Error en la creación del hilo en el Servidor.\n");
            exit(10);
        }
    } 
}
