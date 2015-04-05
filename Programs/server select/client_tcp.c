#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>

#define MAXLEN 1024
#define SERV_PORT 3000

static int exit_global;
static void
sighdl(int signal) {
        exit_global = 1;
}

 
int main(int argc, char **argv)
{
	struct sockaddr_in server_sock;
	int sockfd;
	char line[MAXLEN];
	int len ;

	signal(SIGINT, sighdl);

        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fprintf(stderr, "socket() error => %s ", strerror(errno));
		return -1;
	}

        memset(&server_sock, 0, sizeof(server_sock));
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons(SERV_PORT);
	if(argv[1]) {
		inet_pton(AF_INET, argv[1], &server_sock.sin_addr);
	}
	else {
		inet_pton(AF_INET, "127.0.0.1", &server_sock.sin_addr);
	}

        if(connect(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1) {
		close(sockfd);
		return -1;
	}

	printf("\nQuery => ");
	while(1) {
		gets(line);
		if(strlen(line) <= 0) {
			fprintf(stdout, "errno = %s   %d\n", strerror(errno), errno);
			goto cleanup;
		}
		
	 	if((len = send(sockfd, line, strlen(line), 0)) < 0) {
			if(len == -1)
				fprintf(stderr, "recv(%d) error  => %s", sockfd, strerror(errno));
			break;
                }

		if((len = recv(sockfd, line, sizeof(line)-1, 0)) <=  0) {
			// Interrupt occured
			if(exit_global) {
				fprintf(stdout, "signal captured -- read\n");
				goto cleanup;
			}
			if(len == -1)
				fprintf(stderr, "recv(%d) error  => %s", sockfd, strerror(errno));
			break;
                }
		line[len] = 0;
		printf("\nReply => %s", line);		
		
		if(strcmp(line, "Close socket TCP") == 0)
			break;

		printf("\n\nQuery => ");
	}

cleanup:
	printf("\n\n TCP socket closing");
	close(sockfd);
	return 0;
}
