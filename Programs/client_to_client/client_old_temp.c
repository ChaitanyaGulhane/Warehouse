#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>

#define MAXLEN 1024
#define SERV_PORT 3000
 
int main(int argc, char **argv) {
	
	if(argc < 2) {
		fprintf(stderr, "Arguement insufficient...!!!\n");
		return -1;
	}
	
	struct sockaddr_in server_sock;
	int sockfd;

        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fprintf(stderr, "socket() error => %s ", strerror(errno));
		return -1;
	}

        memset(&server_sock, 0, sizeof(server_sock));
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons(SERV_PORT);
	if(inet_pton(AF_INET, argv[1], &server_sock.sin_addr) == 0 ) {
		fprintf(stderr, "Invalid IP address");
		close(sockfd);
		return -1;
	}

        if(connect(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1) {
		fprintf(stderr, "connect() error  => %s", strerror(errno));
		close(sockfd);
		return -1;
	}

	char line[MAXLEN];
	size_t len = 0; 
	fd_set set;

	FD_ZERO(&set);
	FD_SET(0, &set);
	FD_SET(sockfd, &set);
	int max = sockfd + 1;

	fprintf(stdout, "\nQuery => \n");

	while(1) {

		if(select(max, &set, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select() error => %s", strerror(errno));
			break;
		}	
		line[0]= 0;
		if(FD_ISSET(0, &set)) {	
			gets(line);
			FD_SET(sockfd, &set);
		}
		if(FD_ISSET(sockfd, &set)) {
			if(write(sockfd, line, strlen(line)) <= 0)
				break;

			if((len = read(sockfd, line, sizeof(line))) <= 0)
				break;

			line[len] = 0;
			fprintf(stdout, "\n Reply => %s\n\n", line);	
			if(strcmp(line, "Client closed") == 0) 
				break;
			fprintf(stdout, "\nQuery => \n");
			FD_SET(0, &set);
		}
	}
	fprintf(stdout, "\n\nClosing Client...!!!\n");
	close(sockfd);

	return 0;
}
