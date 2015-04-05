/*

server code

*/


#include "sock_header.h"
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#define SERV_PORT 3000
#define LISTENQ 5 
#define MAXLEN 20

int main()
{
	int sockfd;
	struct sockaddr_in server_sock, client_sock;

	if((sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		exit(-1);
	}

	bzero(&server_sock, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sock.sin_port = htons(SERV_PORT);

	if(Bind(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1)
	{
		Close(sockfd);
		exit(-1);
	}
	
	if(Listen(sockfd, LISTENQ) == -1)
	{
		Close(sockfd);
		exit(-1);
	}
	
	int len = sizeof(client_sock);
	
	int connfd;
	
	while(1)
	{
		if((connfd = Accept(sockfd, (struct sockaddr *)&client_sock, &len)) == -1)
		{
			Close(sockfd);
			exit(-1);
		}

		pid_t pid = fork();
		
		if(pid == 0)
		{
			Close(sockfd);

			char buffer[MAXLEN];

			while((len = Readn(connfd, buffer, MAXLEN)) > 0)
			{
				printf("%s", buffer);
		
				if(Writen(connfd, buffer, strlen(buffer)) == -1)
					break;

				bzero(buffer, MAXLEN);
			}
			
			exit(0);
		}
		
		Close(connfd);
	}

	Close(sockfd);

	return 0;
}
