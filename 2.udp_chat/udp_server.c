/*邓巧 2016220304031*/
/*date: 18-9-26*/
/*A simple tcp chat program,*/ 
/*Server.c*/

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include <unistd.h>
#include<pthread.h>
#include <signal.h>

#define BUFF_SIZE 100

struct sockaddr_in server_addr,client_addr;
int addr_len = sizeof(struct sockaddr_in);
int serverfd,clientfd;

int get_line(char *buffer,int maxlen);
char exit_message[] = "exit\n";
void* recv_message(void* socketfd);
void* send_message(void* socketfd);
int main(int argc,char *argv[]){
    void *retval;
    char line[BUFF_SIZE];
    pthread_t send_pid,recv_pid;
    char send_buffer[BUFF_SIZE];
    char recev_buffer[BUFF_SIZE];
    char ip_addr[] = "0.0.0.0";
    socklen_t len = sizeof(struct sockaddr);
    if(!argv[1]){
        perror("Lack port number!\n");
        exit(EXIT_FAILURE);
    }
    if((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Failed to get socketfd!\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    if(bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed to bind!\n");
        exit(EXIT_FAILURE);
    }
    
    if((recvfrom(serverfd,(void *)recev_buffer,
                    sizeof(recev_buffer),0,
                    (struct sockaddr *)&client_addr,(socklen_t *)&addr_len)) > 0) {
        printf("Connected to client...\n");
        printf("Receive from client: %s",recev_buffer);
        memset(recev_buffer,0,BUFF_SIZE);
    }
  
    if(pthread_create(&send_pid,NULL,send_message,(void *)&client_addr) != 0){
        perror("Failed to create send thread!\n");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&recv_pid,NULL,recv_message,(void *)&client_addr) != 0)
        perror("Failed to create recv thread!\n");
    pthread_join(recv_pid,&retval);
/*     if(pthread_cancel(send_pid) != 0) */
        /* perror("Failed to cancel thread!\n"); */
    /* pthread_join(send_pid,&retval); */
    /* get_line(line,BUFF_SIZE); */
    printf("Server closed.\n");
    close(serverfd);
    exit(EXIT_SUCCESS);
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

void* recv_message(void* client_addr){
    void *retval = NULL;
    char recev_buffer[BUFF_SIZE];
    memset(recev_buffer,0,BUFF_SIZE);
    while(1){
        if(recvfrom(serverfd,(void *)recev_buffer,
                    sizeof(recev_buffer),0,
                    (struct sockaddr *)client_addr,(socklen_t *)&addr_len) > 0){
            if(strcmp(recev_buffer,exit_message) == 0){
                pthread_exit(retval);
                printf("exit\n");
            }
            printf("Receive from client: %s",recev_buffer);
            memset(recev_buffer,0,BUFF_SIZE);
        }
    }
}

void* send_message(void* client_addr){
    char send_buffer[BUFF_SIZE];
    memset(send_buffer,0,BUFF_SIZE);
    if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0)
        perror("Failed to set cancel!\n");
    while(1){
        if(get_line(send_buffer,BUFF_SIZE) > 0){
            sendto(serverfd,send_buffer,strlen(send_buffer),0,(struct sockaddr *)client_addr,addr_len);
            memset(send_buffer,0,BUFF_SIZE);
        }
        pthread_testcancel();
    }
}
