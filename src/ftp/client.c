#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define SIZE 65536
#define MUADDR "192.168.0.3"
#define BSADDR "192.168.0.1"
#define MUADDRE "130.235.200.108"
#define BSADDRE "130.235.200.109"
#define BUFSIZ 1448
#define PORT 8080

void getFile(int socket_fd);
void putFile(int socket_fd);
void ext(int sockfd);
void send_file(char *fp, int conn_sock_fd);
void red();
void green();
void reset();


int main()
{
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
    server_addr.sin_addr.s_addr = inet_addr(MUADDR);

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
        fflush(stdin);
        printf("\n\033[0;32m[+]\033[0m Please select command to perform: \n 1 - Get file \n 2 - Put file\n 3 - Exit\n");
        fflush(stdin);
        int userInput;
        while((userInput = getchar()) == '\n');
        while(getchar() != '\n');
        switch (userInput)
        {
        case 49:
            getFile(sockfd);
            break;

        case 50:
            putFile(sockfd);
            break;

        case 51:
            ext(sockfd);
            finished = true;
            
            break;

        default:
            printf("input: %d\n", userInput);
            printf("\033[0;31m[-]\033[0m Please enter a correct command!\n");
            break;
        }
    }
    //close(sockfd);
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

    char fp[100];
    memset(fp, 0, 100);
    
    printf("\033[0;33m[/]\033[0m Please specify the file path:\n");
    scanf("%s", &fp);
    send(socket_fd, fp, 100, 0);
    size_t file_size;
    printf("\033[0;33m[/]\033[0m Awating file size\n");
    recv(socket_fd, &file_size, sizeof file_size, 0);
    printf("\033[0;32m[+]\033[0m File size received: \033[0;33m%lu\033[0m\n", file_size);
    char* data = calloc(1, file_size + 1);
    FILE* f = fopen(fp, "wb");
    printf("\033[0;32m[+]\033[0m Receiving file\n");
    size_t bytes_received = 0;
    char* temp = calloc(1, file_size + 1);
    clock_t t = clock();
    while(bytes_received < file_size){
        memset(temp, 0, sizeof temp);
        size_t prev_index = bytes_received;
        bytes_received += recv(socket_fd, temp, file_size, 0);
        for(int i = 0; i < bytes_received - prev_index; i++)
            data[prev_index + i] = temp[i];
    }
    t = clock() - t;
    
    printf("\033[0;32m[+]\033[0m File received!\n");
    //fputs(data, f);
    fwrite(data, 1, file_size, f);
    fclose(f);
    double time_elapsed = ((double)t)/CLOCKS_PER_SEC * 1000;
    printf("Elapsed time: %f\n", time_elapsed);
    free(data);
    free(temp);
}

void putFile(int socket_fd) {
    printf("\033[0;32m[+]\033[0m Command chosen: \033[0;32mPUT\033[0m\n");
    char msg[4];
    memset(msg, 0 , 4);
    strcpy(msg, "PUT"); 
    //printf("sending command GET\n");
    send(socket_fd, msg, 4, 0);
    char ok_flag[3];
    //printf("\033[0;33m[/]\033[0m Waiting for ok flag\n");
    recv(socket_fd, ok_flag, sizeof ok_flag, 0);

    if (strcmp(ok_flag, "OK") != 0) {
        printf("\033[0;31m[-]\033[0mDid not receive ok_flag");
        return;
    }

    char fp[100];
    memset(fp, 0, 100);
    printf("\033[0;33m[/]\033[0m Please specify the file path:\n");
    scanf("%s", &fp);

    if (access(fp, F_OK) != -1) {
        printf("\033[0;32m[+]\033[0m File exists. Sending file to server!\n");
        printf("fp size: %d\n", sizeof fp);
        send(socket_fd, fp, 100, 0);
        send_file(fp, socket_fd);
        printf("\033[0;32m[+]\033[0m File sent!\n");
    }
}

void ext(int socket_fd){
    printf("\033[0;32m[+]\033[0m Command chosen: \033[0;32mEXT\033[0m\n");
    char msg[4] = "EXT";
    //memset(msg, 0 , 4);
    //strcpy(msg, "EXT"); 
    send(socket_fd, msg, 4, 0);
}

void send_file(char *fp, int conn_sock_fd) {
    printf("1\n");
    struct stat file_stats;
    stat(fp, &file_stats);
    printf("2\n");
    size_t file_size = file_stats.st_size;
    printf("file size: %lu\n", file_size);
    int x = send(conn_sock_fd, &file_size, sizeof(size_t), 0);
    printf("send: %d\n", x);
    printf("3\n");
    int file_fd = open(fp, O_RDONLY);
    size_t bytes_sent = 0;
    off_t *offset;
    printf("4\n");
    while (bytes_sent < file_size) {
        bytes_sent += sendfile(conn_sock_fd, file_fd, offset, file_size);
        //printf("bytes_sent: %lu, %f%c done\n", bytes_sent, ((bytes_sent/file_size) * 100), '%');
    }
    printf("5\n");
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