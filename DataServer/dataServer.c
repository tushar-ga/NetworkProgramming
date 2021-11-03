#include <unistd.h>
#include <sys/socket.h>
#include<netdb.h>
#include "communicationPackets.h"
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include<stdlib.h>
#define SERV_PORT 9877
#define IP "192.168.1.7"
#define QUEUE 10

void writeHandler(int connfd, struct dataServer_client_req_packet req){
    int fd = open(req.token,O_CREAT|O_WRONLY);
    write(fd,req.payload,sizeof(req.payload));
}
int main(int argc, char * argv[]){
    int childpid;
    int connfd;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    int port = atoi(argv[1]);
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET,IP,&servaddr.sin_addr);
    char server_addr[20];
    inet_ntop(AF_INET,&servaddr,server_addr,sizeof(servaddr));
    printf("Server running on %s\n", server_addr);
    fflush(stdout);
    if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))==-1){
        perror("Bind Error");
        exit(0);
    }
    if(listen(listenfd,QUEUE)==-1){
        perror("Listen Error");
        exit(0);
    }
    for( ; ; ){
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
        if((childpid=fork())==0){
            close(listenfd);
            struct dataServer_client_req_packet req;
            while(1){
                if(read(connfd,&req,sizeof(req))==0) {close(connfd); exit(0);};
                char addr[20];
                inet_ntop(AF_INET,&cliaddr.sin_addr,&addr,20);
                printf("Request received from %s, Command No: %d",addr,req.command_no);
                fflush(stdout);
                switch(req.command_no){
                    case 0:
                    case 1: writeHandler(connfd,req); break;
                    case 2:
                    default : printf("Invalid command number received\n");
                }
            }
            
        }
        close(connfd);
    }
   

}