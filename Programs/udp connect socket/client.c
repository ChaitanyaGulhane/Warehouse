/*
*  UDP Client 
*
*
*/

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLEN 20


int
main(int argc, char ** argv)
{

	uint16_t port1, port2;
	int n;

	if(argc < 4)
	{
		printf("Insufficcient Arguements...!!!");
		return -1;
	}

	if(argv[2] && argv[3])
	{
		port1 = atoi(argv[2]);
		port2 = atoi(argv[3]);
		
	}
	
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("Socket creation error...!!!");
		return -1;
	}

	
	struct sockaddr_in server_sock_conn, server_sock;
	
	bzero((void *)&server_sock_conn, sizeof(server_sock_conn));
	server_sock_conn.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &server_sock_conn.sin_addr.s_addr);
	server_sock_conn.sin_port = htons(port1);
	

	server_sock.sin_family = AF_INET;
        inet_pton(AF_INET, argv[1], &server_sock.sin_addr.s_addr);
        server_sock.sin_port = htons(port2);
	
	char buffer[1024];

	socklen_t len = sizeof(server_sock);
	if(connect(sockfd, (struct sockaddr *)&server_sock_conn, sizeof(server_sock_conn)) == -1)
	{
		printf("Connect Error ...!!!!");
		close(sockfd);
		return -1;
	}


	while(gets(buffer)) {

		// connecting 
		if((n = write(sockfd, buffer, strlen(buffer))) == -1)
		{
			printf("Write Error ");
			close(sockfd);
			return -1;
		}
		fprintf(stdout, "write(%d) bytes %d to 1st severe\n", sockfd, n);

		if((n = read(sockfd, buffer, sizeof(buffer))) == -1)
		{
			printf("Read Error ");
			close(sockfd);
			return -1;
		}
		fprintf(stdout, "read (%d) bytes from 1st server %d\n", sockfd, n);
		buffer[n] = '\0';
		printf("Buffer from 1st Server = %s\n\n", buffer);

		// Disconnecting
		server_sock.sin_family = AF_UNSPEC;
		if(connect(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) == -1)
		{
			printf("Connect Error ...!!!!");
			close(sockfd);
			return -1;
		}

		if(( n = sendto(sockfd, sbuf, strlen(sbuf), 0, (struct sockaddr *)&server_sock, len)) == -1)
		{
			printf("Send to Error ...!!!!");
			close(sockfd);
		}
		fprintf(stdout, "write(%d) bytes %d\n to second server", sockfd, n);	

		if((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_sock, &len)) == -1)
		{
			printf("Receive from error  ");
			close(sockfd);
			return -1;

		}
		buffer[n] = '\0';
		printf("Buffer from second server = %s\n\n", buffer);
	}

	close(sockfd);

	return 0;
}

