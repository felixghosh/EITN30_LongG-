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
    FILE *fp;
    char *filename = calloc(128, sizeof(char));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (e == -1)
    {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    bool finished = false;
    while (!finished)
    {
        printf("Please select command to perforn \n 1 - Get file \n 2 - Put file\n 3 - Exit\n");
        int userInput = getchar();
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
            break;
        }
    }

    return 0;
}

void getFile(int socket_fd) {
    printf("getting file!\n");
    char msg[4];
    memset(msg, 0 , 4);
    strcpy(msg, "GET"); 
    printf("sending command GET\n");
    printf("%d\n", send(socket_fd, msg, 4, 0));
    char ok_flag[3];
    printf("waiting for ok flag\n");
    recv(socket_fd, ok_flag, sizeof ok_flag, 0);
    
    
    if (strcmp(ok_flag, "OK") != 0) {
        printf("Did not receive ok_flag");
        return;
    }
    else printf("ok flag received\n");

    char fp[BUFSIZ];
    printf("Please specify the file path:\n");
    scanf("%s", &fp);
    printf("%s\n", fp);
    printf("sending file path\n");
    int i = send(socket_fd, fp, BUFSIZ, 0);
    printf("i = %d BUFSIZ = %d\n", i, BUFSIZ);
    int file_size;
    printf("waiting for file size\n");
    recv(socket_fd, &file_size, sizeof(int), 0);
    printf("file size: %d\n", file_size);
    char* data = malloc(file_size + 1);
    printf("opening file\n");
    FILE* f = fopen(fp, "w");
    printf("receiveing file\n");
    int x = recv(socket_fd, data, file_size, 0);
    printf("file received!\n");
    data[x] = '\0';
    fputs(data, f);
    fclose(f);
}
