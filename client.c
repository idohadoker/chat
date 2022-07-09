#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

#define port 4220

// Global variables
int flag = 0;
int sockfd = 0;
char name[32], ok[2];
int numlen(int room)
{
    int c = 1;
    while (room)
    {
        room % 10;
        room /= 10;
        c++;
    }
    return c;
}
void str_overwrite_stdout()
{
    printf("%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
    { // trim \n
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}

void send_msg_handler()
{
    char message[BUFSIZ] = {};
    char buffer[BUFSIZ + 34] = {};

    while (1)
    {
        fgets(message, BUFSIZ, stdin);
        str_trim_lf(message, BUFSIZ);
        if (strncmp(message, "exit", 5) == 0)
        {
            send(sockfd, message, strlen(message), 0);
            flag = 1;
            break;
        }
        else
        {
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%s: %s\n", name, message);
            if (strlen(message) >= 1)
                send(sockfd, buffer, strlen(buffer), 0);
        }
        bzero(message, BUFSIZ);
        bzero(buffer, sizeof(buffer));
        str_overwrite_stdout();
    }
}

void recv_msg_handler()
{
    char message[BUFSIZ] = {};
    while (1)
    {
        int receive = recv(sockfd, message, BUFSIZ, 0);
        if (receive > 0)
        {
            printf("%s", message);

            str_overwrite_stdout();
        }
        else if (receive == 0)
        {
            break;
        }
        else
        {
            exit(1);
        }
        bzero(message, sizeof(message));
    }
}

int main(int argc, char **argv)
{
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("ERROR: connect\n");
        exit(1);
    }
    int room = 0;
    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));
    if (strlen(name) > 32 || strlen(name) < 2)
    {
        printf("Name must be less than 30 and more than 2 characters.\n");
        exit(1);
    }
    printf("Please enter your room: ");
    scanf("%d", &room);
    if (room <= 0 && !isdigit(room))
    {
        printf("room not valid \n");
        exit(1);
    }
    char *nums = NULL;
    nums = (char *)malloc(numlen(room));
    if (nums == NULL)
    {
        printf("memory allocation failure\n");
        exit(1);
    }
    sprintf(nums, "%d", room);
    str_trim_lf(nums, strlen(nums));
    send(sockfd, name, sizeof(name), 0);
    recv(sockfd, ok, sizeof(ok), 0);
    send(sockfd, nums, strlen(nums), 0);
    printf("=== WELCOME TO THE CHATROOM %d ===\n", room);
    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        exit(1);
    }
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        exit(1);
    }

    while (1)
    {
        if (flag)
        {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);

    exit(0);
}