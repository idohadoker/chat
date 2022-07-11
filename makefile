all: client server 

client: client
gcc client.c -o client

server: server
gcc server.c -o server

clean:
rm *.o