#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include"input_parser.h"
#include"send_command.h"
int connfd;


#define SERV_PORT 9879
#define IP_ADDR "172.17.43.69"



int main(int argc, char *argv[]){
    struct sockaddr_in server_addr;
    connfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET,IP_ADDR,&server_addr.sin_addr);
    if(connect(connfd,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1){
        perror("Error establishing a connection with the server\n");
        exit(0);
    }
    while(1){
        char cmd[20];
        char src[128];
        char dest[128];
        int size;
    enum command_no num = take_command(cmd,src,dest,&size);
        if(num==LS||num == MV){
            send_cmd(connfd,num,src,dest,size);
        }
    }
}