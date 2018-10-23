/*邓巧 2016220304031*/
/*date: 18-9-26*/
/*A simple tcp chat program*/ 
/*Server.c*/

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 100

int get_line(char *buffer,int maxlen);
int main(int argc,char *argv[]){
    int serverfd,clientfd;
    char send_buffer[BUFF_SIZE];
    char recev_buffer[BUFF_SIZE];
    char exit_message[] = "exit\n";
    char ip_addr[] = "0.0.0.0";
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in server_addr,client_addr;
    //check the parameter
    if(!argv[1]){
        perror("Lack port number!\n");
        exit(EXIT_FAILURE);
    }
    //get socketfd
    if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to get socketfd!\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    //make bind
    if(bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed to bind!\n");
        exit(EXIT_FAILURE);
    }
    if(listen(serverfd,SOMAXCONN) == -1){
        perror("Failed to listen!\n");
        exit(EXIT_FAILURE);
    }
    printf("Listenning...\n");
    //get clientfd
    if((clientfd = accept(serverfd, (struct sockaddr*) &client_addr, &len)) == -1){
        perror("Failed to accept!\n");
        exit(EXIT_FAILURE);
    }
    printf("Receve message from: %s\n",inet_ntoa(client_addr.sin_addr));
    //repeat recv and send util get message "exit"
    while(1){
        memset(send_buffer,0,sizeof(char) * BUFF_SIZE);
        memset(recev_buffer,0,sizeof(char) * BUFF_SIZE);
        if((recv(clientfd,recev_buffer,sizeof(recev_buffer),0)) <= 0){
            perror("Get no message!\n");
            exit(EXIT_FAILURE);
        }
        if(strcmp(recev_buffer,exit_message) == 0)
            break;
        printf("Receive from client: %s",recev_buffer);
        get_line(send_buffer,BUFF_SIZE);
        send(clientfd,send_buffer,strlen(send_buffer),0);
    }
    //get "exit", close socket
    printf("Conversation closed.\n");
    close(serverfd);
    return 0;
}

//a safe way to get message consisting blank spaces
int get_line(char *line, int max) {
    char c;
    int len;
    for (len = 0; len < max && (c = getchar()) != EOF && c != '\n'; ++len)
        line[len] = c;
    if (c == '\n') {
        line[len] = c;
        len++;
    }
    line[len] = '\0';
    return len;
}
