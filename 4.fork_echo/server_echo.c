/*邓巧 2016220304031*/
/*date: 18-9-26*/
/*A simple tcp echo program using nonblock read and accept*/ 
/*Server.c*/

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>

#define BUFF_SIZE 100
#define MAX_CLIENT 10

void echo(int *clientfd,int *serverfd);
void try(int return_value,int error_value,const char *error_message);
char exit_message[] = "exit\n";
int serverfd;

int main(int argc,char *argv[]){
    int client_nums = 0;
    int clientfd[MAX_CLIENT];
    pid_t pid[MAX_CLIENT];
    pid_t child_pid[MAX_CLIENT];
    char ip_addr[] = "0.0.0.0";
    char read_buffer[BUFF_SIZE];
    memset(read_buffer,0,sizeof(char) * BUFF_SIZE);
    socklen_t len[MAX_CLIENT] = {sizeof(struct sockaddr)};
    struct sockaddr_in server_addr,client_addr[MAX_CLIENT];
    //check the parameter
    if(!argv[1]){
        perror("Lack port number!");
        exit(EXIT_FAILURE);
    }

    try((serverfd = socket(AF_INET,SOCK_STREAM,0)) ,-1,"Failed to get socketfd");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);

    //make bind
    try(bind(serverfd,(struct sockaddr*)&server_addr,sizeof(server_addr)),-1,"Failed to bind");
    
    try(listen(serverfd,SOMAXCONN),-1,"Failed to listen");

    printf("Listenning...\n");
    try(fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK),-1,"Failed to set stdin nonblock");
    try(fcntl(serverfd,F_SETFL,O_NONBLOCK),-1,"Failed to set serverfd nonblock");
    while(1){
        //the listenfd is nonblocked.
        //if failed to accept because of nonblock, then sleep, wait for next check
        if((clientfd[client_nums] = 
                       accept(serverfd, (struct sockaddr*)&client_addr[client_nums],&len[client_nums])) == -1){
            if(errno == EAGAIN || errno ==EWOULDBLOCK )
                usleep(10);
        }
        //if success to accept the client,then fork, and do echo()
        else{
            if((pid[client_nums] = fork()) == -1){
                perror("Failed to fork!\n");
                exit(EXIT_FAILURE);
            }
            else if(pid[client_nums] == 0){
                child_pid[client_nums] = getpid();
                echo(&clientfd[client_nums++],&serverfd);
                exit(0);
            }
        }
        //the stdin is set nonblock
        //check if there is something to read
        //if no,then check accept
        if(read(STDIN_FILENO,read_buffer,BUFF_SIZE) == -1){
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                perror("Failed to read\n");
                exit(EXIT_FAILURE);
            }
        }
        //if there is an input, check if it's "exit"
        //if enter "exit",then break and exit.
        //if you enter others, continue.
        else{
            /* printf("get input\n"); */
            if(strcmp(read_buffer,exit_message) == 0)
                break;
            else
                memset(read_buffer,0,sizeof(char) * BUFF_SIZE);
        }
    }
    /* printf("wait: %d\n",wait(NULL)); */
    /* printf("waintng...\n"); */
    printf("%d\n",kill(0,SIGKILL));
    /* perror("kill"); */
    try(wait(NULL),-1,"Failed to wait");
    /* printf("close: %d\n",close(serverfd)); */
    try(close(serverfd),-1,"Failed to close listenfd");
    return 0;
}

void echo(int *clientfd,int *serverfd){
    char write_buffer[BUFF_SIZE];
    char read_buffer[BUFF_SIZE];
    memset(read_buffer,0,sizeof(char) * BUFF_SIZE);
    memset(write_buffer,0,sizeof(char) * BUFF_SIZE);
    //if recv a message then send back
    while(1){
        //the clientfd is nonblock
        //if failed to recv because of nonblock,then sleep and check in next circulation
        if(recv(*clientfd,read_buffer,sizeof(char) * BUFF_SIZE, 0) <= 0){
            if(errno == EAGAIN || errno ==EWOULDBLOCK){
                usleep(10);
            }
            else
                /* perror("recv"); */
                break;
        }
        //if success to recv ,then send back
        else{
            printf("Recev from client #%d : %s",(*clientfd - (*serverfd)),read_buffer);
            sprintf(write_buffer,"Client #%d: %s",(*clientfd - (*serverfd)),read_buffer);
            try(send(*clientfd,write_buffer,strlen(write_buffer),0),-1,"Failed to echo");
            memset(read_buffer,0,sizeof(char) * BUFF_SIZE);
            memset(write_buffer,0,sizeof(char) * BUFF_SIZE);
        }
    }
    close(*clientfd);
    printf("Success close client #%d\n",(*clientfd - (*serverfd))); 
    return;
}

void try(int return_value,int error_value,const char *error_message){
    if(return_value == error_value){
        perror(error_message);
        exit(EXIT_FAILURE);
    }
    return;
}

