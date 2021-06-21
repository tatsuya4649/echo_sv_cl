
#include "echo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc,char *argv[])
{
	int i,maxi,maxfd,listenfd,connfd,sockfd;
	int nready,client[FD_SETSIZE];
	int backlog;
	char *listenq;
	ssize_t n;
	fd_set rset,allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr,servaddr;

	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (listenfd == -1){
		perror("socket");
		return 1;
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) == -1){
		perror("bind");
		return 1;
	}

	listenq = getenv("LISTENQ");
	backlog = listenq!=NULL?atoi(listenq):1;

	if (listen(listenfd,backlog) == -1){
		perror("listen");
		return 1;
	}

	maxfd = listenfd;
	maxi = -1;
	for (i=0;i<FD_SETSIZE;i++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);

	for (;;)
	{
		rset = allset;
		nready = select(maxfd+1,&rset,NULL,NULL,NULL);
		if (nready == -1){
			perror("select");
			continue;
		}

		if (FD_ISSET(listenfd,&rset)){
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd,(struct sockaddr *) &cliaddr,&clilen);
			if (connfd == -1){
				perror("accept");
				return -1;
			}

			for (i=0;i<FD_SETSIZE;i++)
				if (client[i]<0){
					client[i] = connfd;
					break;
				}

			if (i == FD_SETSIZE){
				fprintf(stderr,"too many clients");
				continue;
			}
			FD_SET(connfd,&allset);
			if (connfd > maxfd)
				maxfd = connfd;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		}

		for (i=0;i<=maxi;i++){
			if ((sockfd = client[i])<0)
				continue;
			if (FD_ISSET(sockfd,&rset)){
				if ((n=read(sockfd,buf,MAXLINE)) == 0){
					close(sockfd);
					FD_CLR(sockfd,&allset);
					client[i] = -1;
				}else
					writen(sockfd,buf,n);

				if (--nready <= 0)
					break;
			}
		}
	}
}
