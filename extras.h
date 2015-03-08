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
    Descripción: Se encarga de obtener el argumento que corresponde a un cierto 
                    flag de la entrada.
    Argumentos: - flag: String que puede ser -d, -p, -l.
                - argv: Arreglo que representa los argumentos introducidos.
                - argc: Entero que representa el número total de argumentos.
    Retorno: String con el argumento correspondiente al flag indicado.
*/
extern char *obtenerParametro(char *flag, char *argv[], int argc);


/*
    Nombre: escribirSocket
    Descripción: Se encarga de escribir el tamaño del mensaje y el mensaje en
                    el socket del Cliente.
                    Si el tamaño de mensaje es 0, se toma como finzalización
                    forzada debido al comando ctrl + c.
    Argumentos: - s: File descriptor del socket del Cliente.
                - msj: String que representa el mensaje que se desea enviar.
*/
extern void escribirSocket(int s, char* msj);


/*  Nombre: leerSocket
    Descripción: Se encarga de leer un mensaje del socket del Cliente.
                    Lee primero el tamaño del mensaje y despues el mensaje.
    Argumentos: - s: File descriptor del socket del Cliente.
                - msj: Arreglo de strings que representa el mensaje leido.
*/
extern void leerSocket(int s, char** msj);


/*  Nombre: verificar
    Descripción: Se encarga de tomar el mensaje leido del socket del Cliente y
                    verificar si es el mensaje de culminación del programa o no.
    Parámetros: - mjs: Arreglo de Strings que representa el mensaje a ser
                        enviado.
    Retorno: Entero que representa si se termina el programa o no.

*/
extern int verificar(char** msj);


/*
    Nombre: separar
    Descripción: Se encarga de separar el comando introducido en el nombre
                    del comando y su argumento.
    Parámetros: - comando : String que representa el comando introducido.
    Retorno: Arreglo de Strings en donde el primer elemento es el comando y el
                segundo es el argumento.
*/
extern char **separar (char *comando);


extern char *error_handler(int numerror);