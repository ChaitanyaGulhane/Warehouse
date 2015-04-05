#ifndef _NETLINK_HEADER_H
#define _NETLINK_HEADER_H

#include<linux/rtnetlink.h>
#include<sys/socket.h>

#define SUCCESS	0
#define ERROR 	-1


#define KERNEL_PROCESS 0

// Initializing routines
void init_netlink_socket(struct sockaddr_nl *netlink_sock, pid_t pid, __u32 group);



int connect_sock(const struct sockaddr_nl *netlink_sock);
int bind_sock(struct sockaddr_nl *netlink_sock, int sock);

#endif
