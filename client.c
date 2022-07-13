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

#define port 4221

// Global variables
int flag = 0;
int sockfd = 0;
char name[32];
char *firstMsg = NULL;
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
// gets length of message and message and changes \n to \0 end of string
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
// sends the message plus the name
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
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%s: %s\n", name, message);
            if (strlen(message) >= 1)
                send(sockfd, buffer, strlen(buffer), 0);
        }
        memset(message, 0, BUFSIZ);
        memset(buffer, 0, sizeof(buffer));
        str_overwrite_stdout();
    }
}
// gets the message and prints
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
        memset(message, 0, sizeof(message));
    }
}
int init_socket()
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
    return sockfd;
}
void get_name_and_room()
{
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
    firstMsg = (char *)malloc(strlen(name) + strlen(nums) + 1);
    if (firstMsg == NULL)
    {
        printf("memory allocation failure\n");
        exit(1);
    }
    strncpy(firstMsg, name, strlen(name));
    strncat(firstMsg, "-", 3);
    strncat(firstMsg, nums, strlen(nums));
    send(sockfd, firstMsg, strlen(firstMsg), 0);
    printf("=== WELCOME TO THE CHATROOM %d ===\n", room);
}
void send_recv()
{
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
}
int main(int argc, char **argv)
{
    sockfd = init_socket();
    get_name_and_room();
    send_recv();
    exit(0);
}