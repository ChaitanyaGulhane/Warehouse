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
	int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	return sockfd;
}

static inline
int chk_unlink_and_bind(int sockfd, const char* path, struct sockaddr_un *unix_socket_server) {
	int ret = 0;
	struct stat st;
	memset((void*)&st, 0, sizeof(struct stat));

	if(stat(path, &st) < 0) {
		fprintf(stderr, "stat error => %m\n");
		ret = -1;
		goto end;
	}	
	unlink(path);

	if(strlen(path) >= sizeof(unix_socket_server->sun_path)) {
		fprintf(stderr, "Path of file to be bound is bigger. Maximum size allowed => %d", sizeof(unix_socket_server->sun_path)-1 );
		ret =-1;
		goto end;
	}
	strcpy(unix_socket_server->sun_path, path);
	unix_socket_server->sun_family = AF_LOCAL;

	if(bind(sockfd, (struct sockaddr *)unix_socket_server, sizeof(struct sockaddr_un)) < 0) {
		fprintf(stderr, "bind(%d) error => %m\n", sockfd);
		ret = -1;
	}
end:
	return ret;
}

static inline
int relay_server(int sockfd, const char* path, struct sockaddr_un *unix_socket_server, socklen_t server_len) {
	int ret = 0;
	struct sockaddr_un unix_socket_client;

	memset((void*)&unix_socket_client, 0, sizeof(struct sockaddr_un));

	unix_socket_client.sun_family = AF_LOCAL;
	if(strlen(path) && (strlen(path) < sizeof(unix_socket_client.sun_path))) {
		strcpy(unix_socket_client.sun_path, path);
	}
	else {
		fprintf(stderr, "The path is bigger than expected.\n");
		ret = -1;
		goto done;
	}
	socklen_t len = sizeof(unix_socket_client);
	socklen_t client_len =  len;

	char buff[MAX_BUFF_SIZE];
	int bytes = 0;
	while((bytes = recvfrom(sockfd, buff, MAX_BUFF_SIZE -1, 0, (struct sockaddr*)&unix_socket_client, &client_len)) >= 0) {
		buff[bytes] = '\0';
		fprintf(stdout, "Bytes read => %d\n", bytes);
		fprintf(stdout, "Recved data => %s\n", buff);
		if(strcasecmp(buff, "Bye") == 0) {
			fprintf(stdout, "Stopping server..!!\n");
			break;
		}
		if((bytes = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&unix_socket_client, client_len)) < 0) {
			fprintf(stderr, "sendto(%d) error => %m\n", sockfd);
			ret = -1;
			break;	
		}
		fprintf(stdout, "Bytes sent => %d\n", bytes);
		client_len = len;
	}
	if(errno) {
		fprintf(stderr, "recvfrom error => %m\n");
	}
	fprintf(stdout, "Recv done\n.");
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

	struct sockaddr_un unix_socket_server;
	memset((void*)&unix_socket_server, 0, sizeof(struct sockaddr_un));
	if(chk_unlink_and_bind(sockfd, argv[1], &unix_socket_server) <  0) {
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
	
	socklen_t server_len = sizeof(struct sockaddr_un);
	if(relay_server(sockfd, argv[1], &unix_socket_server, server_len) < 0) {
		ret = -1;
		goto done;
	}
done:
	close(sockfd);
	return ret;
}
