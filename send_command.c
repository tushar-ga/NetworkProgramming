#include "send_command.h"
#include "communicationPackets.h"
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
#define DATA_PORT 9877
enum fileBlockCmd{READ, WRITE, DEL};

int preCheck(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}
int connectClient(char *IP){
     int connfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serveraddr;
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = DATA_PORT;
    printf("IP: %s",IP);
    inet_pton(AF_INET,IP,&serveraddr.sin_addr);
    if(connect(connfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))==-1){
        perror("Connection Data Server");
        exit(0);
    }
    printf("Successful Connect\n");
    fflush(stdout);
    return connfd;
}
void writeBlock(int fd, int blockNo, char *IP, char *token){
    int childpid;
    printf("Coming here\n");
    struct dataServer_client_req_packet req;
    req.command_no = WRITE;
    strcpy(req.token,token);
    lseek(fd,blockNo*1024,SEEK_SET);
    char block[1024];
    read(fd,block,sizeof(block));
    strcpy(req.payload,block);
    write(connfd,&req,sizeof(req));
   
}
void readWriteFileBlock(int connfd, int size, char *src, char *dest){
    int readWriteFlag = 0;
    int fd;
    if(preCheck("./bfs",dest)){
        readWriteFlag = 1;
        fd = open(src,O_RDONLY);
    }
    struct server_resp_packet res;
    char token[20];
    char IP[20];
    int block = size/1024;
    if(size%1024!=0) block++;
    for(int i=0;i<block;i++){
        read(connfd,&res,sizeof(res));
        strcpy(token,res.payload);
        read(connfd,&res,sizeof(res));
        strcpy(IP,res.payload);
        printf("Token: %s\t IP:%s\n",token,IP);
        if(readWriteFlag) writeBlock(fd,i,IP,token);
    }
}
void send_cmd(int connfd, int cmd_no, char *src, char * dest, int size){
    struct client_req_packet packet;
    packet.command_no = cmd_no;
    strcpy(packet.src,src);
    strcpy(packet.dest,dest);
    packet.size = size;
    if(write(connfd,&packet,sizeof(packet))==-1){
        perror("Error in sending data to server\n");
        exit(0);
    }
    if(size>0)
    readWriteFileBlock(connfd,size,src,dest);
    else{
        struct server_resp_packet response;
        bzero(&response,sizeof(response));
        read(connfd,&response,sizeof(response));
        if(response.response_no == 0 && response.payload[0]!='\0')
            printf("Server response:\n%s",response.payload);
    }
    
   
}