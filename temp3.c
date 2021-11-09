#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include <sys/wait.h>
#define MAXCMDS 6
#define SERV_PORT 9879
#define IP_ADDR "192.168.1.7"
#define MAXSERVERS 3

char* commands[]={"ls","cat","rm","mv","cp","quit"};
int DATA_PORT[] = {9875,9876,9877};
enum fileBlockCmd{READ, WRITE, DEL};
int dataServerConn[MAXSERVERS];

enum command_no {LS,CAT,RM,MV,CP,QUIT,INVALID};
struct client_req_packet{
    int command_no;
    char src[256];
    char dest[256];
    int size;
};
struct server_resp_packet{
    int response_no;
    char payload[1048576];
};
struct dataServer_client_req_packet{
    int command_no;
    char token[256];
    char payload[1048576];
};
int connfd;
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
int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
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

//Writing a File Block of 1 MB to server and Reading from Client
void writeBlock(int fd, int blockNo, char *token, int dataServerConnectionFd){
    struct dataServer_client_req_packet req;
    req.command_no = WRITE;
    strcpy(req.token,token);
    lseek(fd,blockNo*1048576,SEEK_SET);
    char block[1048576];
    read(fd,block,sizeof(block));
    strcpy(req.payload,block);
    write(dataServerConnectionFd,&req,sizeof(req));
}

//Reading a File Block from server and writing at Client
void readBlock(int fd, int blockNo, char *token, int dataServerConnectionFd){
    struct dataServer_client_req_packet req;
    req.command_no = READ;
    strcpy(req.token,token);
    write(dataServerConnectionFd,&req,sizeof(req));
    struct server_resp_packet res;
    read(dataServerConnectionFd,&res,sizeof(res));
    lseek(fd,blockNo*1048576,SEEK_SET);
    write(fd,res.payload,sizeof(res.payload));
}
//Reading the Block of File from FileNameServer
void readFileBlock(int connfd, int size, char *src, char *dest){
    int readWriteFlag = 0;
    int fd;
    int block;
    char **token;
    char **IP;
    struct server_resp_packet res;
    if(preCheck("./bfs",dest)&& !preCheck("./bfs",src)){
        readWriteFlag = 1;
        fd = open(src,O_RDONLY);
        block = size/1048576;
        if(size%1048576!=0) block++;
        token = (char **)malloc(sizeof(char *) * block);
        IP = (char **)malloc(sizeof(char *) * block);
        for(int i=0;i<block;i++){
            token[i] = (char *)malloc(20 * sizeof(char));
            IP[i] = (char *)malloc(20 * sizeof(char));
            long n = read(connfd,&res,sizeof(res));
            strcpy(token[i],res.payload);
            long f = read(connfd,&res,sizeof(res));
            strcpy(IP[i],res.payload);
            printf("Token: %s\t IP:%s\n read: %ld, read2:%ld",token[i],IP[i],n,f);
        }
    }
    else if(!preCheck("./bfs",dest)&& preCheck("./bfs",src)){
        readWriteFlag = 0;
        if((fd = open(dest,O_CREAT|O_WRONLY)) == -1){
            perror("Opening Dest File");
            exit(0);
        };
        block = 0;
        int maxsize = 1;
        token = (char **)malloc(maxsize* sizeof(char*));
        IP = (char **)malloc(maxsize* sizeof(char*));
        while(read(connfd,&res,sizeof(res))!=0){
            if(res.response_no==-1) break;
            block++;
            if(block>maxsize){
                maxsize = maxsize*2;
                token = (char **)realloc(token,maxsize* sizeof(char*));
                IP = (char **)realloc(IP,maxsize* sizeof(char*));
            }
            token[block-1] = (char *)malloc(20 * sizeof(char));
            IP[block-1] = (char *)malloc(20 * sizeof(char));
            strcpy(token[block-1],res.payload);
            read(connfd,&res,sizeof(res));
            strcpy(IP[block-1],res.payload);
            printf("Token: %s\t IP:%s\n",token[block-1],IP[block-1]);
        }
    }
   
    for(int i=0;i<MAXSERVERS;i++){
        int dataServerConnectionfd = -1;
       
        if(fork()==0){
            for(int j=i;j<block;j=j+MAXSERVERS){
                if(dataServerConnectionfd==-1)  dataServerConnectionfd = connectClient(IP[j],DATA_PORT[j]);
                if(readWriteFlag) writeBlock(fd,j,token[j], dataServerConnectionfd);
                else {readBlock(fd,j,token[j], dataServerConnectionfd);}
            }
            close(fd);
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
    if(cmd_no == 3)
    readFileBlock(connfd,size,src,dest);
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



