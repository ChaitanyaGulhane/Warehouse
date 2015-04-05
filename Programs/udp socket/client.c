

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <arpa/inet.h>

#define MAXLEN 20
#define SERV_PORT 3000


int
main(int argc, char ** argv)
{

	if(argc < 3)
	{
		printf("Insufficcient Arguements...!!!");
		return -1;
	}
	
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("Socket creation error...!!!");
		return -1;
	}

	
	struct sockaddr_in server_sock;
	
	bzero((void *)&server_sock, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &server_sock.sin_addr.s_addr);
	server_sock.sin_port = htons(SERV_PORT);
		
	char buffer[MAXLEN];
	strcpy(buffer, argv[2]);
	int bytes;

	socklen_t len = sizeof(server_sock);
	bytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_sock, len);
	printf("\n Bytes sent = %d", bytes);
	bytes = recvfrom(sockfd, buffer, MAXLEN, 0, (struct sockaddr*)&server_sock, &len);
	buffer[bytes] = 0;
	printf("%s", buffer);
	close(sockfd);

	return 0;
}

