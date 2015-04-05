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

#define SERV_PORT1 3000
#define SERV_PORT2 4000
#define MAXLEN 1024
#define LISTENQ 5

#define INIT_SOCK(sock, port)								\
memset(&sock, 0, sizeof(sock));								\
sock.sin_family = AF_INET;								\
sock.sin_addr.s_addr = htonl(INADDR_ANY);						\
sock.sin_port = htons(port)								\


#define SOCKET(sockfd, type)									\
if((sockfd = socket(AF_INET, type, 0)) == -1) {				        	\
	fprintf(stderr, "socket() tcp  error  => %s", strerror(errno));			\
	ret = -1;									\
}											\									
																	
													

#define BIND(socket, server_socket)							\
if(bind(socket, (struct sockaddr *)&server_socket, sizeof(server_socket)) == -1 ) {	\
	fprintf(stderr, "bind(%d) failed %s", sockfd_tcp, strerror(errno));		\
	ret = -1;									\
	goto cleanall;									\
}											\

#define LISTEN(sockfd_tcp)								\
if(listen(sockfd_tcp, LISTENQ) < 0) {							\
	fprintf(stderr, "connect() tcp error  => %s", strerror(errno));			\
	ret = -1;									\
	goto cleanall;									\
}											\

int
max(int i, int j, int k) {
	if((i > j) && (i > k))
		return i;
	if(j > k)
		return j;
	return k;
}

static int exit_global;

static void interruptHandeler(int signal) {
	exit_global = 1;
}


int 
accept_conn(int sockfd_tcp) {

	socklen_t socklen;
	struct sockaddr_in client_sock_tcp;
	int ret = 0, connfd;

	socklen = sizeof(client_sock_tcp);
	memset(&client_sock_tcp, 0, sizeof(client_sock_tcp));
	if((connfd = accept(sockfd_tcp, (struct sockaddr *)&client_sock_tcp, &socklen)) < 0) {
		if(exit_global) 
			return -2; 
		fprintf(stderr, "accept(%d) tcp error  => %s", sockfd_tcp, strerror(errno));
		return  -1;
	}
	ret = connfd;
	return ret;
}

// TCP relay
// Return -1 to interrupt, -2 to close, -3 to error
int
tcp_relay(int connfd) {
	
	char buffer[1024];
	int len = 0;
	if((len = recv(connfd, buffer, sizeof(buffer)-1, 0)) <=  0) {
		// Interrupt occured
		if(exit_global) {
			fprintf(stdout, "signal captured -- recv\n");
			return -1;	
		}
		if(len == -1) {
			fprintf(stderr, "recv(%d) error  => %s", connfd, strerror(errno));
			return -3;
		}
	}
	buffer[len] = '\0';
	printf("\n\n%s  => TCP Socket\n", buffer);
	strcat(buffer, "  => TCP Socket");
	if((len = send(connfd, buffer, strlen(buffer), 0)) < 0) {
		if(exit_global) {
			fprintf(stdout, "signal captured -- recv\n");
			return -1;	
		}
		if(len == -1) {
			fprintf(stderr, "recv(%d) error  => %s", connfd, strerror(errno));
			return -3;
		}
	}
	if(strncasecmp(buffer, "Close", 5) == 0)
		return -2; 
	return 0;
}

//UDP relay
// Return -1 to interrupt, -2 to close, -3 to error
int 
udp_relay(int sockfd_udp) {

	socklen_t socklen;
	struct sockaddr_in client_sock_udp;
	char buffer[1024];
	int len;

	socklen = sizeof(client_sock_udp);
	memset(&client_sock_udp, 0, sizeof(client_sock_udp));
	
	if((len = recvfrom(sockfd_udp, buffer, MAXLEN, 0, (struct sockaddr *)&client_sock_udp, &socklen)) < 0) {
		if(exit_global)
			return -1;
		fprintf(stderr, "recvfrom(%d) error => %s", sockfd_udp, strerror(errno));
		return -3;
	}

	buffer[len] = 0;
	printf("\n\n%s   => UDP Socket\n", buffer);
	strcat(buffer, "   => UDP Socket");
	
	if((len = sendto(sockfd_udp, buffer, strlen(buffer), 0, (struct sockaddr *)&client_sock_udp, socklen)) < 0) {
		if(exit_global)
			return -1;
		fprintf(stderr, "sendto(%d) error => %s", sockfd_udp, strerror(errno));
		return -3;
	}
	if(strncasecmp(buffer, "Close", 5) == 0)
		return -2; 

	return 0;
}

int
do_polling(int sockfd_tcp, int sockfd_udp) {

	int connfd, maxfd, ret = 0;
	fd_set set;

	if((connfd = accept_conn(sockfd_tcp)) == -2)
		goto cleanup;

	while(1)
	{
		FD_ZERO(&set);
		FD_SET(connfd, &set);
		FD_SET(sockfd_tcp, &set);
		FD_SET(sockfd_udp, &set);
		maxfd = max(sockfd_tcp, sockfd_udp, connfd)  +  1;
		if(select(maxfd, &set, NULL, NULL, NULL) == -1) {
			if(exit_global) {
				goto cleanup;
			}
		}

		if(FD_ISSET(sockfd_tcp, &set)) {
			if((connfd = accept_conn(sockfd_tcp)) == -2)
				goto cleanup;
		}

		if(FD_ISSET(connfd, &set)) {
			if((ret = tcp_relay(connfd)) < 0 ) {
				if(ret == -1)
					goto cleanup;
				if(ret == -2) {
					close(connfd);
					ret = -2;
					continue;
				}
			}	
		}

		if(FD_ISSET(sockfd_udp, &set)) {
			if((ret = udp_relay(sockfd_udp)) < 0) {
				if(ret == -1)
					goto cleanup;
				if(ret == -2) {
					close(sockfd_udp);
					ret = -2;
					continue;
				}
			}
		}
	}

cleanup:
	close(connfd);
	return ret;
}

int
main(void)
{
	int sockfd_udp, sockfd_tcp;
	struct sockaddr_in server_sock_udp, server_sock_tcp; 
	int  ret = 0;

	signal(SIGINT, interruptHandeler);

	SOCKET(sockfd_tcp, SOCK_STREAM);
	if(sockfd_tcp == -1)
		goto cleantcp;

	SOCKET(sockfd_udp, SOCK_DGRAM);
	if(sockfd_udp == -1)
		goto cleanall;	
	
	INIT_SOCK(server_sock_tcp, SERV_PORT1);
	INIT_SOCK(server_sock_udp, SERV_PORT2);

	BIND(sockfd_tcp, server_sock_tcp);
	BIND(sockfd_udp, server_sock_udp);

	LISTEN(sockfd_tcp);

	ret = do_polling(sockfd_tcp, sockfd_udp);
	if(ret == -2)
		goto cleantcp;
cleanall:
	close(sockfd_udp);
cleantcp:
	close(sockfd_tcp);
clean:
	return ret;
}
