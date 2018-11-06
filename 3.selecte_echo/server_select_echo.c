/*邓巧 2016220304031*/
/*date: 18-10-23*/
/*A simple tcp chat program,*/ 
/*Server.c*/

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/select.h>
#include<sys/time.h>

#define BUFF_SIZE 100
#define MAX_CLIENT 10

void try(int return_value,int error_value,const char *error_message);
char exit_message[] = "exit\n";

int main(int argc,char *argv[]){
    fd_set allset,rset;
    char read_buf[BUFF_SIZE];
    char recv_buf[BUFF_SIZE];
    char send_buf[BUFF_SIZE];
    int listenfd,maxfd,client_nums;
    int clientfd[MAX_CLIENT];
    char ip_addr[] = "0.0.0.0";
    client_nums = 0;
    socklen_t len[MAX_CLIENT] = {sizeof(struct sockaddr)};
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr[MAX_CLIENT];

    if(!argv[1]){
        perror("Lack port number!\n");
        exit(EXIT_FAILURE);
    }

    try((listenfd = socket(AF_INET,SOCK_STREAM,0)), -1, "Failed to get socket");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);

    try(bind(listenfd,(struct sockaddr*)&server_addr,sizeof(server_addr)), -1, "Failed to bind");

    try(listen(listenfd,SOMAXCONN),-1,"Failed to listen");
    printf("Listenning...\n");

    FD_ZERO(&allset); //initialize the set
    FD_SET(listenfd,&allset); //add listenfd to the set
    FD_SET(STDIN_FILENO,&allset); //the keyboard
    maxfd = (listenfd > STDIN_FILENO) ? listenfd : STDIN_FILENO; //get the max

    while(1){
        rset = allset;
        try(select(maxfd + 1, &rset,NULL,NULL,NULL),-1,"Failed to select");

        // if get "exit" message from keyboard, then exit
        if(FD_ISSET(STDIN_FILENO,&rset)){ 
            read(STDIN_FILENO,read_buf,BUFF_SIZE);
            if(strcmp(read_buf,exit_message) == 0){
                while(client_nums > 0){
                    if(clientfd[--client_nums] != -1){
                        try(send(clientfd[client_nums],exit_message,strlen(exit_message),0),-1,"Failed to send");
                        /* printf("%d %d",client_nums,close(clientfd[client_nums])); */
                        --client_nums;
                        /* printf("close success\n"); */
                    }    
                }
                break;
            }
            memset(read_buf,0,sizeof(char) * BUFF_SIZE);
            continue;
        }

        else if(FD_ISSET(listenfd,&rset)){ //if there is a new client
            try((clientfd[client_nums] = 
                        accept(listenfd,(struct sockaddr *)&client_addr[client_nums],&len[client_nums])),-1,"Failed to accept");
            maxfd = (clientfd[client_nums] > (maxfd - 1)) ? clientfd[client_nums] : (maxfd - 1);
            FD_SET(clientfd[client_nums],&allset);
            ++client_nums;
            continue;
        }

        // else, if there is some client send message or close itself
        else{
            for(int i = 0; i < client_nums; ++i){
                //-1 means the client exit before,then skip it
                if(clientfd[i] != -1 && FD_ISSET(clientfd[i],&rset)){
                    if(recv(clientfd[i],recv_buf,sizeof(recv_buf),0) <= 0){
                        close(clientfd[i]);
                        FD_CLR(clientfd[i],&allset);
                        // if a client exit, then clear it from the set, and set it -1 in client[]
                        clientfd[i] = -1;
                    }
                    else{
                        //recev message, then add some information, and send back
                        printf("recev from client #%d : %s",i,recv_buf);
                        sprintf(send_buf,"The server has received %s",recv_buf);
                        try(send(clientfd[i],send_buf,strlen(send_buf),0),-1,"Failed to send");
                        memset(recv_buf,0,BUFF_SIZE);
                        memset(send_buf,0,BUFF_SIZE);
                    }
                }
            }
        }
    }
    close(listenfd);
    exit(EXIT_SUCCESS);
}

void try(int return_value,int error_value,const char *error_message){
    if(return_value == error_value){
        perror(error_message);
        exit(EXIT_FAILURE);
    }
    return;
}



