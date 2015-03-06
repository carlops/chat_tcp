# Makefile 

CFLAGS	= -g
CHFLAGS	= -pthread 
CC	= gcc
OBJSer	= servidor.c extras.c
OBJCli	= cliente.c extras.c
OBJS	= servidor.c cliente.c extras.c

servidor: 
	
		$(CC) $(CFLAGS) $(CHFLAGS) -o scs_svr $(OBJSer)

cliente: 

		$(CC) $(CFLAGS) -o scs_cli $(OBJCli)

all: 
		
		$(CC) $(CFLAGS) $(CHFLAGS) -o $(OBJS)

cleanser:
	-@rm -f *.o scs_svr

cleancli:
	-@rm -f *.o scs_cli

clean:
	-@rm -f *.o scs_* 
