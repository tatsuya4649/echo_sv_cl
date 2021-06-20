#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include "echo.h"
#include "signal.h"
#include "readline.h"

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

void str_echo2(int sockfd)
{
	long arg1,arg2;
	ssize_t n;
	char line[MAXLINE];

	for (;;)
	{
		if ((n = readline(sockfd,line,MAXLINE)) == 0)
			return;

		if (sscanf(line,"%ld%ld",&arg1,&arg2) == 2)
			snprintf(line,sizeof(line),"%ld\n",arg1+arg2);
		else
			snprintf(line,sizeof(line),"input error\n");
		n = strlen(line);
		writen(sockfd,line,n);
	}
}

int main(int argc,char *argv[])
{
	int listenfd,connfd;
	char *backq;
	socklen_t clilen;
	struct sockaddr_in servaddr,cliaddr;
	struct sigaction sa;
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

	/* for SIGCHLD */
	sa.sa_handler = sigchild_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	for(;;){
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd,(struct sockaddr *) &cliaddr,&clilen);
		if (connfd == -1){
			if (errno == EINTR){
				/* signal interrupted */
				continue;
			}else{
				perror("accept");
				return 1;
			}
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
