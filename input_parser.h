#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#define MAXCMDS 6

enum command_no {LS,CAT,RM,MV,CP,QUIT,INVALID};
enum command_no parse_command(char*user_in, char* cmd, char*src, char*dest);
enum command_no take_command(char *cmd, char*src, char*dest, int *size);
char *gets();