#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define SIZE 1024

#define MUADDR "192.168.0.3"
#define BSADDR "192.168.0.1"
#define MUADDRE "130.235.200.108"
#define BSADDRE "130.235.200.109"
#define BUFSIZ 1448
#define PORT 8080
 
void handle_connection(int socket);
int getCommand(char* clientMessage);
void getFile(int conn_sock_fd);
void putFile(int conn_sock_fd);
void terminateConnection(int conn_sock_fd);
void send_file(char* fp, int conn_sock_fd);

 
int main(){
  char *ip = BSADDRE;
  int e;
 
  int listen_sock_fd, conn_sock_fd;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;
  char buffer[SIZE];
 
  listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(listen_sock_fd < 0) {
    perror("[-]Error in socket");
    exit(1);
  }
  if (setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
  printf("[+]Server socket created successfully.\n");
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = PORT;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
  e = bind(listen_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(e < 0) {
    perror("[-]Error in bind");
    exit(1);
  }
  printf("[+]Binding successfull.\n");
 
  if(listen(listen_sock_fd, 10) == 0){
 printf("[+]Listening....\n");
 }else{
 perror("[-]Error in listening");
    exit(1);
 }
 
  addr_size = sizeof(new_addr);
  conn_sock_fd = accept(listen_sock_fd, (struct sockaddr*)&new_addr, &addr_size);
  handle_connection(conn_sock_fd);
  
 
  return 0;
}


void handle_connection(int conn_sock_fd){
    bool finished = false;

    while(!finished){
        char clientMessage[4], serverResponse[BUFSIZ];
        printf("1\n");
        recv(conn_sock_fd, clientMessage, 4, 0);
        printf("2\n");
        int command = getCommand(clientMessage);
        switch (command)
        {
        case 0:
            getFile(conn_sock_fd);
            break;
        case 1:
            putFile(conn_sock_fd);
            break;
        case 2:
            terminateConnection(conn_sock_fd);
            finished = true;
            break;
        
        default:
            break;
        }
        
    }
    
return;
}

int getCommand(char* clientMessage){
    printf("getCommand\n");
    if(strcmp(clientMessage, "GET") == 0)
        return 0;
    else if(strcmp(clientMessage, "PUT") == 0)
        return 1;
    else if(strcmp(clientMessage, "EXIT") == 0)
        return 2;
}

void getFile(int conn_sock_fd){
    printf("getFile\n");
    FILE f;
    char fp[BUFSIZ];
    char okBuf[3] = "OK";
    printf("sending ok!\n");
    send(conn_sock_fd, okBuf, sizeof okBuf, 0);
    printf("waiting for file path\n");
    int x = recv(conn_sock_fd, fp, BUFSIZ, 0);
    printf("x = %d fp = %s\n", x, fp);
    printf("checking access\n");
    if(access(fp, F_OK) != -1) {
        printf("access ok! sening file\n");
        send_file(fp, conn_sock_fd);
        printf("file sent!\n");
    }

}

void putFile(int conn_sock_fd){

}

void terminateConnection(int conn_sock_fd){

}

void send_file(char* fp, int conn_sock_fd) {
    printf("send_file\n");
    struct stat file_stats;
    stat(fp, &file_stats);
    int file_size = file_stats.st_size;
    printf("file size actual: %d\n", file_size);
    int file_fd = open(fp, O_RDONLY);

    printf("file size sent %d\n", send(conn_sock_fd, &file_size, sizeof(int), 0));

    printf("file sent = %d\n", sendfile(conn_sock_fd, file_fd, NULL, file_size));

}