#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h> 
#include<errno.h>
#include<signal.h>

#define SERV_PORT 3000
#define MAXLEN 1024
#define LISTENQ 5

#define INIT_SOCK(sock, port)\
memset(&sock, 0, sizeof(sock));\
sock.sin_family = AF_INET;\
sock.sin_addr.s_addr = INADDR_ANY;\
sock.sin_port = htons(port)\

int
max(int i, int j)
{
	if(i > j)
		return i;
	return j;
}

int
main(void)
{
	
	int sockfd;

	struct sockaddr_in server_sock;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "socket() tcp  error  => %s", strerror(errno));
		return -1;
	}
	

	INIT_SOCK(server_sock, SERV_PORT1);

	if(bind(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) == -1 )
	{
		fprintf(stderr, "bind() tcp error  => %s", strerror(errno));
		close(sockfd);
		return -1;
	}
	

	char buffer[MAXLEN];
	struct sockaddr_in client_sock;
	int recv_len, connfd;
	socklen_t len;

	if((connfd = listen(sockfd, LISTENQ)) == -1)
	{
		fprintf(stderr, "connect() tcp error  => %s", strerror(errno));
		close(sockfd);
		return -1;
	}
	
	fd_set set;
	int maxfd;

	while(1)
	{
		FD_ZERO(&set);
		//FD_SET(sockfd_tcp, &set);
		//FD_SET(sockfd_udp, &set);
		//maxfd = max(sockfd_tcp, sockfd_udp)  +  1;
		//select(maxfd, &set, NULL, NULL, NULL);
		len = sizeof(client_sock);
		recv_len = 0;
		memset(&client_sock, 0, sizeof(client_sock));

		if((connfd = accept(sockfd, (struct sockaddr *)&client_sock, &len)) == -1)
		{
			fprintf(stderr, "accept() tcp error  => %s", strerror(errno));
			close(sockfd);
			return -1;
		}

		while((recv_len = read(connfd, buffer, MAXLEN)) > 0) 
		{
			printf("\n%d bytes received --- TCP \n",recv_len);
			buffer[recv_len] = '\0';
			printf("%s", buffer);
			recv_len = write(connfd, buffer, strlen(buffer));
			printf("\n%d bytes sent ---- TCP \n\n", recv_len);
		}

	}

	close(sockfd);

	return 0;
}
