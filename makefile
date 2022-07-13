all: client server 

client: 
	gcc client.c -lpthread -o client.o

server: 
	gcc server.c -lpthread -o server.o

clean:
	rm *.o