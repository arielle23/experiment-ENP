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
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to get socketfd!\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    if(bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed to bind!\n");
        exit(EXIT_FAILURE);
    }
    if(listen(listenfd,SOMAXCONN) == -1){
        perror("Failed to listen!\n");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&allset); //initialize the set
    FD_SET(listenfd,&allset); //add listenfd to the set
    FD_SET(STDIN_FILENO,&allset);
    maxfd = (listenfd > STDIN_FILENO) ? listenfd : STDIN_FILENO;
    while(1){
        rset = allset;
        if(select(maxfd + 1,&rset,NULL,NULL,NULL) == -1){
            perror("Failed to select!\n");
            exit(EXIT_FAILURE);
        }
        if(FD_ISSET(STDIN_FILENO,&rset)){
            read(STDIN_FILENO,read_buf,BUFF_SIZE);
            if(strcmp(read_buf,exit_message) == 0){
                printf("Server closed\n");
                break;
            }
            memset(read_buf,0,sizeof(char) * BUFF_SIZE);
            continue;
        }
        else if(FD_ISSET(listenfd,&rset)){ //if there is a new client
            if((clientfd[client_nums] = 
                        accept(listenfd,(struct sockaddr *)&client_addr[client_nums],&len[client_nums])) == -1){
               perror("Failed to accept\n");
               exit(EXIT_FAILURE);
            }
            else{
                maxfd = (clientfd[client_nums] > (maxfd - 1)) ? clientfd[client_nums] : (maxfd - 1);
                FD_SET(clientfd[client_nums],&allset);
                ++client_nums;
                continue;
            }
        }
        else{
            for(int i = 0; i < client_nums; ++i){
                if(clientfd[i] != -1 && FD_ISSET(clientfd[i],&rset)){
                    if(recv(clientfd[i],recv_buf,sizeof(recv_buf),0) <= 0){
                        close(clientfd[i]);
                        FD_CLR(clientfd[i],&allset);
                        clientfd[i] = -1;
                    }
                    else{
                        printf("recev from client #%d : %s",i,recv_buf);
                        sprintf(send_buf,"The server has received %s",recv_buf);
                        if(send(clientfd[i],send_buf,strlen(send_buf),0) == -1){
                            perror("Failed to send\n");
                            exit(EXIT_FAILURE);
                        }
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



