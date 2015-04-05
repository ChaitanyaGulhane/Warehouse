/*
* TCP Client
*
*/

#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>
#include<signal.h>

#define MAXLEN 1024
#define SERV_PORT 3000

static int exit_global;
static void
sighdl(int signal) {
	exit_global = 1;
}
 
int main(int argc, char **argv) {
	
	int sockfd;
	struct sockaddr_in server_sock;
	char ipaddr[16];

	signal(SIGINT, sighdl);
	if(argc > 2) {
		fprintf(stderr, "Arguement insufficient...!!!");
		return -1;
	}
	
        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fprintf(stderr, "socket() error => %s ", strerror(errno));
		return -1;
	}

        memset(&server_sock, 0, sizeof(server_sock));
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons(SERV_PORT);
	if(argc == 1) 
		strcpy(ipaddr, "127.0.0.1");
	else {
		if(strlen(argv[1]) > 15) {
			fprintf(stdout, "IP address of IPv4 to be entered \n");
			return -1;
		}
		strcpy(ipaddr, argv[1]);
	}
	if(inet_pton(AF_INET, ipaddr, &server_sock.sin_addr) ==  0) {
		fprintf(stderr, "\nInvalid IP adrdress \n");
		close(sockfd);
		return -1;
	}

        if(connect(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1) {
		fprintf(stderr, "connect() error => %s", strerror(errno));
		close(sockfd);
		return -1;
	}

	char line[MAXLEN];
	size_t len = 0; 
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);
	FD_SET(0, &set);
	int max = sockfd + 1;

	while(1) {
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		FD_SET(0, &set);

		// This multiplexing is used to switch between stdin and sockfd.

		if(select(max, &set, NULL, NULL, NULL) < 0) {
			// Interrupt occured
			if(exit_global) {
				fprintf(stdout, "signal captured -- select()\n");
				goto cleanup;
			}
			fprintf(stderr, "select() error '%s'", strerror(errno));
			break;
		}

		if(FD_ISSET(0, &set)) {
			gets(line);
			if(strlen(line) > 0) 
				fprintf(stdout, "\n\nQuery => %s\n", line);
			if((len = send(sockfd, line, strlen(line), 0)) < 0) {
				if(len == -1) 
					fprintf(stderr, "recv(%d) error  => %s", sockfd, strerror(errno));
				break;
			}
		}
		
		if(FD_ISSET(sockfd, &set)) {
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
			fprintf(stdout, "Reply => %s\n", line);	
			if(strcmp(line, "Client closed") == 0) 
				break;
		}	
	}

cleanup:
	fprintf(stdout, "\n\nClosing Client...!!!\n");
	close(sockfd);
	return 0;
}
