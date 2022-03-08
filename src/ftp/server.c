#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define SIZE 1024

#define MUADDR "192.168.0.3"
#define BSADDR "192.168.0.1"
#define PORT 8080
 
void handle_connection(int socket);
int getCommand(char* clientMessage);
void getFile();
void putFile();
void terminateConnetion();
 
int main(){
  char *ip = BSADDR;
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
    int n;
    FILE *fp;
    char *filename;
    char buffer[SIZE];
    bzero(buffer, SIZE);
    bool finished = false;

    while(!finished){
        char clientMessage[BUFSIZ], serverResponse[BUFSIZ];
        int command = getCommand(clientMessage);
        switch (command)
        {
        case 0:
            getFile();
            break;
        case 1:
            putFile();
            break;
        case 2:
            terminateConnection();
            break;
        
        default:
            break;
        }
        n = recv(conn_sock_fd, clientMessage, BUFSIZ, 0);
        fp = fopen(filename, "w");
        while (1) {
        n = recv(conn_sock_fd, buffer, 200, 0);
        if (n <= 0){
            break;
            return;
        }
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
        }
        printf("[+]Data written in the file successfully.\n");
    }
    
return;
}

int getCommand(char* clientMessage){
    if(strcmp(clientMessage, "GET") == 0)
        return 0;
    else if(strcmp(clientMessage, "PUT") == 0)
        return 1;
    else if(strcmp(clientMessage, "EXIT") == 0)
        return 2;
}

void getFile(){

}

void putFile(){

}

void terminateConnetion(){

}