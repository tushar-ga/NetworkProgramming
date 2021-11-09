#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<fcntl.h>
#define DATA_PORT 9875
#define MAXCMDS 6
int connfd;


#define SERV_PORT 9879
#define IP_ADDR "192.168.1.7"
char* commands[]={"ls","cat","rm","mv","cp","quit"};
enum command_no {LS,CAT,RM,MV,CP,QUIT,INVALID};
enum fileBlockCmd{READ, WRITE, DEL};

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

char *gets(){
    char buff[256];
    int i =0;
    for(i=0;i<256;i++){
        scanf("%c",&buff[i]);
        if(buff[i]=='\n') {
            buff[i]='\0';
            break;
        }
    }
    char * string = malloc(sizeof(char)*(i+1));
    strcpy(string,buff);
    return string;
}

enum command_no parse_command(char*user_in, char* cmd, char*src, char*dest){
    char* tokens[3];
    int c = 0;
    char *token = strtok(user_in," ");
    while(token){
        if(c>=3) {
            printf("Invalid syntax\n");return;
        }
        tokens[c] = token;
        c++;
        token = strtok(NULL," ");
    }
    cmd =tokens[0];
    int cmd_no;
    for(cmd_no=0;cmd_no<MAXCMDS;cmd_no++){
        if(strcmp(commands[cmd_no],cmd)==0) break;
    }
    switch (cmd_no)
        {
        case 0:
            if(c!=1 && c!=2) printf("Please enter a valid syntax.\n");
            else {
                if(c==1){
                    src[0]='\0';
                }
                else{
                    strcpy(src,tokens[1]);
                }
                dest[0]='\0';
            }
            break;
        case 1:
        case 2:
            if(c!=2) printf("Please enter a valid syntax.\n");
            else {
                strcpy(src,tokens[1]);dest='\0';
            }
            break;
        case 3:
        case 4:
            if(c!=3) printf("Please enter a valid syntax.\n");
            else {
                strcpy(src,tokens[1]);
                strcpy(dest,tokens[2]);
            }
            break;
        case 5: break;
        default:
            printf("Not a valid command.\n");
            break;
        }
        fflush(stdout);
        return cmd_no;
}

enum command_no take_command(char *cmd, char*src, char*dest, int *size, int connfd){
    char *user_in;
    *size = 0;
    char orig[256]; //for maintaining original command
    printf(">>>");
    fflush(stdout);
    user_in = gets();
    strcpy(orig,user_in);
    enum command_no cmd_no = parse_command(user_in,cmd,src,dest);
    if(cmd_no == 5){
        close(connfd);
        exit(0);
    }
    int src_bfs = prefix("./bfs",src);
    int des_bfs = prefix("./bfs",dest);
    if(src_bfs&&!des_bfs || !src_bfs&&des_bfs){
        if(des_bfs){
            FILE * fp = fopen(src,"r");
            fseek(fp,0L,SEEK_END);
            *size = ftell(fp);
        }
    }
    return cmd_no;
    
}
struct client_req_packet{
    int command_no;
    char src[256];
    char dest[256];
    int size;
};
struct server_resp_packet{
    int response_no;
    char payload[1024];
};
struct dataServer_client_req_packet{
    int command_no;
    char token[256];
    char payload[1025];
};



int preCheck(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}
int connectClient(char *IP){
    int socketfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serveraddr2;
    bzero(&serveraddr2,sizeof(serveraddr2));
    serveraddr2.sin_family = AF_INET;
    serveraddr2.sin_port = htons(DATA_PORT);
    printf("IP: %s",IP);
    inet_pton(AF_INET,IP,&serveraddr2.sin_addr);
    if(connect(socketfd,(struct sockaddr *)&serveraddr2,sizeof(serveraddr2))==-1){
        perror("Connection Data Server");
        exit(0);
    }
    printf("Successful Connect\n");
    fflush(stdout);
    return socketfd;
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
    int sockfd = connectClient(IP);
    write(sockfd,&req,sizeof(req));
   
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
    enum command_no num = take_command(cmd,src,dest,&size, connfd);
        if(num==LS||num == MV){
            send_cmd(connfd,num,src,dest,size);
        }
    }
}