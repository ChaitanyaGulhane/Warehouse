#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define SERV_PORT 3000
#define MAXLEN 20

int
main()
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("Socket creation error...!!!");
		return -1;
	}
	
	struct sockaddr_in server_sock;
	bzero(&server_sock, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sock.sin_port = htons(SERV_PORT);


	if(bind(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) == -1 )
	{
		printf("Socket Bind Error ...!!!");
		close(sockfd);
		return -1;
	}
	
	char buffer[MAXLEN];
	struct sockaddr_in client_sock;
	int recv_len;
	socklen_t len;
	while(1)
	{
		len = sizeof(client_sock);
		bzero(buffer, MAXLEN);
		bzero(&client_sock, sizeof(client_sock));
		recv_len = recvfrom(sockfd, buffer, MAXLEN, 0, (struct sockaddr *)&client_sock, &len);
		printf("\n %d bytes received\n", recv_len);
		printf(" %s", buffer);
		recv_len = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_sock, len);
		printf("\n %d bytes sent\n", recv_len);
	}

	close(sockfd);

	return 0;
}
