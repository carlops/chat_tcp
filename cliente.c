/* 
 * CLIENTE
 * 
 * compilar con: (-g es opcional para debuggear)
 * gcc -g -o scs_cli cliente.c extras.c
 *
 * Se corre asi:
 * scs_cli -d <IP-servidor> -p <puerto-servidor> [-l <puerto_local>]
 * con lo que va en [] opcional.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "extras.h"

int main(int argc, char *argv[]) {

	char *host = obtener_parametros("-d",argv,argc);
	char *puerto = obtener_parametros("-p",argv,argc);
	char *puerto_local = obtener_parametros("-l",argv,argc);

	if ((argc%2!=1)||(argc>7)||!host||!puerto)
		perror("Parametros invalidos");

	int s;					  // El socket de conexion
	struct sockaddr_in local; // Direccion local
	struct sockaddr_in serv;  // Direccion del servidor
	struct hostent *servidor; // Para gethostbyname() o gethostbyaddr()
							  //  segun sea el caso
	struct in_addr inaddr;	  // Utilizado cuando es suministrada el dominio
							  //  como direccion IP

	/* Creacion del socket */
	s = socket(PF_INET, SOCK_STREAM, 0);
	/* comprobacion de errores */
	if (s<0) {
			perror("creando socket:");
			exit(1);
	}
	/* Asignacion de direccion al socket.
	 * En el caso del cliente el numero de puerto local,
	 * es suministrado por en la invocacion del cliente con la bandera '-l',
	 * este parametro es opcional. 
	 * En caso de no sea suministrado podemos dejar que lo elija el sistema,
	 * dandole el valor cero. 
	 * Como direccion IP daremos INADDR_ANY 
	 */
	local.sin_family=AF_INET;
	if (puerto_local)
			local.sin_port=htons(atoi(puerto_local));
	else
			local.sin_port=htons(0);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	/* En este caso no es totalmente necesario ejecutar el bind()en el cliente,
	 * solamente es necesario cuando se es suministrado el puerto local,
	 * ya que  si intentamos directamente hacer connect() sin haber hecho
	 * antes bind(), el sistema nos asignara automaticamente una direccion 
	 * local elegida por el.
	 */
	if (bind(s, (struct sockaddr *) &local, sizeof local)<0) {
			perror("asignando direccion:");
			exit(2);
	}

	/* Ahora vamos a conectar. Necesitamos la direccion IP del servidor
	 * la cual obtenemos con una llamada a gethosbyname() o con
	 * gethostbyaddr() segun sea el caso
	 */
	if (inet_aton(host,&inaddr))
			servidor = gethostbyaddr((char *) &inaddr, sizeof(inaddr), AF_INET);
	else
			servidor = gethostbyname(host);
	if (servidor==NULL) {
			fprintf(stderr, "Nombre de Dominio o Direccion IP erronea.\n");
			exit(2);
	}
	memcpy(&serv.sin_addr.s_addr, servidor->h_addr, servidor->h_length);
	/* Llenando los ultimos campos de serv */
	serv.sin_family = AF_INET;
	serv.sin_port = htons(atoi(puerto));
	/* Conectando */
	if (connect(s, (struct sockaddr *) &serv, sizeof(serv))<0) {
			perror("La coneccion al servidor fallo.");
			exit(3);
	}
	/* Si hemos llegado hasta aqui, la conexion esta establecida */
	if (write(s, DATOS, sizeof DATOS)<0) {
			perror("escribiendo el socket:");
			exit(3);
	}
	/* Todo bien. Podemos cerrar el socket y terminar */
	close(s);
	exit(0);
}
