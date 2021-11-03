#include<unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include"communicationPackets.h"
#include<sys/signal.h>

#include <sys/stat.h>
#include <fcntl.h>
#include<time.h>
#define SERV_PORT 9879
#define LISTENQ 10
#define BUFSIZE 1024
#define IP "172.17.43.69"

int connfd;
int test;
int serverN;
char ** ServerIP;

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

void sendToClient(int connfd, int response_no, char*payload){
    struct server_resp_packet resp_packet;
    resp_packet.response_no = response_no;
    strcpy(resp_packet.payload,payload);
    write(connfd,&resp_packet,sizeof(resp_packet));
}
void sigalrm(int signal){
    char response[BUFSIZ] = "\0";
    struct server_resp_packet resp_packet;
    resp_packet.response_no = 1;
    strcpy(resp_packet.payload,response);
    write(connfd,&resp_packet,sizeof(resp_packet));
    exit(0);
}
char * nextToken(){
    int fd;
    char *token = malloc(sizeof(char)*256);
    token[0] = 'a'-1;
    token[1] = '\0';
    int n =1;
    if((fd = open("tokenFile",O_RDWR))!=-1){
        n = read(fd,token,256);
    }
    else{
        fd = open("tokenFile",O_RDWR|O_CREAT);
    }
    int i;
    for(i=n-1;i>=0;i--){
        if(token[i]<'z'){
            token[i]=token[i]+1;
            break;
        }
        else token[i]='a';
    }
    if(i==-1) {token[n]='a';n++;}
    token[n]='\0';
    close(fd);
    fd = open("tokenFile",O_WRONLY);
    write(fd,token,n);
    return token;
}
void fileFromClientHandler(int connfd, char *src, char*dest, int size)
{       
        int fd;
        if((fd = open(dest,O_CREAT|O_WRONLY))==-1){
            perror("Opening file");
        }
        else{
            srand(time(0));
            int dataServerNo = (rand()%serverN);
            int blocksNo = size/1024;
            if(size%1024 != 0) {blocksNo++;}
            for(int i=0;i<blocksNo;i++){
                char * token = nextToken();
                sendToClient(connfd,0,token);
                sendToClient(connfd,0,ServerIP[dataServerNo]);
                char newline[2] = "\n";
                strcat(token, newline);
                write(fd,token,strlen(token));
                write(fd,ServerIP[dataServerNo],strlen(ServerIP[dataServerNo]));
                write(fd,newline,strlen(newline)); 
                dataServerNo = (dataServerNo+1)%serverN;
            }
            close(fd);
        }
}
      
        
    //     char response[BUFSIZ];
    //     int pipe_desc[2];
    //     pipe(pipe_desc);
    //     close(1);
    //     close(2);
    //     dup(pipe_desc[1]);
    //     dup(pipe_desc[1]);
    //     char final_cmd[256] = "mv ";
    //     strcat(final_cmd,src);
    //     char space[] =" ";
    //     strcat(final_cmd,space);
    //     strcat(final_cmd,dest);
    //     system(final_cmd);
    //     close(pipe_desc[1]);
    //     signal(SIGALRM,sigalrm);
    //     alarm(1);
    //     int n;
    //     if((n=read(pipe_desc[0],response,BUFSIZE))==0){
    //         strcpy(response,"Empty");
    //     }
    //     alarm(0);
    //     response[n]='\0';
    //     struct server_resp_packet resp_packet;
    //     resp_packet.response_no = 0;
    //     strcpy(resp_packet.payload,response);
    //     write(connfd,&resp_packet,sizeof(resp_packet));
    //     exit(0);
    // }

void cpHandler(int connfd, char* src, char*dest, int size){
    if(vfork()==0){
         if(prefix("./bfs",dest)!=0){
                fileFromClientHandler(connfd,src,dest,size);
            }
        else{
            char response[BUFSIZ];
            int pipe_desc[2];
            pipe(pipe_desc);
            close(1);
            close(2);
            dup(pipe_desc[1]);
            dup(pipe_desc[1]);
            char final_cmd[256] = "cp ";
            strcat(final_cmd,src);
            char space[] =" ";
            strcat(final_cmd,space);
            strcat(final_cmd,dest);
            system(final_cmd);
            close(pipe_desc[1]);
            signal(SIGALRM,sigalrm);
            alarm(1);
            int n;
            if((n=read(pipe_desc[0],response,BUFSIZE))==0){
                strcpy(response,"Empty");
            }
            alarm(0);
            response[n]='\0';
            struct server_resp_packet resp_packet;
            resp_packet.response_no = 0;
            strcpy(resp_packet.payload,response);
            write(connfd,&resp_packet,sizeof(resp_packet));
            exit(0);
        }
        
    }
}

