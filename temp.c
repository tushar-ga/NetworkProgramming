#include <netdb.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#define SERV_PORT 9877
int main(int argc, char **argv){
int sockfd;
struct sockaddr_in servaddr;
if (argc != 2)
printf("usage: tcpcli <IPaddress>");
sockfd = socket(AF_INET, SOCK_STREAM, 0);
bzero(&servaddr, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(SERV_PORT);
inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
char * buf="hello\0";
write(sockfd,buf,6);
exit(0);
}
/* do it all */