#include <unistd.h>

#define SERV_PORT	9000
#define MAXLINE		1000

ssize_t writen(int fd,const void *vptr,size_t n);
