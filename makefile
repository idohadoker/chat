all: client server 

client: 
	gcc client.c -lpthread -o client 

server: 
	gcc server.c -lpthread -o server

clean:
	rm *.o