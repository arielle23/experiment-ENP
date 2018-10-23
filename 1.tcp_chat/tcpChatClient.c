/*邓巧 2016220304031*/
/*date: 18-9-26*/
/*A simple tcp chat program*/ 
/*Client.c*/

#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>

#define BUFF_SIZE 100

int get_line(char *buffer,int maxlen);
int main(int argc, char *argv[]){
    //check the parameter
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
    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to get socketfd!\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //make connection
    if(connect(clientfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("Failed to connect!\n");
        exit(EXIT_FAILURE);
    }
    printf("Success connect to server: %s\n",argv[1]);
    //repeat send() and recv() util enter "exit"
    while(1){
        memset(send_buffer,0,sizeof(char) * BUFF_SIZE);
        memset(recev_buffer,0,sizeof(char) * BUFF_SIZE);
        get_line(send_buffer,BUFF_SIZE);
        send(clientfd,send_buffer,strlen(send_buffer),0);
        if(strcmp(send_buffer,exit_message) == 0)
            break;
        recv(clientfd,recev_buffer,sizeof(recev_buffer),0);
        printf("Receive from server: %s",recev_buffer);
    }
    //get "exit", close socket
    printf("Conversation closed.\n");
    close(clientfd);
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