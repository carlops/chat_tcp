/*
 * EXTRAS
 *
 * funciones auxiliares y eso
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Herror(char *er){
    char msgErr = "Error manejando ";
    strcat(msgErr,er);
    perror(msgErr);
    exit(1);
}
