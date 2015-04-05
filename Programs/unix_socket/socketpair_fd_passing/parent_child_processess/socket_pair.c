#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_BUFF_SIZE 1024

int main() {
	int sock_pair[2], ret = 0, status = 0;
	
	if(socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_pair) < 0) {
		fprintf(stderr, "socketpair() error => %m\n");
		ret = -1;
		goto done;
	}
	
	fprintf(stdout, "Socket created n sock_pair=> %d  %d\n", sock_pair[0], sock_pair[1]);
	
	pid_t child;
	if((child = fork()) == 0) {
		close(sock_pair[0]);
		fprintf(stdout, "In child proc\n");
		char buff[1024];
		sprintf(buff, "%d", sock_pair[1]);
		if(write(sock_pair[1], buff, strlen(buff)) < 0)	{
			fprintf(stderr, "write(%d) err => %m", sock_pair[0]);
		}
		fprintf(stdout, "fd = %d written\n", sock_pair[1]);
	//	close(sock_pair[1]);
	//	exit(0);
			
	}
	else if(child == -1) {
		fprintf(stderr, "fork() error => %m\n");
	}
	else {
		close(sock_pair[1]);
		fprintf(stdout, "In parent proc\n");
		if(waitpid(child, &status, 0) < 0 ) {
			fprintf(stderr, "waitpid() error => %m\n");
			ret = -1;
			goto done1;
		}
		char buff[1024];
		if(read(sock_pair[0], buff, sizeof(buff)-1) < 0) {
			fprintf(stderr, "read(%d) error => %m\n", sock_pair[0]);
			ret =-1;
			goto done1;
		}
		fprintf(stdout, "fd got from child => %s\n", buff);
		/*if((status = WEXITSTATUS(&status)) == 0) {
			read()
			fprintf(stdout, "");
		}*/
		
	}

done1:
	close(sock_pair[0]);
done:
	return ret;
}
