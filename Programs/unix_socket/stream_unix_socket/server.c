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

#define LISTENQ 10
#define MAX_BUFF_SIZE 1024

static inline
int create_unix_socket() {
	int sockfd = socket(AF_LOCAL, SOCK_STREAM, IPPROTO_IP);
	return sockfd;
}

static inline
int chk_unlink_and_bind(int sockfd, const char* path) {
	int ret = 0;
	struct stat st;
	memset((void*)&st, 0, sizeof(struct stat));

	if(stat(path, &st) < 0) {
		fprintf(stderr, "stat error => %m\n");
		ret = -1;
		goto end;
	}	
	unlink(path);

	struct sockaddr_un unix_socket_server;
	memset((void*)&unix_socket_server, 0, sizeof(unix_socket_server));
	if(strlen(path) >= sizeof(unix_socket_server.sun_path)) {
		fprintf(stderr, "Path of file to be bound is bigger. Maximum size allowed => %d", sizeof(unix_socket_server.sun_path)-1 );
		ret =-1;
		goto end;
	}
	strcpy(unix_socket_server.sun_path, path);
	unix_socket_server.sun_family = AF_LOCAL;

	if(bind(sockfd, (struct sockaddr *)&unix_socket_server, sizeof(unix_socket_server)) < 0) {
		fprintf(stderr, "bind(%d) error => %m\n", sockfd);
		ret = -1;
	}
end:
	return ret;
}

static inline
int relay_server(int sockfd) {
	int ret = 0;
	int listenfd = 0;
	struct sockaddr_un unix_socket_client;
	memset((void*)&unix_socket_client, 0, sizeof(struct sockaddr_un));
	socklen_t len = 0;

	if((listenfd = accept(sockfd, (struct sockaddr*)&unix_socket_client, &len)) < 0) {
		fprintf(stderr, "accept(%d) error => %m\n", sockfd);
		ret = -1;
		goto done;
	}
	fprintf(stdout, "Accept successfull \n.");
	char buff[MAX_BUFF_SIZE];
	int bytes = 0;
	while((bytes = read(listenfd, buff, MAX_BUFF_SIZE -1)) >= 0) {
		buff[bytes] = '\0';
		fprintf(stdout, "Bytes read => %d\n", bytes);
		fprintf(stdout, "Read data => %s\n", buff);
		if(strcasecmp(buff, "Bye") == 0) {
			fprintf(stdout, "Stopping server..!!\n");
			break;
		}
		if((bytes = write(listenfd, buff, strlen(buff))) < 0) {
			fprintf(stderr, "write(%d) error => %m\n", sockfd);
			ret = -1;
			break;	
		}
		fprintf(stdout, "Bytes written => %d\n", bytes);
	}
	if(errno) {
		fprintf(stderr, "read error => %m\n");
	}
	fprintf(stdout, "Read done\n.");
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

	fprintf(stdout, "Socket created => %d \n", sockfd);

	if(chk_unlink_and_bind(sockfd, argv[1]) <  0) {
		ret = -1;
		goto done;
	}

	struct sockaddr_un unix_socket_temp;
	memset((void*)&unix_socket_temp, 0, sizeof(struct sockaddr_un));
	socklen_t len = sizeof(struct sockaddr_un);
	
	if(getsockname(sockfd, (struct sockaddr *)&unix_socket_temp, &len) < 0) {
		fprintf(stderr, "getsockname() error => %m\n");		
		ret = -1;
		goto done;
	}

	fprintf(stdout, "Socket bound successfully to => %s\n", unix_socket_temp.sun_path);
	if(listen(sockfd, LISTENQ) < 0) {
		fprintf(stderr, "listen(%d) error => %m\n", sockfd);
		ret = -1;
		goto done;
	}
	if(relay_server(sockfd) < 0) {
		ret = -1;
		goto done;
	}
done:
	close(sockfd);
	return ret;
}
