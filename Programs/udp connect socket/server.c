/*
*
*	UDP Server
*
*/
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#define MAXLEN 1024

int
main(int argc, char **argv)
{
	char portalpha[10];

	uint16_t port;

	if(argc < 2)
	{
		printf("Arguements insufficient...!!!");
		return -1;
	}
	
	if(argv[1])
	{
		strcpy(portalpha, argv[1]);
		port = atoi(portalpha);
	}
	
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		fprintf(stderr, "socket() failed %s\n", strerror(errno));
		return -1;
	}
	
	struct sockaddr_in server_sock;
	bzero(&server_sock, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	//inet_pton
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sock.sin_port = htons(port);


	if(bind(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) == -1 )
	{
		printf("Socket Bind Error ...!!!");
		close(sockfd);
		return -1;
	}
	
	char buffer[MAXLEN];
	struct sockaddr_in client_sock;
	int recv_len;
	
	while(1)
	{
		socklen_t len = sizeof(client_sock);
		bzero(&client_sock, sizeof(client_sock));
		recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_sock, &len);
		fprintf(stdout, "read %d bytes\n", recv_len);
		buffer[recv_len] = '\0';
		printf("%s\n", buffer);
		sprintf(&buffer[recv_len], ":%d", port);
		recv_len = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_sock, len);
		fprintf(stdout, "write %d bytes\n", recv_len);
	}

	close(sockfd);

	return 0;
}
