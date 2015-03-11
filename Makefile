# Makefile 

CFLAGS	= -g
CHFLAGS	= -pthread 
CC	= gcc
OBJSer	= servidor.c extras.o
OBJCli	= cliente.c extras.o

all: servidor cliente

extras.o: extras.c extras.h
	$(CC) $(CFLAGS) -c extras.c

servidor: servidor.c extras.o
	$(CC) $(CFLAGS) $(OBJSer) $(CHFLAGS) -o scs_svr 

cliente: 
	$(CC) $(CFLAGS) $(OBJCli) $(CHFLAGS) -o scs_cli 

clean:
	-@rm -f *.o scs_* 
