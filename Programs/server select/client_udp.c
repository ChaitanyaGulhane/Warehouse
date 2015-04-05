

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>
#include<signal.h>

#define MAXLEN 20
#define SERV_PORT 4000


static int exit_global;
static void
sighdl(int signal) {
        exit_global = 1;
}

int
main(int argc, char ** argv)
{
	int sockfd;
	struct sockaddr_in server_sock;
	int bytes;
	char buffer[MAXLEN];
	socklen_t len ;

	signal(SIGINT, sighdl);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Socket()  error  => %s", strerror(errno));
		return -1;
	}
	
	bzero((void *)&server_sock, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	if(argv[1]) {
		inet_pton(AF_INET, argv[1], &server_sock.sin_addr.s_addr);
	}
	else {
		inet_pton(AF_INET, "127.0.0.1", &server_sock.sin_addr.s_addr);
	}
	server_sock.sin_port = htons(SERV_PORT);
		
	len = sizeof(server_sock);

	while(gets(buffer)) {
		printf("\n\nQuery => %s\n", buffer);
		
		if((bytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_sock, len)) < 0) {
			if(exit_global)
				goto cleanup;
			fprintf(stderr, "sendto(%d) error => %s", sockfd, strerror(errno));
			break;
		}
		
		if((bytes = recvfrom(sockfd, buffer, MAXLEN, 0, (struct sockaddr*)&server_sock, &len)) < 0) {
			if(exit_global)
				goto cleanup;
			fprintf(stderr, "recvfrom(%d) error => %s", sockfd, strerror(errno));
			break;
		}
		buffer[bytes] = 0;
		
		printf("\nReply => %s\n\n\n", buffer);
		
		if(strcmp(buffer, "Close socket UDP") == 0)
			break;
	}

cleanup:
	printf("\n\n UDP socket closing");
	close(sockfd);

	return 0;
}

