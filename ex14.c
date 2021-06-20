
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERV_PORT	9000
#define MAXLINE		1000

ssize_t writen(int fd,const void *vptr,size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft>0){
		if ((nwritten = write(fd,ptr,nleft))<=0){
			if (nwritten<0&&errno==EINTR)
				nwritten = 0;
			else
				return -1;
		}

		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

static int read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];
static ssize_t new_read(int fd,char *ptr)
{
	if (read_cnt <= 0){
	again:
		if ((read_cnt = read(fd,read_buf,sizeof(read_buf))) < 0){
			if (errno == EINTR)
				goto again;
			return -1;
		}else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd,void *vptr,size_t maxlen)
{
	ssize_t n,rc;
	char c,*ptr;

	ptr = vptr;
	for (n=1;n<maxlen;n++){
	againt:
		if ((rc = new_read(fd,&c)) == 1){
			*ptr++ = c;
			if (c == '\n')
				break;
		}else if (rc == 0){
			*ptr = 0;
			return n - 1;
		}else
			return -1;
	}

	*ptr = 0;
	return n;
}

void str_cli(FILE *fp,int connfd)
{
	char sendline[MAXLINE],recvline[MAXLINE];

	while(fget(sendline,MAXLINE,fp) != NULL){
		writen(sockfd,sendline,strlen(sendline));
		if (Readline(sockfd,recvline,MAXLINE) == 0){
			perror("Readline");
			return;
		}
		if (fputs(recvline,stdout) == -1){
			perror("fputs");
		}
	}
}

int main(int argc,char *argv[])
{
	int sockfd;
	struct sockaddr_in servaddr;
	
	if (argc!=2 || strcmp(argv[1],"--help")==0){
		fprintf(stderr,"Usage: %s ip-address\n",argv[0]);
		return 1;
	}

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == -1){
		perror("socket");
		return 1;
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET,argv[1],&servaddr.sin_addr) == -1){
		perror("inet_pton");
		return 1;
	}

	if (connect(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) == -1){
		perror("connect");
		return 1;
	}

	str_cli(stdin,sockfd);

	exit(0);
}
