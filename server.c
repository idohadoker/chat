#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#define port 4220
static int size;
int uid = 10;
#define BUFFER_SZ 2048
typedef struct
{
    struct sockaddr_in address;
    int clSock;
    int room;
    int uid;
    char name[32];
} client_t, *clientptr;

client_t **clients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message(char *s, int userId, int room)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < size; ++i)
    {
        if (clients[i] != NULL)
        {
            if (clients[i]->room == room)
                if (clients[i]->uid != userId)
                {
                    int status = send(clients[i]->clSock, s, strlen(s), 0);
                    if (status == -1)
                    {
                        perror("ERROR: write to descriptor failed");
                        exit(1);
                    }
                }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_add(client_t *cl)
{
    pthread_mutex_lock(&clients_mutex);
    int flag = 1;
    int num = 1;
    for (int i = 0; i < size && flag; ++i)
    {
        if (!clients[i])
        {
            clients[i] = cl;
            flag = 0;
        }
    }
    if (flag)
    {
        clients = (clientptr *)realloc(clients, (size + 1) * sizeof(clientptr));
        if (clients == NULL)
        {
            printf("memory allocation failure\n");
            exit(1);
        }

        clients[size++] = cl;
    }
    pthread_mutex_unlock(&clients_mutex);
}
void queue_remove(int uid)
{
    pthread_mutex_lock(&clients_mutex);
    int flag = 1;
    for (int i = 0; i < size && flag; ++i)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                clients[i] = clients[size - 1];
                // flag--;
                flag = 0;
            }
        }
    }
    clients = (clientptr *)realloc(clients, (size--) * sizeof(clientptr));
    pthread_mutex_unlock(&clients_mutex);
}
void *handle_client(client_t *cli)
{
    char buff_out[BUFFER_SZ];
    char buffnum[BUFFER_SZ];
    char name[32];
    int flag = 1;
    recv(cli->clSock, name, 32, 0);
    send(cli->clSock, "ok", 2, 0);
    bzero(buffnum, sizeof(buffnum));
    recv(cli->clSock, buffnum, BUFFER_SZ, 0);

    if (strlen(name) < 2 || strlen(name) >= 32 - 1)
    {
        printf("name is not legal.\n");
        flag = 0;
    }
    else
    {
        strncpy(cli->name, name, strlen(name));
        cli->room = atoi(buffnum);
        bzero(buff_out, BUFFER_SZ);
        bzero(buffnum, BUFFER_SZ);
        sprintf(buff_out, "%s has joined to room %d\n", cli->name, cli->room);
        puts(buff_out);
        send_message(buff_out, cli->uid, cli->room);
    }
    while (flag)
    {
        bzero(buff_out, BUFFER_SZ);
        int receive = recv(cli->clSock, buff_out, BUFFER_SZ, 0);
        if (receive > 0)
        {
            if (strncmp(buff_out, "exit", sizeof(buff_out)) == 0)
            {
                sprintf(buff_out, "%s has left \n", cli->name);
                printf("%s", buff_out);
                send_message(buff_out, cli->uid, cli->room);
                flag = 0;
            }
            else if (strlen(buff_out) > 0)
            {
                send_message(buff_out, cli->uid, cli->room);
            }
        }
        else if (receive < 0)
        {
            printf("ERROR: -1\n");
            flag = 0;
        }
        bzero(buff_out, BUFFER_SZ);
    }
    close(cli->clSock);
    queue_remove(cli->uid);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    pthread_t tid;
    char serMsg[255];
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(servSockD, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1)
        exit(1);
    if (listen(servSockD, size + 1) == -1)
        exit(1);
    while (1)
    {
        socklen_t clilen = sizeof(clientAddr);
        int clientSocket = accept(servSockD, (struct sockaddr *)&clientAddr, &clilen);
        printf("Connection succeed\n");
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        if (cli == NULL)
        {
            printf("memory allocation failure\n");
            exit(1);
        }
        cli->address = clientAddr;
        cli->clSock = clientSocket;
        cli->uid = uid++;
        queue_add(cli);
        pthread_create(&tid, NULL, (void *)&handle_client, cli);
    }
    return 0;
}