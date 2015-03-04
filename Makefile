C = gcc
CFLAGS = -g
#LIBS = -lsocket -lnsl

all : schat cchat

procedimientos.o : extras.c extras.h
		$(CC) $(CFLAGS) -c extras.c

schat : schat.c procedimientos.o
		$(CC) $(CFLAGS) servidor.c extras.o -lpthread -o scs_ser

cchat : cchat.c procedimientos.o
		$(CC) $(CFLAGS) cliente.c extras.o -lpthread -o scs_cli 

clean:
		rm *.o scs_ser scs_cli *.c~



