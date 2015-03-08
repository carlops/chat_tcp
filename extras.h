/*
 *	Proyecto: Sistema de Chat Simple (SCS)
 *	
 * 	Módulo Extra
 *	extras.h
 *
 *	Descripción:	Este módulo contiene las firmas de los métodos extras.
 *
 * 	Fecha:	11/03/2015
 *
 *	Autores:	09-10329 María Gabriela Giménez
 *				09-10672 Carlo Polisano
 *				09-10971 Alejandro Guevara
 *				10-10534 Jesús Adolfo Parra
 *
 */


/*
    Nombre: obternerParametro
    Descripcion: Se encarga de obtener el argumento que corresponde a un cierto 
                    flag de la entrada.
    Argumentos: -flag: String que puede ser -d, -p, -l.
                -argv: Arreglo que representa los argumentos introducidos.
                -argc: Entero que representa el numero total de argumentos.
    Return: String con el argumento correspondiente al flag indicado.
*/
extern char *obtenerParametro(char *flag,char *argv[], int argc);

// Struct para los mensajes del cliente al servidor
struct mensaje_cliente {
	char cmd;
	char tiempo[20];
	char usuario[50];
	char mensaje[500];
};

// Struct para los mensajes del servidor al cliente
struct mensaje_servidor {
	char cmd;
	char tiempo[20];
	char usuario[50];
	char ack;
	char mensaje[500];
};

// Alias para los mensajes
typedef struct mensaje_cliente msj_cli;
typedef struct mensaje_servidor msj_srv;

/*
void Herror(char *er){
    char msgErr = "Error manejando ";
    strcat(msgErr,er);
    perror(msgErr);
    exit(1);
}
*/