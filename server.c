#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include "echo.h"

void str_echo(int connfd)
{
	ssize_t n;
	char buf[MAXLINE];

again:
	while((n=read(connfd,buf,MAXLINE)) > 0){
		if (writen(connfd,buf,n) < 0){
			perror("writen");
			return;
		}
	}

	if (n<0 && errno==EINTR)
		goto again;
	else if (n<0)
		perror("read");
}

int main(int argc,char *argv[])
{
	int listenfd,connfd;
	char *backq;
	socklen_t clilen;
	struct sockaddr_in servaddr,cliaddr;
	pid_t childpid;

	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (listenfd == -1){
		perror("socket");
		return 1;
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	if (bind(listenfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) == -1){
		perror("bind");
		return 1;
	}

	backq = getenv("LISTENQ");

	if (listen(listenfd,backq!=NULL?atoi(backq):1) == -1){
		perror("listen");
		return 1;
	}

	for(;;){
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd,(struct sockaddr *) &cliaddr,&clilen);
		if (connfd == -1){
			perror("accept");
			return 1;
		}
		if ((childpid = fork()) == 0){
			close(listenfd);
			str_echo(connfd);
			exit(0);
		}
		if (close(connfd) == -1){
			continue;
		}
	}
}
