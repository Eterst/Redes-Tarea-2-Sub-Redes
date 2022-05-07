#ESTO COMPILA EL PROGRAMA PRINCIPAL FUERA DE CONTENEDOR
server: server/lib/server.c
	gcc server/lib/server.c -o server.o -lpthread
	./server.o 9801
clean:
	rm *.o