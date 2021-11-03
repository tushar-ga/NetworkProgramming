#include "send_command.h"
#include "communicationPackets.h"
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#define MAXSERVERS 3
// #define DATA_PORT 9877
int DATA_PORT[] = {9875,9876,9877};
enum fileBlockCmd{READ, WRITE, DEL};
int dataServerConn[MAXSERVERS];



int preCheck(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}


//connecting to Data Servers
int connectClient(char *IP, int port){
    int socketfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serveraddr2;
    bzero(&serveraddr2,sizeof(serveraddr2));
    serveraddr2.sin_family = AF_INET;
    serveraddr2.sin_port = htons(port);
    // printf("IP: %s",IP);
    inet_pton(AF_INET,IP,&serveraddr2.sin_addr);
    if(connect(socketfd,(struct sockaddr *)&serveraddr2,sizeof(serveraddr2))==-1){
        perror("Connection Data Server");
        exit(0);
    }
    // printf("Successful Connect\n");
    // fflush(stdout);
    return socketfd;
}

//Writing a File Block of 1 MB
void writeBlock(int fd, int blockNo, char *token, int dataServerConnectionFd){
    int childpid;
    // printf("Coming here\n");
    struct dataServer_client_req_packet req;
    req.command_no = WRITE;
    strcpy(req.token,token);
    lseek(fd,blockNo*1024,SEEK_SET);
    char block[1024];
    read(fd,block,sizeof(block));
    strcpy(req.payload,block);
    write(dataServerConnectionFd,&req,sizeof(req));
}

//Reading the Write Block of File from FileNameServer
void readWriteFileBlock(int connfd, int size, char *src, char *dest){
    int readWriteFlag = 0;
    int fd;
    if(preCheck("./bfs",dest)){
        readWriteFlag = 1;
        fd = open(src,O_RDONLY);
    }
    struct server_resp_packet res;
    int block = size/1024;
    if(size%1024!=0) block++;
    char token[block][20];
    char IP[block][20];
    for(int i=0;i<block;i++){
        read(connfd,&res,sizeof(res));
        strcpy(token[i],res.payload);
        read(connfd,&res,sizeof(res));
        strcpy(IP[i],res.payload);
        printf("Token: %s\t IP:%s\n",token[i],IP[i]);
    }
    for(int i=0;i<MAXSERVERS;i++){
        int dataServerConnectionfd = -1;
        if(fork()==0){
            for(int j=i;j<block;j=j+MAXSERVERS){
                if(dataServerConnectionfd==-1)  dataServerConnectionfd = connectClient(IP[j],DATA_PORT[j]);
                if(readWriteFlag) writeBlock(fd,j,token[j], dataServerConnectionfd);
            }
            close(dataServerConnectionfd);
            exit(0);
        }
    }
    for(int i=0;i<block;i++) wait(NULL); //wait for children to finish
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