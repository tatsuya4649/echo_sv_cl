#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <errno.h>
#include "echo.h"


int main(int argc,char *argv[])
{
	int i,maxi,listenfd,connfd,sockfd;
	int nready;
	int listenq;
	char *listenenv;
	ssize_t n;
	char buf[MAXLINE];
	socklen_t clilen;
	struct pollfd client[FOPEN_MAX];
	struct sockaddr_in cliaddr,servaddr;

	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (listenfd == -1){
		perror("socket");
		return 1;
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);

	if (bind(listenfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) == -1){
		perror("bind");
		return 1;
	}

	listenenv = getenv("LISTENQ");
	listenq = listenenv!=NULL?atoi(listenenv):1;

	if (listen(listenfd,listenq) == -1){
		perror("listen");
		return 1;
	}

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i=1;i<FOPEN_MAX;i++)
		client[i].fd = -1;
	maxi = 0;
	
	for (;;)
	{
		nready = poll(client,maxi+1,-1);
		if (nready == -1){
			perror("poll");
			return 1;
		}
		
		if (client[0].revents & POLLRDNORM){
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd,(struct sockaddr *) &cliaddr,&clilen);
			if (connfd == -1){
				perror("accept");
				continue;
			}

			for (i=1;i<FOPEN_MAX;i++){
				if (client[i].fd < 0){
					client[i].fd = connfd;
					break;
				}
			}
			if (i == FOPEN_MAX){
				fprintf(stderr,"too many clients\n");
				continue;
			}
			client[i].events = POLLRDNORM;
			if (i>maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		}

		for (i=1;i<=maxi;i++){
			if ((sockfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)){
				if ((n = read(sockfd,buf,MAXLINE)) < 0){
					if (errno == ECONNRESET){
						close(sockfd);
						client[i].fd = -1;
					}else{
						fprintf(stderr,"read error");
					}
				}else if (n == 0){
					close(sockfd);
					client[i].fd = -1;
				}else
					writen(sockfd,buf,n);
				if (--nready <= 0)
					break;
			}
		}
	}
}
