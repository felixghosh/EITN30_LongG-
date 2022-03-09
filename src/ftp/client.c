#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 65536
#define MUADDR "192.168.0.3"
#define BSADDR "192.168.0.1"
#define MUADDRE "130.235.200.108"
#define BSADDRE "130.235.200.109"
#define BUFSIZ 1448
#define PORT 8080

void getFile(int socket_fd);
void red();
void green();
void reset();

void send_file(FILE *fp, int sockfd)
{
    int n;
    char data[SIZE] = {0};

    while (fgets(data, SIZE, fp) != NULL)
    {
        if (send(sockfd, data, 200, 0) == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(data, SIZE);
    }
}

int main()
{
    char *ip = MUADDRE;
    int port = 8080;
    int e;

    int sockfd;
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("\033[0;31m[-]\033[0m Error in socket");
        exit(1);
    }
    printf("\033[0;32m[+]\033[0m Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (e == -1)
    {
        perror("\033[0;31m[-]\033[0m Error in socket");
        exit(1);
    }
    printf("\033[0;32m[+]\033[0m Connected to Server.\n");

    bool finished = false;
    while (!finished)
    {
        printf("\n\033[0;32m[+]\033[0m Please select command to perform: \n 1 - Get file \n 2 - Put file\n 3 - Exit\n");
        int userInput = getchar();
        while(getchar() != '\n');
        switch (userInput)
        {
        case 49:
            getFile(sockfd);
            break;

        case 50:

            break;

        case 51:

            break;

        default:
            printf("vafan: %d\n", userInput);
            printf("\033[0;31m[-]\033[0m Please enter a correct command!\n");
            break;
        }
    }

    return 0;
}

void getFile(int socket_fd) {
    printf("\033[0;32m[+]\033[0m Command chosen: \033[0;32mGET\033[0m\n");
    char msg[4];
    memset(msg, 0 , 4);
    strcpy(msg, "GET"); 
    //printf("sending command GET\n");
    send(socket_fd, msg, 4, 0);
    char ok_flag[3];
    //printf("\033[0;33m[/]\033[0m Waiting for ok flag\n");
    recv(socket_fd, ok_flag, sizeof ok_flag, 0);
    
    
    if (strcmp(ok_flag, "OK") != 0) {
        printf("\033[0;31m[-]\033[0mDid not receive ok_flag");
        return;
    }
    //else printf("\033[0;32m[+]\033[0m Ok flag received\n");

    char fp[100];
    memset(fp, 0, 100);
    printf("\033[0;33m[/]\033[0m Please specify the file path:\n");
    scanf("%s", &fp);
    int i = send(socket_fd, fp, 100, 0);
    size_t file_size;
    printf("\033[0;33m[/]\033[0m Awating file size\n");
    recv(socket_fd, &file_size, sizeof file_size, 0);
    printf("\033[0;32m[+]\033[0m File size received: \033[0;33m%lu\033[0m\n", file_size);
    char data[file_size + 1];
    memset(data, 0, file_size + 1);
    //printf("opening file\n");
    FILE* f = fopen(fp, "w");
    printf("\033[0;32m[+]\033[0m Receiving file\n");
    size_t bytes_received = 0;
    char temp[file_size + 1];
    while(bytes_received < file_size){
        memset(temp, 0, file_size + 1);
        bytes_received += recv(socket_fd, temp, file_size, 0);
        //printf("bytes_received: %lu, %d%c done\n", bytes_received, (int)((bytes_received/file_size) * 100), '%');
        strcat(data, temp);
    }
    
    printf("\033[0;32m[+]\033[0m File received!\n");
    data[bytes_received] = '\0';
    fputs(data, f);
    fclose(f);
}

void red()
{
    printf("\033[1;31m");
}

void green()
{
    printf("\033[0;32m");
}

void reset()
{
    printf("\033[0m");
}