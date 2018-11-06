#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF 1024

void try(int return_value, int error_value, char *error_message);

int main(int argc,char *argv[]){
    speed_t speed = B115200;
    /* const char pathname[] = "/dev/ttyS0"; */
    const char end[] = "0x0D";
    const char exit_message[] = "exit\n";
    int port_fd;
    int flag = 0;
    struct termios port_info;
    char read_buffer[MAX_BUF];
    char write_buffer[MAX_BUF];
    memset(write_buffer,0,sizeof(write_buffer));
    memset(read_buffer,0,sizeof(read_buffer));
    memset(&port_info,0,sizeof(struct termios));
    try((port_fd = open(argv[1],O_RDWR)),-1,"Failed to get port fd");

    try(fcntl(port_fd,F_SETFL,O_NONBLOCK),-1,"Failed to set port nonblock");
    try(fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK),-1,"Failed to set stdin nonblock");

    try(tcgetattr(port_fd,&port_info),-1,"Failed to get port information");
    try(cfsetospeed(&port_info,speed),-1,"Failed to set output baud speed");
    try(cfsetispeed(&port_info,0),-1,"Failed to set input baud speed");
    port_info.c_cflag |= CSTOPB; //Set two stop bits, rather than one.
    port_info.c_cflag |= (PARODD|PARENB); //set Odd Parity Check

    int return_v = 0;
    if((return_v = tcsetattr(port_fd,TCSANOW,&port_info)) == -1){
        if(errno == EINVAL)
            printf("Initialized\n");
        else{
            perror("Failed to set");
            exit(EXIT_FAILURE);
        }
    }

    while(1){
        if(read(port_fd,read_buffer,MAX_BUF) == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                usleep(10);
            else{
                perror("Failed to read");
                exit(EXIT_FAILURE);
            }
        }
        else{
            char *index = read_buffer;
            while(*(index++) !=  '\n')           
                ;
            *index = '\0';
            if(strcmp(read_buffer,exit_message) == 0)
                break;
            printf("Recev: %s",read_buffer);
            memset(read_buffer,0,sizeof(char) * MAX_BUF); 
        }
        if(read(STDIN_FILENO,write_buffer,MAX_BUF) == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                usleep(10);
            else{
                perror("Failed to read from stdin");
                exit(EXIT_FAILURE);
            }
        }
        else{
            flag = strcmp(write_buffer,exit_message);
            strcat(write_buffer,end);
            try(write(port_fd,write_buffer,MAX_BUF),-1,"Faied to write to port");
            memset(write_buffer,0,sizeof(char) * MAX_BUF);
            if(flag == 0) break;
        }
    }
    try(close(port_fd),-1,"Failed to close the port");
    printf("The port close\n");
    return 0;
}

void try(int return_value, int error_value, char *error_message){
    if(return_value == error_value){
        perror(error_message);
        exit(EXIT_FAILURE);
    }
    return;
}
