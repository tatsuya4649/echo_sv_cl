
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "echo.h"
#include "readline.h"
#include <errno.h>

int max(int a,int b)
{
	return a >= b ? a : b;
}

void str_cli(FILE *fp,int connfd)
{
	int maxfdpl,stdineof;
	int n;
	fd_set rset;
	char buf[MAXLINE];

	FD_ZERO(&rset);

	for (;;){
		if (stdineof == 0)
			FD_SET(fileno(fp),&rset);
		FD_SET(connfd,&rset);
		maxfdpl = max(fileno(fp),connfd)+1;
		if (select(maxfdpl,&rset,NULL,NULL,NULL) == -1){
			perror("select");
			return;
		}

		if (FD_ISSET(connfd,&rset)){
			if ((n=read(connfd,buf,MAXLINE)) == 0){
				if (stdineof == 1)
					return;
				else{
					fprintf(stderr,"str_cli: server terminated prematurely\n");
					return;
				}
			}
			write(fileno(stdout),buf,n);
		}
		if (FD_ISSET(fileno(fp),&rset)){
			if ((n=read(fileno(fp),buf,MAXLINE)) == 0){
				stdineof = 1;
				shutdown(connfd,SHUT_WR);
				FD_CLR(fileno(fp),&rset);
				continue;
			}
			writen(connfd,buf,n);
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
