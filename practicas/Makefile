CFLAGS= -g -O0
all: lista.o p3.o lista1.o
	gcc $(CFLAGS) -o shell p3.o lista.o lista1.c
	
p2.o:
	gcc $(CFLAGS) -c p3.c
		
lista.o:
	gcc $(CFLAGS) -c lista.c 

lista1.o:
	gcc $(CFLAGS) -c lista1.c 
clean:
	rm -f *.o shell

	
