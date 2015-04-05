#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "netlink_status.h"
#define MAX_SIZE 4096

struct req_nl
{	
	struct nlmsghdr nlmsg;
	struct ifinfomsg ifinfo;
	char buffer[MAX_SIZE];		
};

void init_netlink_socket(struct sockaddr_nl *netlink_sock, pid_t pid, __u32 group)
{
	memset((void *)netlink_sock, 0, sizeof(struct sockaddr_nl));
	
	netlink_sock->nl_family = AF_NETLINK;
	netlink_sock->nl_pid	= pid;	
	netlink_sock->nl_groups = group;
}

int connect_sock(const struct sockaddr_nl *netlink_sock)
{
	int sock;
	
	if((sock = connect(AF_NETLINK, (struct sockaddr *)netlink_sock, NETLINK_ROUTE)) < 0)
		return ERROR;
	
	return sock;
}

int bind_sock(struct sockaddr_nl *netlink_sock, int sock)
{
	if(bind(sock, (struct sockaddr *) netlink_sock, sizeof(netlink_sock)) < 0)
		return ERROR;	

	return SUCCESS;
}

int init_request(struct req_nl *req, struct msghdr *msg, struct iovec *iov, struct sockaddr_nl *netlink_sock)
{

	memset((void *)&req->nlmsg, 0 , sizeof(struct nlmsghdr));
	req->nlmsg.nlmsg_len 	= NLMSG_LENGTH(sizeof(struct rtmsg));
	req->nlmsg.nlmsg_flags 	= NLM_F_REQUEST | NLM_F_DUMP;
	req->nlmsg.nlmsg_pid 	= getpid();


	memset((void *)&req->ifinfo, 0 , sizeof(struct ininfomsg);
	//req->ifinfo	
	

	memset((void *)msg, 0, sizeof(struct msghdr));
	msg->msg_name 	    = (void *) netlink_sock;
	msg->msg_namelen    = sizeof(struct sockaddr_nl);
	msg->msg_iov	    = iov;
	msg->msg_iovlen     = sizeof(iov)/sizeof(struct iovec);
	msg->msg_flags 	    = 0;


	return SUCCESS;
}

int netlink_status()
{
	int sock;

	struct sockaddr_nl netlink_socket;
	struct req_nl req_nl;	
	struct msghdr msg;
	struct iovec iov;

	init_netlink_socket(&netlink_socket, KERNEL_PROCESS, 0);
	
	if((sock = connect_sock(&netlink_socket)) == ERROR)
	{	
		perror("Error in connecting socket...!!!!");
		return ERROR;
	}
	
	if(bind_sock(&netlink_socket, sock) == ERROR)
	{
		perror("Error in binding socket...!!!!");
		return ERROR;
	}

	if(init_request(&req_nl, &msg, &iov, &netlink_socket) == ERROR)
	{
		perror("Error in initializing requst..!!!");
		return ERROR;
	}

	return SUCCESS;
}

int main()
{
	if(netlink_status() != 0)
	{
		perror("Error while reading netlink statistics...!!!");
		return -1;
	}

	return 0;
}
