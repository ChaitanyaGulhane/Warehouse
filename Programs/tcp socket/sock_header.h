/*
 header file for socket operations
*/

#ifndef SOCK_HEADER_H
#define SOCK_HEADER_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int Socket(int family, int type, int protocol);
int Bind(int sockfd, struct sockaddr * server_sock, socklen_t sock_len);
int Accept(int sockfd, struct sockaddr * client_sock, socklen_t *sock_len);
int Listen(int sockfd, int backlog);
int Connect(int sockfd, struct sockaddr * server_sock, socklen_t sock_len);
int Writen(int sockfd, char * buf, int len);
int Readn(int sockfd, char * buf, int len);
void Close(int sockfd);
void err_sys(char * string);

#endif
