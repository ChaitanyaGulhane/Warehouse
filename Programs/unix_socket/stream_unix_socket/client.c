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
	int sockfd = socket(AF_LOCAL, SOCK_STREAM, IPPROTO_IP);
	return sockfd;
}

static inline
int connect_unix_socket(int sockfd, const char* path) {
	int ret = 0;
	struct stat st;
	memset((void*)&st, 0, sizeof(struct stat));

	if(stat(path, &st) < 0) {
		fprintf(stderr, "stat error => %m\n");
		ret = -1;
		goto end;
	}	
	
	struct sockaddr_un unix_socket_server;
	memset((void*)&unix_socket_server, 0, sizeof(unix_socket_server));
	if(strlen(path) >= sizeof(unix_socket_server.sun_path)) {
		fprintf(stderr, "Path of file to be bound is bigger. Maximum size allowed => %d", sizeof(unix_socket_server.sun_path)-1 );
		ret =-1;
		goto end;
	}
	strcpy(unix_socket_server.sun_path, path);
	unix_socket_server.sun_family = AF_LOCAL;

	if(connect(sockfd, (struct sockaddr *)&unix_socket_server, sizeof(unix_socket_server)) < 0) {
		fprintf(stderr, "connect(%d) error => %m\n", sockfd);
		ret = -1;
	}
end:
	return ret;
}

static inline
int client_relay(int sockfd) {
	int ret = 0;
	char buff[MAX_BUFF_SIZE];
	int bytes = 0;
	while(1) {
		printf("Enter string => ");
		scanf("%s", buff);
		if((bytes = write(sockfd, buff, strlen(buff))) < 0) {
			fprintf(stderr, "write(%d) error => %m\n", sockfd);
			ret = -1;
			goto done;			
		}
		fprintf(stdout, "Bytes written => %d\n", bytes);
		if(strcasecmp(buff, "Bye") == 0) {
			break;
		}
		if((bytes = read(sockfd, buff, MAX_BUFF_SIZE-1)) < 0) {
			fprintf(stderr, "read(%d) error => %m\n", sockfd);
			ret = -1;
			goto done;	
		}
		fprintf(stdout, "Bytes read => %d\n", bytes);
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
	if(connect_unix_socket(sockfd, argv[1]) < 0) {
		ret = -1;
		goto done;
	}
	if(client_relay(sockfd) < 0) {
		ret = -1;
		goto done;
	}
done:
	close(sockfd);
	return ret;
}
