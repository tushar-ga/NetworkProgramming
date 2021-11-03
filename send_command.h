
void send_cmd(int connfd, int cmd_no, char *src, char * dest, int size);
int preCheck(const char *pre, const char *str);
void writeBlock(int fd, int blockNo, char *token, int dataServerConnectionFd);
void readWriteFileBlock(int connfd, int size, char *src, char *dest);