#include"input_parser.h"
char* commands[]={"ls","cat","rm","mv","cp","quit"};
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

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
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
        if(src_bfs){
            FILE * fp = fopen(dest,"r");
            fseek(fp,0L,SEEK_END);
            *size = ftell(fp);
        }
        else{
            FILE * fp = fopen(src,"r");
            fseek(fp,0L,SEEK_END);
            *size = ftell(fp);
        }
    }
    return cmd_no;
    
}