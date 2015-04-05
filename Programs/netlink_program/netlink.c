/*
Author   - Chaitanya Gulhane
Email ID - chaitanya.linu.014@gmail.com	

Program - Get the interface details by using Netlink socket
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_LEN 16384			// This buffer size should be large.

// Setup the local address to bind to that.
#define INIT_NETLINK_SOCK(x) 				\
	memset((void *)&x, 0, sizeof(x));		\
	x.nl_family = AF_NETLINK;			\
	x.nl_pid    = getpid();				\
	x.nl_groups = 0;				\


#define IPV4ADDR(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]

#define PRINT_FMT "%u.%u.%u.%u"

/*Used following structures below

struct nlmsghdr {
    __u32 nlmsg_len;    // Length of message including header. 
    __u16 nlmsg_type;   // Type of message content. 
    __u16 nlmsg_flags;  // Additional flags. 
    __u32 nlmsg_seq;    // Sequence number. 
    __u32 nlmsg_pid;    // Sender port ID. 
};

- This is used for creating the message. This mesage header is required 
to create any netlink socket message to kernel.


struct rtgenmsg {
      unsigned char  rtgen_family;
};

- This is used pass the generic address family.

*/
struct _request {
	struct nlmsghdr nh;
	struct rtgenmsg gen;
};

/*
Used following structure below:

struct msghdr {
	void         *msg_name        // optional address
	socklen_t     msg_namelen     // size of address
	struct iovec *msg_iov         // scatter/gather array
	int           msg_iovlen      // members in msg_iov
	void         *msg_control     // ancillary data - used only related to struct cmsg
	socklen_t     msg_controllen  // ancillary data - used only when cmsg header is used
	int           msg_flags       // flags on received message
}

- This is used to put the header in any messgae sent by sentmsg().

struct iovec {
      void  *iov_base;    // Starting address 
      size_t iov_len;     // Number of bytes to transfer 
};

- This is used to supply input/output vector in msghdr.

*/
struct _msg {
	struct msghdr msg;
	struct iovec io;
};

struct sockaddr_nl nl;

static inline 
int open_netlink_socket(int *socket_netlink) {
	int ret = 0;
	if(((*socket_netlink) = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0 ) {
		fprintf(stderr, "socket() error => %m\n");
		ret = *socket_netlink;	
	}
	return ret;
}

static inline 
int bind_netlink_socket(const int socket_netlink, const struct sockaddr_nl *sock_addr) {
	int ret = 0;
	if((ret = bind(socket_netlink, (struct sockaddr *)sock_addr, sizeof(struct sockaddr_nl))) < 0 ) {
		fprintf(stderr, "bind(%d) error =. %m\n", socket_netlink);
	}
	return ret;
}

static inline
void prepare_request(struct _request *req) {

	memset(req, 0 , sizeof(*req));

	//Preparing nlmsghdr 
	req->nh.nlmsg_type = RTM_GETADDR;				// Getting address 
	req->nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;		// The type of message is request and (Get the root OR Get the matching tree)
	req->nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));	// Length of ifaddrmsg
	req->nh.nlmsg_seq = 1;						// Indicates first message
	req->nh.nlmsg_pid = getpid();					// Indicates the pid 

	req->gen.rtgen_family = AF_PACKET;				// To get all link information rather than just associated with IP address
	
}

static inline 
int send_msg_to_kernel(const int sock_netlink, struct _request *req) {

	if(send(sock_netlink, req, req->nh.nlmsg_len, 0) < 0) {
		fprintf(stderr, "sendmsg(%d) error => %d - %s\n", sock_netlink, errno, strerror(errno));
		return -1;
	}
	fprintf(stdout, "sendmsg success..!!\n");
	return 0;
}

static inline
int parse_attribute(struct rtattr* rtattribute) {
	struct in_addr *in;
	switch(rtattribute->rta_type) {	
		case IFLA_IFNAME:
			fprintf(stdout, "Interface name = %s \n", (char*)RTA_DATA(rtattribute));
		break;

		case IFA_ADDRESS:
			in = (struct in_addr *)RTA_DATA(rtattribute);
			fprintf(stdout, "addr0: "PRINT_FMT"\n", IPV4ADDR(*in));	
		break;
		
/*		case IFA_LOCAL:
			in = (struct in_addr *)RTA_DATA(rtattribute);
			fprintf(stdout, "addr1: "PRINT_FMT"\n", IPV4ADDR(*in));	
		break;*/
	
		case IFA_BROADCAST:
			in = (struct in_addr *)RTA_DATA(rtattribute);
			fprintf(stdout, "Broadcast: "PRINT_FMT"\n", IPV4ADDR(*in));	
		break;
		
		case IFA_ANYCAST:
			in = (struct in_addr *)RTA_DATA(rtattribute);
			fprintf(stdout, "AnyCast: "PRINT_FMT"\n", IPV4ADDR(*in));	
		break;
	}
	return 0;
}

static inline
int parse_msg(const char* buff, int recv_len) {

	struct nlmsghdr *nlmsg = NULL;
	struct ifaddrmsg *ifm = NULL;
	struct rtattr *rtattribute = NULL;
	int rtattrlen = 0;;

	for(nlmsg = (struct nlmsghdr* )buff; NLMSG_OK(nlmsg, recv_len); nlmsg = NLMSG_NEXT(nlmsg, recv_len)) {

		ifm = (struct ifaddrmsg *) NLMSG_DATA(nlmsg);
		rtattribute = (struct rtattr *) IFA_RTA(ifm);

		fprintf(stdout, "\n\nIndex Of Interface= %d\n", ifm->ifa_index);
		rtattrlen = IFA_PAYLOAD(nlmsg);

		for (; RTA_OK(rtattribute, rtattrlen); rtattribute = RTA_NEXT(rtattribute, rtattrlen)) {
			if(parse_attribute(rtattribute) <  0) {
				return -1;
			}
		}
	}
	return 0;
}


static inline
int recv_msg_from_kernel(int sock_netlink) {
	
	int ret = 0;
	char buff[MAX_LEN];
	while(1) {
		if((ret = recv(sock_netlink, buff, sizeof(buff), 0)) < 0) {
			fprintf(stderr, "recv(%d) err => %m\n", sock_netlink);
			break;
		}
		if(parse_msg(buff, ret) == 0) {
			break;
		}
	}	
	return ret;
}

int main() {
	int socket_netlink = 0;
	int ret = 0;
	struct sockaddr_nl sock_addr;
	struct _request req;
	if((ret = open_netlink_socket(&socket_netlink)) <  0) {
		goto cleanup;
	}
	fprintf(stdout, "Socket opened => %d\n", socket_netlink);
	INIT_NETLINK_SOCK(sock_addr);
	if((bind_netlink_socket(socket_netlink, &sock_addr)) < 0) {
		goto close_sock;
	}
	fprintf(stdout, "bind(%d) successfull\n", socket_netlink);
	prepare_request(&req);
	
	fprintf(stdout, "request prepared\n");
	if(send_msg_to_kernel(socket_netlink, &req) < 0) {
		goto close_sock;
	}
	fprintf(stdout, "send msg done\n");
	if(recv_msg_from_kernel(socket_netlink)  < 0) {
		goto close_sock;
	}
	fprintf(stdout, "\n\nrecv msg done\n");
close_sock:
	close(socket_netlink);	
	fprintf(stdout, "closed socket\n");
cleanup:
	return ret;
}
