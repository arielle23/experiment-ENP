/*邓巧 2016220304031*/
/*date: 18-10-23*/
/*A simple tcp chat program*/ 
/*client.c*/

#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include<string.h>

#define BUFF_SIZE 100

int get_line(char *buffer,int maxlen);
char exit_message[] = "exit\n";
void* send_message(void* socketfd);
void* recv_message(void* socketfd);

int main(int argc, char *argv[]){
    void* ret;
    pthread_t send_pid,recv_pid;
    if(argc != 3){
        perror("Lack port and address!\n");
        exit(EXIT_FAILURE);
    }
    int serverfd,clientfd;
    char send_buffer[BUFF_SIZE];
    char recev_buffer[BUFF_SIZE];
    struct sockaddr_in server_addr;
    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to get socketfd!\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if(connect(clientfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("Failed to connect!\n");
        exit(EXIT_FAILURE);
    }
    printf("Success connect to server: %s\n",argv[1]);

    if(pthread_create(&send_pid,NULL,send_message,(void *)&clientfd) != 0){
        perror("Failed to create send pthread!\n");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&recv_pid,NULL,recv_message,(void *)&clientfd) != 0){
        perror("Failed to create recv pthread!\n");
    }
    pthread_join(send_pid,&ret);
    if(pthread_cancel(recv_pid) != 0){
        perror("Failed to cancel thread!\n");
    }
    pthread_join(recv_pid,&ret);
    printf("Conversation closed.\n");
    /* close(clientfd); */
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

void* recv_message(void* socketfd){
    char recev_buffer[BUFF_SIZE];
    memset(recev_buffer,0,BUFF_SIZE);
    if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0){
        perror("Failed to set cancel!\n");
    }
    while(1){
        pthread_testcancel();
        if(recv(*((int *)socketfd),recev_buffer,sizeof(recev_buffer),0) > 0){
            printf("Receive from server: %s",recev_buffer);
            memset(recev_buffer,0,BUFF_SIZE);
        }
    }
}

void* send_message(void *socketfd){
    void *retval = NULL;
    char send_buffer[BUFF_SIZE];
    memset(send_buffer,0,BUFF_SIZE);
    // printf("The pid for recv thread: %d\n",((struct parameter*)send_parameter)->pid);
    while(1){
        if(get_line(send_buffer,BUFF_SIZE) > 0){
            if(strcmp(send_buffer,exit_message) == 0){
                pthread_exit(retval);
            }
            else{
                send(*((int*)socketfd),send_buffer,strlen(send_buffer),0);
                memset(send_buffer,0,BUFF_SIZE);
            }
        }
    }
}
