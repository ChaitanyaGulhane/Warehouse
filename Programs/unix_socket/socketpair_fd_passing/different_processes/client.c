#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/un.h>
#include <errno.h>
#include <linux/in.h>

#define MAX_BUFF_SIZE 1024

static inline
int create_unix_socket() {
	int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	return sockfd;
}

static inline
int bind_sock(int sockfd) {
	int ret = 0;
	struct sockaddr_un unix_socket_client;

	memset((void*)&unix_socket_client, 0, sizeof(struct sockaddr_un));
	unix_socket_client.sun_family = AF_LOCAL;
	
	if(bind(sockfd, (struct sockaddr*)&unix_socket_client, sizeof(struct sockaddr_un)) <  0) {
		fprintf(stderr, "bind(%d) error => %m\n", sockfd);
		ret = -1;
		goto done;
	}
done:	
	return ret;
}

static inline
int client_relay(int sockfd, const char* path) {
	int ret = 0;
	char buff[MAX_BUFF_SIZE];
	int bytes = 0;
	struct stat st;
	struct sockaddr_un unix_socket_server;

	memset((void*)&st, 0 , sizeof(struct stat));
	
	if(stat(path, &st) < 0) {
		fprintf(stderr, "File does not present\n");
		ret = -1;
		goto done;
	}

	memset((void*)&unix_socket_server, 0, sizeof(unix_socket_server));

	if(strlen(path) >= sizeof(unix_socket_server.sun_path)) {
		fprintf(stderr, "Path of file to be bound is bigger. Maximum size allowed => %d\n", sizeof(unix_socket_server.sun_path)-1);
		ret =-1;
		goto done;
	}
	strcpy(unix_socket_server.sun_path, path);
	unix_socket_server.sun_family = AF_LOCAL;
	socklen_t server_len = sizeof(struct sockaddr_un);

	while(1) {
		if((bytes = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&unix_socket_server, server_len)) < 0) {
			fprintf(stderr, "sendto(%d) error => %m\n", sockfd);
			ret = -1;
			goto done;			
		}
		fprintf(stdout, "Bytes sent => %d\n", bytes);
		if(strcasecmp(buff, "Bye") == 0) {
			break;
		}
		if((bytes = recvfrom(sockfd, buff, MAX_BUFF_SIZE-1, 0, NULL, NULL)) < 0) {
			fprintf(stderr, "recvfrom(%d) error => %m\n", sockfd);
			ret = -1;
			goto done;	
		}
		fprintf(stdout, "Bytes revded => %d\n", bytes);
		fprintf(stdout, "Read data => %s\n", buff);
	}
done:
	return ret;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Insufficient arguements provided\n");
		return -1;
	}
	int sockfd = 0, ret = 0;
	if((sockfd = create_unix_socket()) < 0) {
		fprintf(stderr, "socket(%d) error => %m\n", sockfd);
		return -1;
	}
	if(bind_sock(sockfd) < 0) {
		ret = -1;
		goto done;
	}
	if(client_relay(sockfd, argv[1] ) < 0) {
		ret = -1;
	}
done:
	close(sockfd);
	return ret;
}
