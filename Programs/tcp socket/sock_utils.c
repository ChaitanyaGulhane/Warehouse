/*
Socket functions defination
*/

#include "sock_header.h"
#include<errno.h>

int errno;

int Socket(int family, int type, int protocol)
{
	int sockfd;
	
	if((sockfd = socket(family, type, protocol)) < 0)
	{
		err_sys("Socket Create Eror");
		return -1;
	}

	return sockfd;
}

int Connect(int sockfd, struct sockaddr * sock, socklen_t sock_len)
{
	if(connect(sockfd, (struct sockaddr *) sock, sock_len) < 0)
	{
		err_sys("Socket Connect Error");
		return -1;
	}
}

int Bind(int sockfd, struct sockaddr * sock, socklen_t sock_len)
{
        if(bind(sockfd, (struct sockaddr *) sock, sock_len) < 0)
        {
                err_sys("Socket Bind Error");
                return -1;
        }
	return 0;
}

int Listen(int sockfd, int backlog)
{
	if( listen(sockfd, backlog) < 0)
	{
		err_sys("Socket Listen Error");
		return -1;
	}
	return 0;
}

int Accept(int sockfd, struct sockaddr * client_sock, socklen_t *sock_len)
{
	int connfd;

        if((connfd = accept(sockfd, (struct sockaddr *) client_sock, sock_len)) < 0)
        {
                err_sys("Socket Accept Error");
                return -1;
        }
        return connfd;
}

int Writen(int sockfd, char *buf, int len)
{
	int no_of_bytes;
	if((no_of_bytes = write(sockfd, buf, len)) < 0)
	{
		err_sys("Write Error ....!!!!");
		return -1;
	}
	return no_of_bytes;
}


int Readn(int sockfd, char *buf, int len)
{
        int no_of_bytes;
        if((no_of_bytes = read(sockfd, buf, len)) < 0)
	{
		err_sys("Read Error ....!!!");
                return -1;
	}

        return no_of_bytes;
}

void Close(int sockfd)
{
	close(sockfd);
}

void err_sys(char *string)
{
	perror(string);
}

