
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "echo.h"
#include "readline.h"
#include <errno.h>

void str_cli(FILE *fp,int connfd)
{
	char sendline[MAXLINE],recvline[MAXLINE];

	while(fgets(sendline,MAXLINE,fp) != NULL){
		writen(connfd,sendline,strlen(sendline));
		if (readline(connfd,recvline,MAXLINE) == 0){
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
