/*邓巧 2016220304031*/
/*date: 18-9-26*/
/*A simple tcp echo program*/ 
/*Client.c*/

#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#include<sys/select.h>

#define BUFF_SIZE 100

void try(int,int,const char*);

int main(int argc, char *argv[]){
    //check the parameter
    fd_set set,rset;
    int maxfd;
    if(argc != 3){
        perror("Lack port and address!\n");
        exit(EXIT_FAILURE);
    }
    int serverfd,clientfd;
    char send_buffer[BUFF_SIZE];
    char recev_buffer[BUFF_SIZE];
    char exit_message[] = "exit\n";
    struct sockaddr_in server_addr;
    //get socketfd
    try((clientfd = socket(AF_INET, SOCK_STREAM, 0)),-1,"Failed to get socketfd");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //make connection
    try(connect(clientfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)),-1,"Failed to connect");

    printf("Success connect to server: %s\n",argv[1]);
    
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO,&set);
    FD_SET(clientfd,&set);
    maxfd = (clientfd > STDIN_FILENO) ? clientfd : STDIN_FILENO;
    while(1){
        rset = set;
        if(select(maxfd + 1, &rset, NULL,NULL,NULL) == -1){
            perror("Failed to select\n");
            break;
        }
        if(FD_ISSET(STDIN_FILENO,&rset)){
            memset(send_buffer,0,sizeof(char) * BUFF_SIZE);
            read(STDIN_FILENO,send_buffer,BUFF_SIZE);
            if(strcmp(send_buffer,exit_message) == 0)
                break;
            else{
                if(send(clientfd,send_buffer,strlen(send_buffer),0) == -1){
                    perror("Failed to send\n");
                    break;
                }
            }
        }
        else{
            memset(recev_buffer,0,sizeof(char) * BUFF_SIZE);
            if(recv(clientfd,recev_buffer,sizeof(recev_buffer),0) <= 0){
                break;
            } 
            else{
                printf("%s",recev_buffer);
            }
        }
    }
    printf("Conversation closed.\n");
    try(close(clientfd),-1,"Failed to close");
    return 0;
}

void try(int return_value,int error_value,const char *error_message){
    if(return_value == error_value){
        perror(error_message);
        exit(EXIT_FAILURE);
    }
    return;
}