void mvHandler(int connfd, char* src, char*dest, int size){
    if(vfork()==0){
            if(prefix("./bfs",dest)!=0){
                fileFromClientHandler(connfd,src,dest,size);
            }
            else
            {    
                char response[BUFSIZ];
                int pipe_desc[2];
                pipe(pipe_desc);
                close(1);
                close(2);
                dup(pipe_desc[1]);
                dup(pipe_desc[1]);
                char final_cmd[256] = "mv ";
                strcat(final_cmd,src);
                char space[] =" ";
                strcat(final_cmd,space);
                strcat(final_cmd,dest);
                system(final_cmd);
                close(pipe_desc[1]);
                signal(SIGALRM,sigalrm);
                alarm(1);
                int n;
                if((n=read(pipe_desc[0],response,BUFSIZE))==0)
                    {
                        strcpy(response,"Empty");
                    }
                alarm(0);
                response[n]='\0';
                struct server_resp_packet resp_packet;
                resp_packet.response_no = 0;
                strcpy(resp_packet.payload,response);
                write(connfd,&resp_packet,sizeof(resp_packet));
            }
            exit(0);
        }
}
void lsHandler(int connfd, char* src){
    if(vfork()==0){
        char response[BUFSIZ];
        int pipe_desc[2];
        pipe(pipe_desc);
        close(1);
        close(2);
        dup(pipe_desc[1]);
        dup(pipe_desc[1]);
        char final_cmd[256] = "ls ./bfs/";
        strcat(final_cmd,src);
        system(final_cmd);
        close(pipe_desc[1]);
        test =0;
        signal(SIGALRM,sigalrm);
        alarm(1);
        int n;
         if((n=read(pipe_desc[0],response,BUFSIZE))==0){
            strcpy(response,"Empty");
        }
        alarm(0);
        response[n]='\0';
        struct server_resp_packet resp_packet;
        resp_packet.response_no = 0;
        strcpy(resp_packet.payload,response);
        write(connfd,&resp_packet,sizeof(resp_packet));
        exit(0);
    }
}

void readEnv(){
    FILE *fp = fopen("./Env.conf","r");
    fscanf(fp,"%d",&serverN);
    fscanf(fp,"\n");
    ServerIP = malloc(sizeof(char*)*serverN);
    for(int i=0;i<serverN;i++){
        ServerIP[i] = (char *)(malloc(sizeof(char)*20));
        fscanf(fp,"%s\n",ServerIP[i]);
        printf("Server IP %d: %s\n",i,ServerIP[i]);
    }
    fclose(fp);
}
int main(int argc, char **argv)
{
    readEnv();
    // fileFromClientHandler(0, " ", "./bfs/f1/test2", 4000);
    int listenfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, 1, sizeof(int));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons (SERV_PORT);
    // char server_addr[20];
    // inet_ntop(AF_INET,&servaddr,server_addr,sizeof(servaddr));
    // printf("Server running on %s\n", server_addr);
    // fflush(stdout);
    if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))==-1){
        perror("Bind Error");
        exit(0);
    };

    if(listen(listenfd, LISTENQ)==-1){
        perror("Error listening\n");
        exit(0);
    };
    char server_addr[20];
    servaddr.sin_addr.s_addr = ntohl(servaddr.sin_addr.s_addr);
    servaddr.sin_port = ntohs (servaddr.sin_port);
    inet_ntop(AF_INET,&servaddr,server_addr,sizeof(servaddr));
    printf("Server running on %s\n", server_addr);
    fflush(stdout);
    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        if ( (childpid = fork()) == 0) { /* child process */
                close(listenfd); /* close listening socket */
                
                struct client_req_packet request;
                // int stdout_fd = dup(1);
                // FILE * out = fdopen(stdout_fd,"a");
                while(1){
                if(read(connfd,&request,sizeof(request))==0){
                    close(connfd);
                    exit(0);
                };
                char addr[20];
                inet_ntop(AF_INET,&cliaddr.sin_addr,&addr,20);
                printf("Request received from %s: Command no: %d Src: %s Dest: %s Size:%d,\n",addr,request.command_no,request.src,request.dest,request.size);
                switch(request.command_no){
                    case 0 : lsHandler(connfd, request.src); break;
                    case 3 : mvHandler(connfd,request.src,request.dest,request.size); break;
                    case 4 : cpHandler(connfd,request.src,request.dest,request.size); break;
                    default : printf("This command is still not functional\n");
                } /* process the request */
                //  fprintf(out,"Request received from %s: Command no: %d \n",addr,request.command_no); /* process the request */
                
                }
                exit (0);
            }
        close(connfd);/* parent closes connected socket */
    }
}